#define BUILDING_NODE_EXTENSION
#include <v8.h>
#include <node.h>
#include <string>
#include <vector>
#include "kytea/kytea.h"
#include "kytea/kytea-struct.h"

namespace node_kytea {

class Analyzer: node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> Analyze(const v8::Arguments& args);

    enum StatusType {
        ST_OK, ST_FAIL
    };

    struct Baton {
        uv_work_t request;
        v8::Persistent<v8::Function> callback;
        StatusType status;
        std::string message;
        Analyzer* kt;

        Baton(Analyzer* kt_, v8::Handle<v8::Function> cb_) :
            kt(kt_), status(ST_OK) {
            kt->Ref();
            uv_ref( uv_default_loop());
            request.data = this;
            callback = v8::Persistent<v8::Function>::New(cb_);
        }
        virtual ~Baton() {
            kt->Unref();
            uv_unref( uv_default_loop());
            callback.Dispose();
        }
    };

    struct ModelIoBaton: Baton {
        std::string filename;
        ModelIoBaton(Analyzer* kt_, v8::Handle<v8::Function> cb_, std::string filename_) :
            Baton(kt_, cb_), filename(filename_) {
        }
    };

    struct AnalyzeBaton: Baton {
        std::string sentence;
        kytea::KyteaSentence::Words words;
        AnalyzeBaton(Analyzer* kt_, v8::Handle<v8::Function> cb_, std::string sentence_) :
            Baton(kt_, cb_), sentence(sentence_) {
        }
    };

private:
    kytea::Kytea* kytea;
    char in_format;
    char out_format;

    static void ParseConfig(v8::Handle<v8::Object> opt, kytea::KyteaConfig *config);
    static void Work_ReadModel(uv_work_t* req);
    static void Work_AfterReadModel(uv_work_t* req);
    static void Work_Analyze(uv_work_t* req);
    static void Work_AfterAnalyze(uv_work_t* req);
};

}
