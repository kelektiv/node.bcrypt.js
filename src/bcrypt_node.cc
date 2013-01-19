/*
 * Copyright (c) 2010, Nicholas Campbell <nicholas.j.campbell@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *  1. Redistributions of source code must retain the above copyright 
 * notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the <ORGANIZATION> nor the names of its 
 * contributors may be used to endorse or promote products derived from 
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <node.h>
#include <node_version.h>

#include <string>
#include <cstring>
#include <vector>

#include <openssl/rand.h>
#include <openssl/crypto.h>
#include "node_blf.h"

//pulled from node commit - 97cada0
#ifdef _WIN32
# include <windows.h>
#else
# include <pthread.h>
#endif

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

using namespace v8;
using namespace node;

#ifdef _WIN32

static HANDLE* locks;


static void crypto_lock_init(void) {
  int i, n;

  n = CRYPTO_num_locks();
  locks = new HANDLE[n];

  for (i = 0; i < n; i++)
    if (!(locks[i] = CreateMutex(NULL, FALSE, NULL)))
      abort();
}

static void crypto_lock_cb(int mode, int n, const char* file, int line) {
  if (mode & CRYPTO_LOCK)
    WaitForSingleObject(locks[n], INFINITE);
  else
    ReleaseMutex(locks[n]);
}

static unsigned long crypto_id_cb(void) {
  return (unsigned long) GetCurrentThreadId();
}

#else /* !_WIN32 */

static pthread_rwlock_t* locks;

static void crypto_lock_init(void) {

  const int n = CRYPTO_num_locks();
  locks = new pthread_rwlock_t[n];

  for (int i = 0; i < n; i++)
    if (pthread_rwlock_init(locks + i, NULL))
      abort();
}

static void crypto_lock_cb(int mode, int n, const char* file, int line) {
  if (mode & CRYPTO_LOCK) {
    if (mode & CRYPTO_READ) pthread_rwlock_rdlock(locks + n);
    if (mode & CRYPTO_WRITE) pthread_rwlock_wrlock(locks + n);
  } else {
    pthread_rwlock_unlock(locks + n);
  }
}


static unsigned long crypto_id_cb(void) {
  return (unsigned long) pthread_self();
}

#endif /* !_WIN32 */

namespace {

struct baton_base {
    v8::Persistent<v8::Function> callback;
    std::string error;

