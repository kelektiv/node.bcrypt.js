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

#include <nan.h>

#include <string>
#include <cstring>
#include <vector>
#include <stdlib.h> // atoi

#include "node_blf.h"

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

using namespace v8;
using namespace node;

namespace {

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

class SaltAsyncWorker : public NanAsyncWorker {
  public:
    SaltAsyncWorker(NanCallback *callback, std::string seed, ssize_t rounds)
        : NanAsyncWorker(callback), seed(seed), rounds(rounds) {
    }

    ~SaltAsyncWorker() {}

    void Execute() {
        char salt[_SALT_LEN];
        bcrypt_gensalt(rounds, (u_int8_t *)&seed[0], salt);
        this->salt = std::string(salt);
    }

    void HandleOKCallback() {
        NanScope();

        Handle<Value> argv[2];
        argv[0] = NanUndefined();
        argv[1] = Encode(salt.c_str(), salt.size(), BINARY);
        callback->Call(2, argv);
    }

  private:
    std::string seed;
    std::string salt;
    ssize_t rounds;
};

NAN_METHOD(GenerateSalt) {
    NanScope();

    if (args.Length() < 3) {
        NanThrowError(Exception::TypeError(NanNew("3 arguments expected")));
        NanReturnUndefined();
    }

    if (!Buffer::HasInstance(args[1]) || Buffer::Length(args[1].As<Object>()) != 16) {
        NanThrowError(Exception::TypeError(NanNew("Second argument must be a 16 byte Buffer")));
        NanReturnUndefined();
    }

    const ssize_t rounds = args[0]->Int32Value();
    Local<Object> seed = args[1].As<Object>();
    Local<Function> callback = Local<Function>::Cast(args[2]);

    SaltAsyncWorker* saltWorker = new SaltAsyncWorker(new NanCallback(callback),
        std::string(Buffer::Data(seed), 16), rounds);

    NanAsyncQueueWorker(saltWorker);

    NanReturnUndefined();
}

NAN_METHOD(GenerateSaltSync) {
    NanScope();

    if (args.Length() < 2) {
        NanThrowError(Exception::TypeError(NanNew("2 arguments expected")));
        NanReturnUndefined();
    }

    if (!Buffer::HasInstance(args[1]) || Buffer::Length(args[1].As<Object>()) != 16) {
        NanThrowError(Exception::TypeError(NanNew("Second argument must be a 16 byte Buffer")));
        NanReturnUndefined();
    }

    const ssize_t rounds = args[0]->Int32Value();
    u_int8_t* seed = (u_int8_t*)Buffer::Data(args[1].As<Object>());

    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, seed, salt);

    NanReturnValue(Encode(salt, strlen(salt), BINARY));
}

/* ENCRYPT DATA - USED TO BE HASHPW */

class EncryptAsyncWorker : public NanAsyncWorker {
  public:
    EncryptAsyncWorker(NanCallback *callback, std::string input, std::string salt)
        : NanAsyncWorker(callback), input(input), salt(salt) {
    }

    ~EncryptAsyncWorker() {}

    void Execute() {
        if (!(ValidateSalt(salt.c_str()))) {
            error = "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue";
        }

        char bcrypted[_PASSWORD_LEN];
        bcrypt(input.c_str(), salt.c_str(), bcrypted);
        output = std::string(bcrypted);
    }

    void HandleOKCallback() {
        NanScope();

        Handle<Value> argv[2];

        if (!error.empty()) {
            argv[0] = Exception::Error(NanNew(error.c_str()));
            argv[1] = NanUndefined();
        } else {
            argv[0] = NanUndefined();
            argv[1] = Encode(output.c_str(), output.size(), BINARY);
        }

        callback->Call(2, argv);
    }

  private:
    std::string input;
    std::string salt;
    std::string error;
    std::string output;
};

NAN_METHOD(Encrypt) {
    NanScope();

    if (args.Length() < 3) {
        NanThrowError(Exception::TypeError(NanNew("3 arguments expected")));
        NanReturnUndefined();
    }

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    EncryptAsyncWorker* encryptWorker = new EncryptAsyncWorker(new NanCallback(callback),
        std::string(*data), std::string(*salt));

    NanAsyncQueueWorker(encryptWorker);

    NanReturnUndefined();
}

