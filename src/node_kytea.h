#define BUILDING_NODE_EXTENSION
#ifndef NODE_KYTEA_H
#define NODE_KYTEA_H

#include <string>
#include <v8.h>
#include <node.h>
#include "kytea/kytea.h"
#include "kytea/kytea-struct.h"

using namespace v8;
using namespace node;
using namespace kytea;

namespace node_kytea {

class NodeKyTea: node::ObjectWrap {
public:
    NodeKyTea();
    ~NodeKyTea();

    enum StatusType {
        ST_OK, ST_FAIL
    };

    struct Baton {
        uv_work_t request;
        NodeKyTea* kt;
        Persistent<Function> callback;
        StatusType status;
        std::string message;

        Baton(NodeKyTea* kt_, Handle<Function> cb_) :
            kt(kt_), status(ST_OK) {
            kt->Ref();
            uv_ref( uv_default_loop());
            request.data = this;
            callback = Persistent<Function>::New(cb_);
        }
        virtual ~Baton() {
            kt->Unref();
            uv_unref( uv_default_loop());
            callback.Dispose();
        }
    };

    struct ModelIoBaton: Baton {
        std::string filename;
        ModelIoBaton(NodeKyTea* kt_, Handle<Function> cb_, std::string filename_) :
            Baton(kt_, cb_), filename(filename_) {
        }
    };

    struct ExecBaton: Baton {
        std::string sentence;
        KyteaSentence::Words words;
        ExecBaton(NodeKyTea* kt_, Handle<Function> cb_, std::string sentence_) :
            Baton(kt_, cb_), sentence(sentence_) {
        }
    };

    static void Init(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> ReadModel(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteModel(const v8::Arguments& args);
    static v8::Handle<v8::Value> Analyze(const v8::Arguments& args);
    static v8::Handle<v8::Value> TrainAll(const v8::Arguments& args);

private:
    Kytea* kytea;

    static void Work_ReadModel(uv_work_t* req);
    static void Work_AfterReadModel(uv_work_t* req);
    static void Work_Analyze(uv_work_t* req);
    static void Work_AfterAnalyze(uv_work_t* req);
};

}
#endif /* NODE_KYTEA_H */
