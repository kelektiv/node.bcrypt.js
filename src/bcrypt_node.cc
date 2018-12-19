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
        case 'b':
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

char ToCharVersion(Local<String> str) {
  //String::Utf8Value value(str);

  std::string our_str = *Nan::Utf8String(str);

  return our_str[0];
}

/* SALT GENERATION */

class SaltAsyncWorker : public Nan::AsyncWorker {
public:
    SaltAsyncWorker(Nan::Callback *callback, std::string seed, ssize_t rounds, char minor_ver)
        : Nan::AsyncWorker(callback, "bcrypt:SaltAsyncWorker"), seed(seed),
          rounds(rounds), minor_ver(minor_ver) {
    }

    ~SaltAsyncWorker() {}

    void Execute() {
        char salt[_SALT_LEN];
        bcrypt_gensalt(minor_ver, rounds, (u_int8_t *)&seed[0], salt);
        this->salt = std::string(salt);
    }

    void HandleOKCallback() {
        Nan::HandleScope scope;

        Local<Value> argv[2];
        argv[0] = Nan::Undefined();
        argv[1] = Nan::Encode(salt.c_str(), salt.size(), Nan::BINARY);
        callback->Call(2, argv, async_resource);
    }

private:
    std::string seed;
    std::string salt;
    ssize_t rounds;
    char minor_ver;
};

NAN_METHOD(GenerateSalt) {
    Nan::HandleScope scope;

    if (info.Length() < 4) {
        Nan::ThrowTypeError("4 arguments expected");
        return;
    }

    if(!info[0]->IsString()) {
        Nan::ThrowTypeError("First argument must be a string");
        return;
    }

    if (!Buffer::HasInstance(info[2]) || Buffer::Length(info[2].As<Object>()) != 16) {
        Nan::ThrowTypeError("Third argument must be a 16 byte Buffer");
        return;
    }

    const char minor_ver = ToCharVersion(Nan::To<v8::String>(info[0]).ToLocalChecked());
    const int32_t rounds = Nan::To<int32_t>(info[1]).FromMaybe(0);
    Local<Object> seed = info[2].As<Object>();
    Local<Function> callback = Local<Function>::Cast(info[3]);

    SaltAsyncWorker* saltWorker = new SaltAsyncWorker(new Nan::Callback(callback),
        std::string(Buffer::Data(seed), 16), rounds, minor_ver);

    Nan::AsyncQueueWorker(saltWorker);
}

NAN_METHOD(GenerateSaltSync) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("2 arguments expected");
        return;
    }

    if(!info[0]->IsString()) {
        Nan::ThrowTypeError("First argument must be a string");
        return;
    }

    if (!Buffer::HasInstance(info[2]) || Buffer::Length(info[2].As<Object>()) != 16) {
        Nan::ThrowTypeError("Third argument must be a 16 byte Buffer");
        return;
    }

    const char minor_ver = ToCharVersion(Nan::To<v8::String>(info[0]).ToLocalChecked());
    const int32_t rounds = Nan::To<int32_t>(info[1]).FromMaybe(0);
    u_int8_t* seed = (u_int8_t*)Buffer::Data(info[2].As<Object>());

    char salt[_SALT_LEN];
    bcrypt_gensalt(minor_ver, rounds, seed, salt);

    info.GetReturnValue().Set(Nan::Encode(salt, strlen(salt), Nan::BINARY));
}

/* ENCRYPT DATA - USED TO BE HASHPW */

class EncryptAsyncWorker : public Nan::AsyncWorker {
  public:
    EncryptAsyncWorker(Nan::Callback *callback, std::string input, std::string salt)
        : Nan::AsyncWorker(callback, "bcrypt:EncryptAsyncWorker"), input(input),
          salt(salt) {
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
        Nan::HandleScope scope;

        Local<Value> argv[2];

        if (!error.empty()) {
            argv[0] = Nan::Error(error.c_str());
            argv[1] = Nan::Undefined();
        } else {
            argv[0] = Nan::Undefined();
            argv[1] = Nan::Encode(output.c_str(), output.size(), Nan::BINARY);
        }

        callback->Call(2, argv, async_resource);
    }

