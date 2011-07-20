#include "node_blf.h"
#include <pthread.h>

class BCrypt : public node::ObjectWrap {
    static int EIO_GenSalt(eio_req *req);
    static int EIO_GenSaltAfter(eio_req *req);
    static int EIO_Encrypt(eio_req *req);
    static int EIO_EncryptAfter(eio_req *req);
    static int EIO_Compare(eio_req *req);
    static int EIO_CompareAfter(eio_req *req);
    public:
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> GenerateSaltSync(const v8::Arguments& args);
        static v8::Handle<v8::Value> EncryptSync(const v8::Arguments& args);
        static v8::Handle<v8::Value> CompareSync(const v8::Arguments& args);
        static v8::Handle<v8::Value> GenerateSalt(const v8::Arguments& args);
        static v8::Handle<v8::Value> Encrypt(const v8::Arguments& args);
        static v8::Handle<v8::Value> Compare(const v8::Arguments& args);
};

struct base_request {
    v8::Persistent<v8::Function> callback;
    void *bcrypt_obj;
    char *error;
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
