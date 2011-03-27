#include "node_blf.h"

class BCrypt : public node::ObjectWrap {
    static int EIO_GenSalt(eio_req *req);
    static int EIO_GenSaltAfter(eio_req *req);
    static int GetSeed(u_int8_t *seed, int size);
    public:
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> GenerateSaltSync(const v8::Arguments& args);
        static v8::Handle<v8::Value> HashPWSync(const v8::Arguments& args);
        static v8::Handle<v8::Value> CompareSync(const v8::Arguments& args);
        /*static v8::Handle<v8::Value> GenerateSalt(const v8::Arguments& args);
        static v8::Handle<v8::Value> HashPW(const v8::Arguments& args);
        static v8::Handle<v8::Value> Compare(const v8::Arguments& args);*/
};
