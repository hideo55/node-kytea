#ifndef NODE_KYTEA_ANALYZER_H_
#define NODE_KYTEA_ANALYZER_H_
#define BUILDING_NODE_EXTENSION
#include <v8.h>
#include <node.h>
#include <string>
#include <tr1/unordered_map>
#include <utility>
#include "kytea/kytea.h"
#include "kytea/kytea-struct.h"

namespace node_kytea {

class NodeKytea: node::ObjectWrap {
public:
    NodeKytea();
    ~NodeKytea();

    static void Init(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> isEnableHalf2Full(const v8::Arguments& args);
    static v8::Handle<v8::Value> getWS(const v8::Arguments& args);
    static v8::Handle<v8::Value> getTags(const v8::Arguments& args);
    static v8::Handle<v8::Value> getAllTags(const v8::Arguments& args);

    enum StatusType {
        ST_OK, ST_FAIL
    };

    struct Baton {
        uv_work_t request;
        v8::Persistent<v8::Function> callback;
        StatusType status;
        std::string message;
        NodeKytea* kt;

        Baton(NodeKytea* kt_, v8::Handle<v8::Function> cb_) :
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

    struct ReadBaton: Baton {
        std::string filename;
        ReadBaton(NodeKytea* kt_, v8::Handle<v8::Function> cb_, std::string filename_) :
            Baton(kt_, cb_), filename(filename_) {
        }
    };

    struct WsBaton: Baton {
        std::string sentence;
        kytea::KyteaSentence::Words words;
        WsBaton(NodeKytea* kt_, v8::Handle<v8::Function> cb_, std::string sentence_) :
            Baton(kt_, cb_), sentence(sentence_) {
        }
    };

    struct TagsBaton: WsBaton {
        bool all;
        TagsBaton(NodeKytea* kt_, v8::Handle<v8::Function> cb_, std::string sentence_, bool all_ = false) :
            WsBaton(kt_, cb_, sentence_), all(all_) {
        }
    };

private:
    kytea::Kytea* kytea;
    bool isModelLoaded;
    bool enableH2F;

    static void ParseConfig(v8::Handle<v8::Object> opt, kytea::KyteaConfig *config);
    static void Work_ReadModel(uv_work_t* req);
    static void Work_AfterReadModel(uv_work_t* req);
    static void Work_WS(uv_work_t* req);
    static void Work_AfterWS(uv_work_t* req);
    static void Work_Tags(uv_work_t* req);
    static void Work_AfterTags(uv_work_t* req);
};

}

#endif