    virtual ~baton_base() {
        callback.Dispose();
    }
};

struct salt_baton : baton_base {
    std::string salt;
    int rand_len;
    ssize_t rounds;
};

struct encrypt_baton : baton_base {
    std::string salt;
    std::string input;
    std::string output;
};

struct compare_baton : baton_base {
    std::string input;
    std::string encrypted;
    bool result;
};

int GetSeed(uint8_t* seed, int size) {

    // try to get good random bytes first
    if (RAND_bytes((unsigned char *)seed, size) > 0) {
        return 1;
    }

    return RAND_pseudo_bytes(seed, size);
}

bool ValidateSalt(const char* salt) {

    if (!salt || *salt != '$') {
        return false;
    }

    // discard $
    salt++;

    if (*salt > BCRYPT_VERSION) {
        return false;
    }

    if (salt[1] != '$') {
        switch (salt[1]) {
        case 'a':
            salt++;
            break;
        default:
            return false;
        }
    }

    // discard version + $
    salt += 2;

    if (salt[2] != '$') {
        return false;
    }

    int n = atoi(salt);
    if (n > 31 || n < 0) {
        return false;
    }

    if (((uint8_t)1 << (uint8_t)n) < BCRYPT_MINROUNDS) {
        return false;
    }

    salt += 3;
    if (strlen(salt) * 3 / 4 < BCRYPT_MAXSALT) {
        return false;
    }

    return true;
}

/* SALT GENERATION */
void GenSaltAsync(uv_work_t* req) {

    salt_baton* baton = static_cast<salt_baton*>(req->data);

    std::vector<uint8_t> seed(baton->rand_len);
    switch(GetSeed(&seed[0], baton->rand_len)) {
        case -1:
            baton->error = "Rand operation not supported.";
        case 0:
            baton->error = "Rand operation did not generate a cryptographically sound seed.";
    }

    char salt[_SALT_LEN];
    bcrypt_gensalt(baton->rounds, &seed[0], salt);
    baton->salt = std::string(salt);
}

void GenSaltAsyncAfter(uv_work_t* req) {
    HandleScope scope;

    salt_baton* baton = static_cast<salt_baton*>(req->data);
    delete req;

    Handle<Value> argv[2];

    if (!baton->error.empty()) {
        argv[0] = Exception::Error(String::New(baton->error.c_str()));
        argv[1] = Undefined();
    }
    else {
        argv[0] = Undefined();
        argv[1] = Encode(baton->salt.c_str(), baton->salt.size(), BINARY);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    delete baton;
}

Handle<Value> GenerateSalt(const Arguments &args) {
    HandleScope scope;

    const ssize_t rounds = args[0]->Int32Value();
    const int rand_len = args[1]->Int32Value();
    Local<Function> callback = Local<Function>::Cast(args[2]);

    salt_baton* baton = new salt_baton();

    baton->callback = Persistent<Function>::New(callback);
    baton->rand_len = rand_len;
    baton->rounds = rounds;

    uv_work_t* req = new uv_work_t;
    req->data = baton;
    uv_queue_work(uv_default_loop(), req, GenSaltAsync, (uv_after_work_cb)GenSaltAsyncAfter);

    return Undefined();
}

Handle<Value> GenerateSaltSync(const Arguments& args) {
    HandleScope scope;

    const ssize_t rounds = args[0]->Int32Value();
    const int size = args[1]->Int32Value();

    std::vector<uint8_t> seed(size);
    switch(GetSeed(&seed[0], size)) {
        case -1:
            return ThrowException(Exception::Error(String::New("Rand operation not supported.")));
        case 0:
            return ThrowException(Exception::Error(String::New("Rand operation did not generate a cryptographically sound seed.")));
    }

    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, &seed[0], salt);

    return scope.Close(Encode(salt, strlen(salt), BINARY));
}

/* ENCRYPT DATA - USED TO BE HASHPW */
void EncryptAsync(uv_work_t* req) {
    encrypt_baton* baton = static_cast<encrypt_baton*>(req->data);

    if (!(ValidateSalt(baton->salt.c_str()))) {
        baton->error = "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue";
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(baton->input.c_str(), baton->salt.c_str(), bcrypted);
    baton->output = std::string(bcrypted);
}

void EncryptAsyncAfter(uv_work_t* req) {
    HandleScope scope;

    encrypt_baton* baton = static_cast<encrypt_baton*>(req->data);
    delete req;

    Handle<Value> argv[2];

    if (!baton->error.empty()) {
        argv[0] = Exception::Error(String::New(baton->error.c_str()));
        argv[1] = Undefined();
    }
    else {
        argv[0] = Undefined();
        argv[1] = Encode(baton->output.c_str(), baton->output.size(), BINARY);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    delete baton;
}

Handle<Value> Encrypt(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    encrypt_baton* baton = new encrypt_baton();
    baton->callback = Persistent<Function>::New(callback);
    baton->input = std::string(*data);
    baton->salt = std::string(*salt);

    uv_work_t* req = new uv_work_t;
    req->data = baton;
    uv_queue_work(uv_default_loop(), req, EncryptAsync, (uv_after_work_cb)EncryptAsyncAfter);

    return Undefined();
}

Handle<Value> EncryptSync(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());

    if (!(ValidateSalt(*salt))) {
        return ThrowException(Exception::Error(String::New("Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue")));
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*data, *salt, bcrypted);
    return scope.Close(Encode(bcrypted, strlen(bcrypted), BINARY));
}

/* COMPARATOR */
bool CompareStrings(const char* s1, const char* s2) {

    bool eq = true;
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);

    if (s1_len != s2_len) {
        eq = false;
    }

    const int max_len = (s2_len < s1_len) ? s1_len : s2_len;

    // to prevent timing attacks, should check entire string
    // don't exit after found to be false
    for (int i = 0; i < max_len; ++i) {
      if (s1_len >= i && s2_len >= i && s1[i] != s2[i]) {
        eq = false;
      }
    }

    return eq;
}

void CompareAsync(uv_work_t* req) {
    compare_baton* baton = static_cast<compare_baton*>(req->data);

    char bcrypted[_PASSWORD_LEN];
    bcrypt(baton->input.c_str(), baton->encrypted.c_str(), bcrypted);
    baton->result = CompareStrings(bcrypted, baton->encrypted.c_str());
}

void CompareAsyncAfter(uv_work_t* req) {
    HandleScope scope;

    compare_baton* baton = static_cast<compare_baton*>(req->data);
    delete req;

    Handle<Value> argv[2];

    if (!baton->error.empty()) {
        argv[0] = Exception::Error(String::New(baton->error.c_str()));
        argv[1] = Undefined();
    } else {
        argv[0] = Undefined();
        argv[1] = Boolean::New(baton->result);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    // done with the baton
    // free the memory and callback
    delete baton;
}

Handle<Value> Compare(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value input(args[0]->ToString());
    String::Utf8Value encrypted(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    compare_baton* baton = new compare_baton();
    baton->callback = Persistent<Function>::New(callback);
    baton->input = std::string(*input);
    baton->encrypted = std::string(*encrypted);

    uv_work_t* req = new uv_work_t;
    req->data = baton;
    uv_queue_work(uv_default_loop(), req, CompareAsync, (uv_after_work_cb)CompareAsyncAfter);

    return Undefined();
}

Handle<Value> CompareSync(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value pw(args[0]->ToString());
    String::Utf8Value hash(args[1]->ToString());

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*pw, *hash, bcrypted);
    return Boolean::New(CompareStrings(bcrypted, *hash));
}

Handle<Value> GetRounds(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value hash(args[0]->ToString());
    u_int32_t rounds;
    if (!(rounds = bcrypt_get_rounds(*hash))) {
        return ThrowException(Exception::Error(String::New("invalid hash provided")));
    }

    return Integer::New(rounds);
}

} // anonymous namespace

// bind the bcrypt module
extern "C" void init(Handle<Object> target) {
    HandleScope scope;

    crypto_lock_init();
    CRYPTO_set_locking_callback(crypto_lock_cb);
    CRYPTO_set_id_callback(crypto_id_cb);

    NODE_SET_METHOD(target, "gen_salt_sync", GenerateSaltSync);
    NODE_SET_METHOD(target, "encrypt_sync", EncryptSync);
    NODE_SET_METHOD(target, "compare_sync", CompareSync);
    NODE_SET_METHOD(target, "get_rounds", GetRounds);
    NODE_SET_METHOD(target, "gen_salt", GenerateSalt);
    NODE_SET_METHOD(target, "encrypt", Encrypt);
    NODE_SET_METHOD(target, "compare", Compare);
};

NODE_MODULE(bcrypt_lib, init);
