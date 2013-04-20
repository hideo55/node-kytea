#ifndef NODE_KYTEA_ANALYZER_H_
#define NODE_KYTEA_ANALYZER_H_
#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <string>
#include <vector>
#include <memory>
#include <tr1/memory>
#include <utility>
#include <stdexcept>
#include <kytea/kytea-model.h>
#include <kytea/dictionary.h>
#include <kytea/kytea.h>
#include <kytea/kytea-struct.h>
#include <kytea/string-util.h>

#if NODE_VERSION_AT_LEAST(0,11,0)
#define __GET_ISOLATE_FOR_NEW v8::Isolate::GetCurrent(),
#define __GET_ISOLATE_FOR_DISPOSE v8::Isolate::GetCurrent()
#else
#define __GET_ISOLATE_FOR_NEW
#define __GET_ISOLATE_FOR_DISPOSE
#endif

namespace node_kytea {

class NodeKytea: node::ObjectWrap {
public:
    NodeKytea();
    ~NodeKytea();

    static void Init(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
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
            status(ST_OK), kt(kt_) {
            kt->Ref();
            request.data = this;
            callback = v8::Persistent<v8::Function>::New(__GET_ISOLATE_FOR_NEW cb_);
        }
        virtual ~Baton() {
            kt->Unref();
            callback.Dispose(__GET_ISOLATE_FOR_DISPOSE);
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
    std::tr1::shared_ptr<kytea::Kytea> kytea;
    bool isModelLoaded;
    kytea::StringUtil* util;

    static void ParseConfig(v8::Handle<v8::Object> opt, kytea::KyteaConfig *config);
    static void Work_ReadModel(uv_work_t* req);
    static void Work_WS(uv_work_t* req);
    static void Work_Tags(uv_work_t* req);
#if NODE_VERSION_AT_LEAST(0,9,4)
    static void Work_AfterReadModel(uv_work_t* req, int status);
    static void Work_AfterWS(uv_work_t* req, int status);
    static void Work_AfterTags(uv_work_t* req, int status);
#else
    static void Work_AfterReadModel(uv_work_t* req);
    static void Work_AfterWS(uv_work_t* req);
    static void Work_AfterTags(uv_work_t* req);
#endif
};

}

#endif
