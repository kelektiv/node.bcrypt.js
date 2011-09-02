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
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/rand.h>

#include "node_blf.h"

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

using namespace v8;
using namespace node;

namespace {

struct base_request {
    v8::Persistent<v8::Function> callback;
    const char *error;
};

struct salt_request : base_request {
    char *salt;
    int salt_len;
    int rand_len;
    ssize_t rounds;
};

struct encrypt_request : base_request {
    char *salt;
    char *input;
    char *output;
    int output_len;
};

struct compare_request : base_request {
    char *input;
    char *encrypted;
    bool result;
};

int GetSeed(u_int8_t *seed, int size) {
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

bool ValidateSalt(char *str) {
    int count = 0;
    bool valid = true;

    int str_len = strlen(str);
    char *new_str = (char *)malloc(str_len * sizeof(str));
    memcpy(new_str, str, str_len * sizeof(str));
    char *next_tok = new_str;
    char *result = strsep(&next_tok, "$");

    while (result != NULL) {
        if (count == 1) {
            //check version
            if (!isdigit(result[0])) {
                return false;
            }
            if (strlen(result) == 2 && !isalpha(result[1])) {
                return false;
            }
        } else if (count == 2) {
            //check rounds
            if (!(isdigit(result[0]) && isdigit(result[1]))) {
                return false;
            }
        }

        count++;
        result = strsep(&next_tok, "$");
    }

    free(new_str);
    free(result);

    return (count == 4);
}

/* SALT GENERATION */
#if NODE_LESS_THAN
int EIO_GenSalt(eio_req *req) {
#else
void EIO_GenSalt(eio_req *req) {
#endif
    salt_request *s_req = (salt_request *)req->data;

    char *salt = (char *)malloc(_SALT_LEN);
    try {
        u_int8_t *_seed = (u_int8_t *)malloc(s_req->rand_len * sizeof(u_int8_t));
        switch(GetSeed(_seed, s_req->rand_len)) {
            case -1:
                s_req->error = "Rand operation not supported.";
            case 0:
                s_req->error = "Rand operation did not generate a cryptographically sound seed.";
        }
        bcrypt_gensalt(s_req->rounds, _seed, salt);
        s_req->salt_len = strlen(salt);
        s_req->salt = salt;

        free(_seed);
    } catch (const char *err) {
        s_req->error = err;
        free(salt);
    }
#if NODE_LESS_THAN
    return 0;
#endif
}

int EIO_GenSaltAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    salt_request *s_req = (salt_request *)req->data;

    Handle<Value> argv[2];

    if (s_req->error) {
        argv[0] = Exception::Error(String::New(s_req->error));
        argv[1] = Undefined();
    }
    else {
        argv[0] = Undefined();
        argv[1] = Encode(s_req->salt, s_req->salt_len, BINARY);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    s_req->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    s_req->callback.Dispose();
    free(s_req->salt);

    free(s_req);

    return 0;
}

Handle<Value> GenerateSalt(const Arguments &args) {
    HandleScope scope;

    Local<Function> callback;
    int rand_len = 20;
    ssize_t rounds = 10;
    if (args.Length() < 1) {
        return ThrowException(Exception::Error(String::New("Must provide at least a callback.")));
    } else if (args[0]->IsFunction()) {
        callback = Local<Function>::Cast(args[0]);
    } else if (args.Length() == 1) {
        return ThrowException(Exception::Error(String::New("Must provide at least a callback.")));
    }
    if (args.Length() > 1) {
        if (args[1]->IsFunction()) {
            rounds = args[0]->Int32Value();
            callback = Local<Function>::Cast(args[1]);
        } else if (args.Length() > 2 && args[1]->IsNumber()) {
            rand_len = args[1]->Int32Value();

            if (args[2]->IsFunction()) {
                callback = Local<Function>::Cast(args[2]);
            } else {
                return ThrowException(Exception::Error(String::New("No callback supplied."))); 
            }
        } else {
            return ThrowException(Exception::Error(String::New("No callback supplied."))); 
        }
    } else {
        return ThrowException(Exception::Error(String::New("No callback supplied."))); 
    }

    salt_request *s_req = (salt_request *)malloc(sizeof(*s_req));
    if (!s_req)
        return ThrowException(Exception::Error(String::New("malloc in GenerateSalt failed.")));

    s_req->callback = Persistent<Function>::New(callback);
    s_req->rand_len = rand_len;
    s_req->rounds = rounds;
    s_req->error = NULL;

    eio_custom(EIO_GenSalt, EIO_PRI_DEFAULT, EIO_GenSaltAfter, s_req);

    ev_ref(EV_DEFAULT_UC);

    return Undefined();
}

Handle<Value> GenerateSaltSync(const Arguments& args) {
    HandleScope scope;

    int size = 20;
    if (args.Length() < 1) {
        return ThrowException(Exception::Error(String::New("Must give number of rounds.")));
    } else if (!args[0]->IsNumber()) {
        return ThrowException(Exception::Error(String::New("Param must be a number.")));
    }

    ssize_t rounds = args[0]->Int32Value();
    if (args.Length() > 1 && args[1]->IsNumber()) {
        size = args[1]->Int32Value();
    }

    u_int8_t *_seed = (u_int8_t *)malloc(size * sizeof(u_int8_t));
    switch(GetSeed(_seed, size)) {
        case -1:
            return ThrowException(Exception::Error(String::New("Rand operation not supported.")));
        case 0:
            return ThrowException(Exception::Error(String::New("Rand operation did not generate a cryptographically sound seed.")));
    }
    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, _seed, salt);
    int salt_len = strlen(salt);
    free(_seed);
    Local<Value> outString = Encode(salt, salt_len, BINARY);

    return scope.Close(outString);
}

/* ENCRYPT DATA - USED TO BE HASHPW */
#if NODE_LESS_THAN 
int EIO_Encrypt(eio_req *req) {
#else
void EIO_Encrypt(eio_req *req) {
#endif
    encrypt_request *encrypt_req = (encrypt_request *)req->data;

    if (!(ValidateSalt(encrypt_req->salt))) {
        encrypt_req->error = "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue";
#if NODE_LESS_THAN
        return 1;
#else
        return;
#endif
    }

    char* bcrypted = (char *)malloc(_PASSWORD_LEN);
    try {
      bcrypt((const char *)encrypt_req->input, (const char *)encrypt_req->salt, bcrypted);
      encrypt_req->output_len = strlen(bcrypted);
      encrypt_req->output = bcrypted;
    } catch (const char *err) {
      encrypt_req->error = err;
      free(bcrypted);
    }

#if NODE_LESS_THAN 
    return 0;
#endif
}

int EIO_EncryptAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    encrypt_request *encrypt_req = (encrypt_request *)req->data;

    Handle<Value> argv[2];

    if (encrypt_req->error != NULL) {
        argv[0] = Exception::Error(String::New(encrypt_req->error));
        argv[1] = Undefined();
    }
    else {
        argv[0] = Undefined();
        argv[1] = Encode(encrypt_req->output, encrypt_req->output_len, BINARY);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    encrypt_req->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    encrypt_req->callback.Dispose();
    free(encrypt_req->salt);
    free(encrypt_req->input);
    free(encrypt_req->output);

    free(encrypt_req);

    return 0;
}

Handle<Value> Encrypt(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 3) {
        return ThrowException(Exception::Error(String::New("Must give data, salt and callback.")));
    } else if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsFunction()) {
        return ThrowException(Exception::Error(String::New("Data and salt must be strings and the callback must be a function.")));
    }

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    encrypt_request *encrypt_req = (encrypt_request *)malloc(sizeof(*encrypt_req));
    if (!encrypt_req)
        return ThrowException(Exception::Error(String::New("malloc in Encrypt failed.")));

    encrypt_req->callback = Persistent<Function>::New(callback);
    encrypt_req->input = strdup(*data);
    encrypt_req->salt = strdup(*salt);
    encrypt_req->output = NULL;
    encrypt_req->error = NULL;

    eio_custom(EIO_Encrypt, EIO_PRI_DEFAULT, EIO_EncryptAfter, encrypt_req);

    ev_ref(EV_DEFAULT_UC);

    return Undefined();
}

Handle<Value> EncryptSync(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 2) {
        return ThrowException(Exception::Error(String::New("Must give password and salt.")));
    } else if (!args[0]->IsString() || !args[1]->IsString()) {
        return ThrowException(Exception::Error(String::New("Params must be strings.")));
    }

