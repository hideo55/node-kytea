#ifndef NODE_KYTEA_ANALYZER_H_
#define NODE_KYTEA_ANALYZER_H_
#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <nan.h>
#include <string>
#include <vector>
#include <kytea/kytea-model.h>
#include <kytea/dictionary.h>
#include <kytea/kytea.h>
#include <kytea/kytea-struct.h>
#include <kytea/string-util.h>

namespace node_kytea {

class NodeKytea: public node::ObjectWrap {
public:
    NodeKytea();
    ~NodeKytea();

    static void Init(v8::Handle<v8::Object> exports);
    static NAN_METHOD(New);
    static NAN_METHOD(Open);
    static NAN_METHOD(getWS);
    static NAN_METHOD(getTags);
    static NAN_METHOD(getAllTags);

    void openModel(std::string& filename);

    void calculateWS(std::string& text, kytea::KyteaSentence::Words& words);

    void calculateTags(std::string& text, kytea::KyteaSentence::Words& words);

    void MakeWsResult(kytea::KyteaSentence::Words& words, v8::Local<v8::Array>& result);

    void MakeTagsResult(kytea::KyteaSentence::Words& words, v8::Local<v8::Array>& result, bool all);

private:
    static Nan::Persistent<v8::FunctionTemplate> constructor_template;
    kytea::Kytea* kytea_;
    bool isModelLoaded_;

    static void ParseConfig(v8::Handle<v8::Object> opt, kytea::KyteaConfig *config);
};

}

#endif