NAN_METHOD(EncryptSync) {
    NanScope();

    if (args.Length() < 2) {
        NanThrowError(Exception::TypeError(NanNew("2 arguments expected")));
        NanReturnUndefined();
    }

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());

    if (!(ValidateSalt(*salt))) {
        NanThrowError("Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
        NanReturnUndefined();
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*data, *salt, bcrypted);
    NanReturnValue(Encode(bcrypted, strlen(bcrypted), BINARY));
}

/* COMPARATOR */

NAN_INLINE bool CompareStrings(const char* s1, const char* s2) {

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

class CompareAsyncWorker : public NanAsyncWorker {
  public:
    CompareAsyncWorker(NanCallback *callback, std::string input, std::string encrypted)
        : NanAsyncWorker(callback), input(input), encrypted(encrypted) {

        result = false;
    }

    ~CompareAsyncWorker() {}

    void Execute() {
        char bcrypted[_PASSWORD_LEN];
        if (ValidateSalt(encrypted.c_str())) {
            bcrypt(input.c_str(), encrypted.c_str(), bcrypted);
            result = CompareStrings(bcrypted, encrypted.c_str());
        }
    }

    void HandleOKCallback() {
        NanScope();

        Handle<Value> argv[2];
        argv[0] = NanUndefined();
        argv[1] = NanNew<Boolean>(result);
        callback->Call(2, argv);
    }

  private:
    std::string input;
    std::string encrypted;
    bool result;
};

NAN_METHOD(Compare) {
    NanScope();

    if (args.Length() < 3) {
        NanThrowError(Exception::TypeError(NanNew("3 arguments expected")));
        NanReturnUndefined();
    }

    String::Utf8Value input(args[0]->ToString());
    String::Utf8Value encrypted(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    CompareAsyncWorker* compareWorker = new CompareAsyncWorker(new NanCallback(callback),
        std::string(*input), std::string(*encrypted));

    NanAsyncQueueWorker(compareWorker);

    NanReturnUndefined();
}

NAN_METHOD(CompareSync) {
    NanScope();

    if (args.Length() < 2) {
        NanThrowError(Exception::TypeError(NanNew("2 arguments expected")));
        NanReturnUndefined();
    }

    String::Utf8Value pw(args[0]->ToString());
    String::Utf8Value hash(args[1]->ToString());

    char bcrypted[_PASSWORD_LEN];
    if (ValidateSalt(*hash)) {
        bcrypt(*pw, *hash, bcrypted);
        NanReturnValue(NanNew<Boolean>(CompareStrings(bcrypted, *hash)));
    } else {
        NanReturnValue(NanFalse());
    }
}

NAN_METHOD(GetRounds) {
    NanScope();

    if (args.Length() < 1) {
        NanThrowError(Exception::TypeError(NanNew("1 argument expected")));
        NanReturnUndefined();
    }

    String::Utf8Value hash(args[0]->ToString());
    u_int32_t rounds;
    if (!(rounds = bcrypt_get_rounds(*hash))) {
        NanThrowError("invalid hash provided");
        NanReturnUndefined();
    }

    NanReturnValue(NanNew(rounds));
}

} // anonymous namespace

// bind the bcrypt module
extern "C" void init(Handle<Object> target) {
    NanScope();

    NODE_SET_METHOD(target, "gen_salt_sync", GenerateSaltSync);
    NODE_SET_METHOD(target, "encrypt_sync", EncryptSync);
    NODE_SET_METHOD(target, "compare_sync", CompareSync);
    NODE_SET_METHOD(target, "get_rounds", GetRounds);
    NODE_SET_METHOD(target, "gen_salt", GenerateSalt);
    NODE_SET_METHOD(target, "encrypt", Encrypt);
    NODE_SET_METHOD(target, "compare", Compare);
};

NODE_MODULE(bcrypt_lib, init);