    String::Utf8Value data(args[0]->ToString());
    String::Utf8Value salt(args[1]->ToString());

    if (!(ValidateSalt(*salt))) {
        return ThrowException(Exception::Error(String::New("Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue")));
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*data, *salt, bcrypted);
    int bcrypted_len = strlen(bcrypted);
    Local<Value> outString = Encode(bcrypted, bcrypted_len, BINARY);

    return scope.Close(outString);
}

/* COMPARATOR */
bool CompareStrings(char* s1, char* s2) {
    bool eq = true;
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);

    if (s1_len != s2_len) {
        eq = false;
    }
    int max_len = (s2_len < s1_len)?s1_len:s2_len; 

    for (int i = 0; i < max_len; i++) {
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
    compare_request *compare_req = (compare_request *)req->data;

    try {
        char bcrypted[_PASSWORD_LEN];
        bcrypt((const char *)compare_req->input, (const char *)compare_req->encrypted, bcrypted);
        compare_req->result = CompareStrings(bcrypted, (char *)compare_req->encrypted);
    } catch (const char *err) {
        compare_req->error = err;
    }
#if NODE_LESS_THAN
    return 0;
#endif
}

int EIO_CompareAfter(eio_req *req) {
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    compare_request *compare_req = (compare_request *)req->data;

    Handle<Value> argv[2];

    if (compare_req->error) {
        argv[0] = Exception::Error(String::New(compare_req->error));
        argv[1] = Undefined();
    } else {
        argv[0] = Undefined();
        if (compare_req->result) {
          argv[1] = Boolean::New(true);
        } else {
          argv[1] = Boolean::New(false);
        }
    }

    TryCatch try_catch; // don't quite see the necessity of this

    compare_req->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    compare_req->callback.Dispose();
    free(compare_req->encrypted);
    free(compare_req->input);
    free(compare_req);

    return 0;
}

