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

#include <openssl/rand.h>

#include "node_blf.h"

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

using namespace v8;
using namespace node;

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
    switch (RAND_bytes((unsigned char *)seed, size)) {
        case -1:
        case 0:
            switch (RAND_pseudo_bytes(seed, size)) {
                case -1:
                    return -1;
                case 0:
                    return 0;
                default:
                    return 1;
            }
        default:
            return 1;
    }
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
#if NODE_LESS_THAN
int EIO_GenSalt(eio_req *req) {
#else
void EIO_GenSalt(eio_req *req) {
#endif

    salt_baton* baton = static_cast<salt_baton*>(req->data);

    char* salt = new char[_SALT_LEN];
    uint8_t* seed = new uint8_t[baton->rand_len];
    switch(GetSeed(seed, baton->rand_len)) {
        case -1:
            baton->error = "Rand operation not supported.";
        case 0:
            baton->error = "Rand operation did not generate a cryptographically sound seed.";
    }

    bcrypt_gensalt(baton->rounds, seed, salt);
    baton->salt = std::string(salt);

    delete[] salt;
    delete[] seed;

#if NODE_LESS_THAN
    return 0;
#endif
}

int EIO_GenSaltAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    salt_baton* baton = static_cast<salt_baton*>(req->data);

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
    return 0;
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

    eio_custom(EIO_GenSalt, EIO_PRI_DEFAULT, EIO_GenSaltAfter, baton);

    ev_ref(EV_DEFAULT_UC);

    return Undefined();
}

Handle<Value> GenerateSaltSync(const Arguments& args) {
    HandleScope scope;

    const ssize_t rounds = args[0]->Int32Value();
    const int size = args[1]->Int32Value();

    uint8_t* seed = new uint8_t[size];
    switch(GetSeed(seed, size)) {
        case -1:
            return ThrowException(Exception::Error(String::New("Rand operation not supported.")));
        case 0:
            return ThrowException(Exception::Error(String::New("Rand operation did not generate a cryptographically sound seed.")));
    }

    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, seed, salt);
    delete[] seed;

    return scope.Close(Encode(salt, strlen(salt), BINARY));
}

/* ENCRYPT DATA - USED TO BE HASHPW */
#if NODE_LESS_THAN
int EIO_Encrypt(eio_req *req) {
#else
void EIO_Encrypt(eio_req *req) {
#endif
    encrypt_baton* baton = static_cast<encrypt_baton*>(req->data);

    if (!(ValidateSalt(baton->salt.c_str()))) {
        baton->error = "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue";
#if NODE_LESS_THAN
        return 1;
#else
        return;
#endif
    }

    char* bcrypted = new char[_PASSWORD_LEN];
    bcrypt(baton->input.c_str(), baton->salt.c_str(), bcrypted);
    baton->output = std::string(bcrypted);

    delete[] bcrypted;

#if NODE_LESS_THAN
    return 0;
#endif
}

int EIO_EncryptAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    encrypt_baton* baton = static_cast<encrypt_baton*>(req->data);

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
    return 0;
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

    eio_custom(EIO_Encrypt, EIO_PRI_DEFAULT, EIO_EncryptAfter, baton);

    ev_ref(EV_DEFAULT_UC);

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

#if NODE_LESS_THAN
int EIO_Compare(eio_req *req) {
#else
void EIO_Compare(eio_req *req) {
#endif
    compare_baton* baton = static_cast<compare_baton*>(req->data);

    char bcrypted[_PASSWORD_LEN];
    bcrypt(baton->input.c_str(), baton->encrypted.c_str(), bcrypted);
    baton->result = CompareStrings(bcrypted, baton->encrypted.c_str());

#if NODE_LESS_THAN
    return 0;
#endif
}

int EIO_CompareAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);

    compare_baton* baton = static_cast<compare_baton*>(req->data);

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

    return 0;
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

    eio_custom(EIO_Compare, EIO_PRI_DEFAULT, EIO_CompareAfter, baton);

    ev_ref(EV_DEFAULT_UC);

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

    NODE_SET_METHOD(target, "gen_salt_sync", GenerateSaltSync);
    NODE_SET_METHOD(target, "encrypt_sync", EncryptSync);
    NODE_SET_METHOD(target, "compare_sync", CompareSync);
    NODE_SET_METHOD(target, "get_rounds", GetRounds);
    NODE_SET_METHOD(target, "gen_salt", GenerateSalt);
    NODE_SET_METHOD(target, "encrypt", Encrypt);
    NODE_SET_METHOD(target, "compare", Compare);
};