  private:
    std::string input;
    std::string salt;
    std::string error;
    std::string output;
};

NAN_METHOD(Encrypt) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("3 arguments expected");
        return;
    }

    Nan::Utf8String data(Nan::To<v8::String>(info[0]).ToLocalChecked());
    Nan::Utf8String salt(Nan::To<v8::String>(info[1]).ToLocalChecked());
    Local<Function> callback = Local<Function>::Cast(info[2]);

    EncryptAsyncWorker* encryptWorker = new EncryptAsyncWorker(new Nan::Callback(callback),
        std::string(*data), std::string(*salt));

    Nan::AsyncQueueWorker(encryptWorker);
}

NAN_METHOD(EncryptSync) {
    Nan::HandleScope scope;

    if (info.Length() < 2) {
        Nan::ThrowTypeError("2 arguments expected");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    Nan::Utf8String data(Nan::To<v8::String>(info[0]).ToLocalChecked());
    Nan::Utf8String salt(Nan::To<v8::String>(info[1]).ToLocalChecked());

    if (!(ValidateSalt(*salt))) {
        Nan::ThrowError("Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*data, *salt, bcrypted);
    info.GetReturnValue().Set(Nan::Encode(bcrypted, strlen(bcrypted), Nan::BINARY));
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

class CompareAsyncWorker : public Nan::AsyncWorker {
  public:
    CompareAsyncWorker(Nan::Callback *callback, std::string input, std::string encrypted)
        : Nan::AsyncWorker(callback, "bcrypt:CompareAsyncWorker"), input(input),
          encrypted(encrypted) {

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
        Nan::HandleScope scope;

        Local<Value> argv[2];
        argv[0] = Nan::Undefined();
        argv[1] = Nan::New<Boolean>(result);
        callback->Call(2, argv, async_resource);
    }

  private:
    std::string input;
    std::string encrypted;
    bool result;
};

NAN_METHOD(Compare) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("3 arguments expected");
        return;
    }

    Nan::Utf8String input(Nan::To<v8::String>(info[0]).ToLocalChecked());
    Nan::Utf8String encrypted(Nan::To<v8::String>(info[1]).ToLocalChecked());
    Local<Function> callback = Local<Function>::Cast(info[2]);

    CompareAsyncWorker* compareWorker = new CompareAsyncWorker(new Nan::Callback(callback),
        std::string(*input), std::string(*encrypted));

    Nan::AsyncQueueWorker(compareWorker);
}

NAN_METHOD(CompareSync) {
    Nan::HandleScope scope;

    if (info.Length() < 2) {
        Nan::ThrowTypeError("2 arguments expected");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    Nan::Utf8String pw(Nan::To<v8::String>(info[0]).ToLocalChecked());
    Nan::Utf8String hash(Nan::To<v8::String>(info[1]).ToLocalChecked());

    char bcrypted[_PASSWORD_LEN];
    if (ValidateSalt(*hash)) {
        bcrypt(*pw, *hash, bcrypted);
        info.GetReturnValue().Set(Nan::New<Boolean>(CompareStrings(bcrypted, *hash)));
    } else {
        info.GetReturnValue().Set(Nan::False());
    }
}

NAN_METHOD(GetRounds) {
    Nan::HandleScope scope;

    if (info.Length() < 1) {
        Nan::ThrowTypeError("1 argument expected");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    Nan::Utf8String hash(Nan::To<v8::String>(info[0]).ToLocalChecked());
    u_int32_t rounds;
    if (!(rounds = bcrypt_get_rounds(*hash))) {
        Nan::ThrowError("invalid hash provided");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    info.GetReturnValue().Set(Nan::New(rounds));
}

} // anonymous namespace

NAN_MODULE_INIT(init) {
    Nan::Export(target, "gen_salt_sync", GenerateSaltSync);
    Nan::Export(target, "encrypt_sync", EncryptSync);
    Nan::Export(target, "compare_sync", CompareSync);
    Nan::Export(target, "get_rounds", GetRounds);
    Nan::Export(target, "gen_salt", GenerateSalt);
    Nan::Export(target, "encrypt", Encrypt);
    Nan::Export(target, "compare", Compare);
};

NODE_MODULE(bcrypt_lib, init);