Handle<Value> Compare(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 3) {
        return ThrowException(Exception::Error(String::New("Must give input, data and callback.")));
    } else if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsFunction()) {
        return ThrowException(Exception::Error(String::New("Input and data to compare against must be strings and the callback must be a function.")));
    }

    String::Utf8Value input(args[0]->ToString());
    String::Utf8Value encrypted(args[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(args[2]);

    compare_request *compare_req = (compare_request *)malloc(sizeof(*compare_req));
    if (!compare_req)
        return ThrowException(Exception::Error(String::New("malloc in Compare failed.")));
    compare_req->callback = Persistent<Function>::New(callback);
    compare_req->input = strdup(*input);
    compare_req->encrypted = strdup(*encrypted);
    compare_req->error = NULL;

    eio_custom(EIO_Compare, EIO_PRI_DEFAULT, EIO_CompareAfter, compare_req);

    ev_ref(EV_DEFAULT_UC);

    return Undefined();
}

Handle<Value> CompareSync(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 2) {
        return ThrowException(Exception::Error(String::New("Must give password and hash.")));
    } else if (!args[0]->IsString() || !args[1]->IsString()) {
        return ThrowException(Exception::Error(String::New("Params must be strings.")));
    }

    String::Utf8Value pw(args[0]->ToString());
    String::Utf8Value hash(args[1]->ToString());

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*pw, *hash, bcrypted);
    return Boolean::New(CompareStrings(bcrypted, *hash));
}

} // anonymous namespace

// bind the bcrypt module
extern "C" void init(Handle<Object> target) {
    HandleScope scope;

    NODE_SET_METHOD(target, "gen_salt_sync", GenerateSaltSync);
    NODE_SET_METHOD(target, "encrypt_sync", EncryptSync);
    NODE_SET_METHOD(target, "compare_sync", CompareSync);
    NODE_SET_METHOD(target, "gen_salt", GenerateSalt);
    NODE_SET_METHOD(target, "encrypt", Encrypt);
    NODE_SET_METHOD(target, "compare", Compare);
};

