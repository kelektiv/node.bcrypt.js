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
#include <node_events.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "node_blf.h"

using namespace v8;
using namespace node;

class BCrypt : public ObjectWrap {
    public:
        static void
            Initialize (v8::Handle<v8::Object> target)
            {
                HandleScope scope;

                Local<FunctionTemplate> t = FunctionTemplate::New(New);

                t->InstanceTemplate()->SetInternalFieldCount(1);

                NODE_SET_PROTOTYPE_METHOD(t, "gen_salt", BCryptGenerateSalt);
                NODE_SET_PROTOTYPE_METHOD(t, "hashpw", BCryptHashPW);
                NODE_SET_PROTOTYPE_METHOD(t, "compare", BCryptCompare);

                target->Set(String::NewSymbol("BCrypt"), t->GetFunction());
            }

        char* BCryptGenerateSalt(u_int8_t rounds)
        {
            return bcrypt_gensalt(rounds);
        }

        char* BCryptHashPW(char* key, char* salt)
        {
            return bcrypt(key, salt);
        }
    protected:
        static Handle<Value>
            New (const Arguments& args)
            {
                HandleScope scope;

                BCrypt *bcrypt = new BCrypt();
                bcrypt->Wrap(args.This());

                return args.This();
            }

        static Handle<Value>
            BCryptGenerateSalt(const Arguments& args) {
                BCrypt *bcrypt = ObjectWrap::Unwrap<BCrypt>(args.This());

                HandleScope scope;

                if (args.Length() < 1) {
                    return ThrowException(Exception::Error(String::New("Must give number of rounds.")));
                } else if (!args[0]->IsNumber()) {
                    return ThrowException(Exception::Error(String::New("Param must be a number.")));
                }

                ssize_t rounds = args[0]->Int32Value();

                char* salt = bcrypt->BCryptGenerateSalt(rounds);
                int salt_len = strlen(salt);
                Local<Value> outString = Encode(salt, salt_len, BINARY);

                return scope.Close(outString);
            }

        static Handle<Value>
            BCryptHashPW(const Arguments& args) {
                BCrypt *bcrypt = ObjectWrap::Unwrap<BCrypt>(args.This());

                HandleScope scope;

                if (args.Length() < 2) {
                    return ThrowException(Exception::Error(String::New("Must give password and salt.")));
                } else if (!args[0]->IsString() || !args[1]->IsString()) {
                    return ThrowException(Exception::Error(String::New("Params must be strings.")));
                }

                String::Utf8Value pw(args[0]->ToString());
                String::Utf8Value salt(args[1]->ToString());

                char* bcrypted = bcrypt->BCryptHashPW(*pw, *salt);
                int bcrypted_len = strlen(bcrypted);
                Local<Value> outString = Encode(bcrypted, bcrypted_len, BINARY);

                return scope.Close(outString);
            }


        static Handle<Value>
            BCryptCompare(const Arguments& args) {
                BCrypt *bcrypt = ObjectWrap::Unwrap<BCrypt>(args.This());

                HandleScope scope;

                if (args.Length() < 2) {
                    return ThrowException(Exception::Error(String::New("Must give password and hash.")));
                } else if (!args[0]->IsString() || !args[1]->IsString()) {
                    return ThrowException(Exception::Error(String::New("Params must be strings.")));
                }

                String::Utf8Value pw(args[0]->ToString());
                String::Utf8Value hash(args[1]->ToString());

                return Boolean::New(strcmp(bcrypt->BCryptHashPW(*pw, *hash),*hash) == 0);
            }

        BCrypt () : ObjectWrap () 
    {
    }

        ~BCrypt ()
        {
        }
        //private:
};

    extern "C" void
init (Handle<Object> target) 
{
    HandleScope scope;

    BCrypt::Initialize(target);
};
