#include <napi.h>

#include <string>
#include <cstring>
#include <vector>
#include <stdlib.h> // atoi

#include "node_blf.h"

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

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

class SaltAsyncWorker : public Napi::AsyncWorker {
    public:
        SaltAsyncWorker(Napi::Function& callback, std::string seed, ssize_t rounds)
            : Napi::AsyncWorker(callback), seed(seed), rounds(rounds) {
        }

        ~SaltAsyncWorker() {}

        void Execute() {
            char salt[_SALT_LEN];
            bcrypt_gensalt((char) rounds, (u_int8_t *)&seed[0], salt);
            this->salt = std::string(salt);
        }

        void OnOK() {
            Napi::HandleScope scope(Env());   
            Callback().Call({Env().Undefined(), Napi::String::New(Env(), salt)});
        }

    private:
        std::string seed;
        std::string salt;
        ssize_t rounds;
        
};

Napi::Value GenerateSalt(const Napi::CallbackInfo& info) {
    if (info.Length() < 3) {
        throw Napi::TypeError::New(info.Env(), "3 arguments expected");
    }
    if (!info[1].IsBuffer() || (info[1].As<Napi::Buffer<char>>()).Length() != 16) {
        throw Napi::TypeError::New(info.Env(), "Second argument must be a 16 byte Buffer");
    }
    const int32_t rounds = info[0].As<Napi::Number>();
    Napi::Function callback = info[2].As<Napi::Function>();
    Napi::Buffer<char> seed = info[1].As<Napi::Buffer<char>>();
    SaltAsyncWorker* saltWorker = new SaltAsyncWorker(callback, std::string(seed.Data(), 16), rounds);
    saltWorker->Queue();
    return info.Env().Undefined();
}

Napi::Value GenerateSaltSync (const Napi::CallbackInfo& info) {
    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "2 arguments expected");
    }
    if (!info[1].IsBuffer() || (info[1].As<Napi::Buffer<char>>()).Length() != 16) {
        throw Napi::TypeError::New(info.Env(), "Second argument must be a 16 byte Buffer");
    }
    const int32_t rounds = info[0].As<Napi::Number>();
    Napi::Buffer<u_int8_t> buffer = info[1].As<Napi::Buffer<u_int8_t>>();
    u_int8_t* seed = (u_int8_t*) buffer.Data();
    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, seed, salt);
    return Napi::String::New(info.Env(), salt, strlen(salt));
}

/* ENCRYPT DATA - USED TO BE HASHPW */

class EncryptAsyncWorker : public Napi::AsyncWorker {
  public:
    EncryptAsyncWorker(Napi::Function& callback, std::string input, std::string salt)
        : Napi::AsyncWorker(callback), input(input), salt(salt) {
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

    void OnOK() {
        Napi::HandleScope scope(Env());  
        if (!error.empty()) {
            Callback().Call({
                Napi::Error::New(Env(), error.c_str()).Value(),
                Env().Undefined()                       
            });
        } else {
            Callback().Call({
                Env().Undefined(), 
                Napi::String::New(Env(), output)
            });
        }   
    }

  private:
    std::string input;
    std::string salt;
    std::string error;
    std::string output;
};

Napi::Value Encrypt(const Napi::CallbackInfo& info) {
    if (info.Length() < 3) {
        throw Napi::TypeError::New(info.Env(), "3 arguments expected");
    }
    std::string data = info[0].As<Napi::String>();;
    std::string salt = info[1].As<Napi::String>();;
    Napi::Function callback = info[2].As<Napi::Function>();
    EncryptAsyncWorker* encryptWorker = new EncryptAsyncWorker(callback, data, salt);
    encryptWorker->Queue();
    return info.Env().Undefined();
}

Napi::Value EncryptSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "2 arguments expected");   
    }
    std::string data = info[0].As<Napi::String>();;
    std::string salt = info[1].As<Napi::String>();;
    if (!(ValidateSalt(salt.c_str()))) {
        throw Napi::Error::New(info.Env(), "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
    }
    char bcrypted[_PASSWORD_LEN];
    bcrypt(data.c_str(), salt.c_str(), bcrypted);
    return Napi::String::New(env, bcrypted, strlen(bcrypted));
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

class CompareAsyncWorker : public Napi::AsyncWorker {
  public:
    CompareAsyncWorker(Napi::Function& callback, std::string input, std::string encrypted)
        : Napi::AsyncWorker(callback), input(input), encrypted(encrypted) {
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

    void OnOK() {
        Napi::HandleScope scope(Env());   
        Callback().Call({Env().Undefined(), Napi::Boolean::New(Env(), result)});      
    }

  private:
    std::string input;
    std::string encrypted;
    bool result;
};

Napi::Value Compare(const Napi::CallbackInfo& info) {
    if (info.Length() < 3) {
        throw Napi::TypeError::New(info.Env(), "3 arguments expected");
    }
    std::string input = info[0].As<Napi::String>();
    std::string encrypted = info[1].As<Napi::String>();
    Napi::Function callback = info[2].As<Napi::Function>();
    CompareAsyncWorker* compareWorker = new CompareAsyncWorker(callback, input, encrypted);
    compareWorker->Queue();
    return info.Env().Undefined();
}

Napi::Value CompareSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "2 arguments expected");    
    }
    std::string pw = info[0].As<Napi::String>();
    std::string hash = info[1].As<Napi::String>();
    char bcrypted[_PASSWORD_LEN];
    if (ValidateSalt(hash.c_str())) {
        bcrypt(pw.c_str(), hash.c_str(), bcrypted);
        return Napi::Boolean::New(env, CompareStrings(bcrypted, hash.c_str()));
    } else {
        return Napi::Boolean::New(env, false);
    }
}

Napi::Value GetRounds(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "1 argument expected");    
    }
    Napi::String hashed = info[0].As<Napi::String>();
    std::string hash = hashed.ToString();
    const char* bcrypt_hash = hash.c_str();
    u_int32_t rounds;
    if (!(rounds = bcrypt_get_rounds(bcrypt_hash))) {
        throw Napi::Error::New(info.Env(), "invalid hash provided");
    }
    return Napi::Number::New(env, rounds);
}

} // anonymous namespace

Napi::Object init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "gen_salt_sync"), Napi::Function::New(env, GenerateSaltSync));
    exports.Set(Napi::String::New(env, "encrypt_sync"), Napi::Function::New(env, EncryptSync));
    exports.Set(Napi::String::New(env, "compare_sync"), Napi::Function::New(env, CompareSync));
    exports.Set(Napi::String::New(env, "get_rounds"), Napi::Function::New(env, GetRounds));
    exports.Set(Napi::String::New(env, "gen_salt"), Napi::Function::New(env, GenerateSalt));
    exports.Set(Napi::String::New(env, "encrypt"), Napi::Function::New(env, Encrypt));
    exports.Set(Napi::String::New(env, "compare"), Napi::Function::New(env, Compare));
    return exports;
};

NODE_API_MODULE(bcrypt_lib, init);
