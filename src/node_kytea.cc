#include "node_kytea.h"
#include "node_kytea_async.h"
#include "macros.h"
#include <stdexcept>

using namespace v8;
using namespace node;
using namespace kytea;

namespace node_kytea {

Nan::Persistent<FunctionTemplate> NodeKytea::constructor_template;

void NodeKytea::Init(Handle<Object> exports) {
    Nan::HandleScope scope;
    Local<FunctionTemplate> t = Nan::New<v8::FunctionTemplate>(NodeKytea::New);
    constructor_template.Reset(t);
    t->SetClassName(Nan::New<String>("Kytea").ToLocalChecked());
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "open", NodeKytea::Open);
    Nan::SetPrototypeMethod(t, "getWS", NodeKytea::getWS);
    Nan::SetPrototypeMethod(t, "getTags", NodeKytea::getTags);
    Nan::SetPrototypeMethod(t, "getAllTags", NodeKytea::getAllTags);

    exports->Set(Nan::New<String>("Kytea").ToLocalChecked(), t->GetFunction());
}

NodeKytea::NodeKytea() :
        kytea_(new Kytea()), isModelLoaded_(false) {
}

NodeKytea::~NodeKytea() {
    delete kytea_;
}

void NodeKytea::openModel(std::string& filename) {
    kytea_->readModel(filename.c_str());
    isModelLoaded_ = true;
}

void NodeKytea::calculateWS(std::string& text, kytea::KyteaSentence::Words& words) {
    if(!isModelLoaded_){
        throw std::runtime_error("Model is not loaded");
    }

    StringUtil* util = kytea_->getStringUtil();
    KyteaString word_str = util->mapString(text);
    KyteaSentence sentence(word_str, util->normalize(word_str));
    kytea_->getConfig()->setDoWS(true);
    kytea_->calculateWS(sentence);
    words = sentence.words;
}

void NodeKytea::calculateTags(std::string& text, kytea::KyteaSentence::Words& words) {

    if(!isModelLoaded_){
        throw std::runtime_error("Model is not loaded");
    }


    StringUtil* util = kytea_->getStringUtil();
    KyteaString word_str = util->mapString(text);
    KyteaSentence sentence(word_str, util->normalize(word_str));

    kytea::KyteaConfig* config = kytea_->getConfig();
    config->setDoWS(true);
    config->setDoTags(true);

    kytea_->calculateWS(sentence);

    for (int i = 0; i < config->getNumTags(); i++) {
        if (config->getDoTag(i)){
            kytea_->calculateTags(sentence, i);
        }
    }

    words = sentence.words;
}

void NodeKytea::MakeWsResult(kytea::KyteaSentence::Words& words, Local<Array>& result) {
    Nan::HandleScope scope;
    int word_num = words.size();
    StringUtil* util = kytea_->getStringUtil();
    for (int i = 0; i < word_num; i++) {
        kytea::KyteaWord& w = words[i];
        std::string surf = util->showString(w.surface);
        result->Set(Nan::New<Integer>(i), Nan::New<String>(surf.c_str(), surf.size()).ToLocalChecked());
    }
}

void NodeKytea::MakeTagsResult(kytea::KyteaSentence::Words& words, Local<Array>& result, bool all) {
    Nan::HandleScope scope;

    int word_num = words.size();
    StringUtil* util = kytea_->getStringUtil();
    kytea::KyteaConfig* config = kytea_->getConfig();

    for (int i = 0; i < word_num; i++) {
        kytea::KyteaWord& w = words[i];
        std::string surf = util->showString(w.surface);
        Local<Object> elm = Nan::New<Object>();
        elm->Set(Nan::New<String>("surf").ToLocalChecked(), Nan::New<String>(surf.c_str(), surf.size()).ToLocalChecked());

        int tags_size = w.getNumTags();
        Local <Array> elm_tags = Nan::New<Array>(tags_size);

        for (int j = 0; j < tags_size; j++) {
            if (config->getDoTag(j) == 0) {
                elm_tags->Set(Nan::New<Integer>(j), Nan::New<Array>(0));
            } else {
                const std::vector<KyteaTag>& tags = w.getTags(j);
                int tag_size = tags.size();
                if (!all) {
                    tag_size = 1;
                }
                Local <Array> tag_set = Nan::New<Array>(tag_size);
                for (int k = 0; k < tag_size; k++) {
                    Local <Array> tag = Nan::New<Array>(2);
                    std::string tag_str = util->showString(tags[k].first);
                    tag->Set(Nan::New<Integer>(0), Nan::New<String>(tag_str.c_str(), tag_str.size()).ToLocalChecked());
                    tag->Set(Nan::New<Integer>(1), Nan::New<Number>(tags[k].second));
                    tag_set->Set(Nan::New<Integer>(k), tag);
                }
                elm_tags->Set(Nan::New<Integer>(j), tag_set);
            }
        }
        elm->Set(Nan::New<String>("tags").ToLocalChecked(), elm_tags);
        result->Set(Nan::New<Integer>(i), elm);
    }
}

NAN_METHOD(NodeKytea::New) {
    Nan::HandleScope scope;

    NodeKytea* obj = new NodeKytea();
    obj->Wrap(info.Holder());
    kytea::KyteaConfig* config = obj->kytea_->getConfig();

    if (info.Length() == 1) {
        REQ_OBJ_ARG(0);
        Local<Object> opt = info[0].As<Object>();
        ParseConfig(opt, config);
    } else if (info.Length() != 0) {
        return Nan::ThrowError("Too many arguments");
    }

    config->setOnTraining(false);
    config->setDoTags(true);
    for (int i = 0; i < config->getNumTags(); i++) {
        config->setDoTag(i, true);
    }

    info.GetReturnValue().Set(info.Holder());
}

void NodeKytea::ParseConfig(Handle<Object> opt, KyteaConfig *config) {
    Nan::HandleScope scope;
    if (opt->Get(Nan::New<String>("debug").ToLocalChecked())->ToBoolean()->IsTrue()) {
        config->setDebug(1);
    }
    CHK_OPT_INT(config, setTagMax, opt, "tagmax");
    CHK_OPT_STR(config, setDefaultTag, opt, "deftag");
    CHK_OPT_BOOL(config, setDoUnk, opt, "nounk", true);
    CHK_OPT_INT(config, setUnkBeam, opt, "unkbeam");
    CHK_OPT_STR(config, setUnkTag, opt, "unktag");
    Local<String> notag = Nan::New<String>("notag").ToLocalChecked();

    if (opt->Has(notag)) {
        if (opt->Get(notag)->IsArray()) {
            Local <Array> notag = opt->Get(notag).As<Array>();
            for (unsigned int i = 0; i < notag->Length(); i++) {
                if (notag->Get(Nan::New<Integer>(i))->IsInt32()) {

                    unsigned int tag_index = notag->Get(Nan::New<Integer>(i))->ToUint32()->Value();
                    if (tag_index < 1) {
                        Nan::ThrowTypeError("Illegal setting for \"notag\" (must be 1 or greater)");
                    } else {
                        config->setDoTag(tag_index - 1, false);
                    }
                } else {
                    Nan::ThrowTypeError("Option \"notag\" must be a array of integer");
                }
            }
        } else {
            Nan::ThrowTypeError("Option \"notag\" must be a array of integer");
        }
    }
}

NAN_METHOD(NodeKytea::Open) {
    Nan::HandleScope scope;

    REQ_STR_ARG(0);
    std::string filename = *String::Utf8Value(info[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(info.Holder());

    Nan::Callback *callback = new Nan::Callback(cb);
    Nan::AsyncQueueWorker(new OpenWorker(callback, kt, filename));
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(NodeKytea::getWS) {
    Nan::HandleScope scope;

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(info[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(info.Holder());

    Nan::Callback *callback = new Nan::Callback(cb);
    Nan::AsyncQueueWorker(new WSWorker(callback, kt, sentence));

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(NodeKytea::getTags) {
    Nan::HandleScope scope;

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(info[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(info.Holder());

    Nan::Callback *callback = new Nan::Callback(cb);
    Nan::AsyncQueueWorker(new TagWorker(callback, kt, sentence));

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(NodeKytea::getAllTags) {
    Nan::HandleScope scope;

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(info[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(info.Holder());

    Nan::Callback *callback = new Nan::Callback(cb);
    Nan::AsyncQueueWorker(new TagWorker(callback, kt, sentence, true));

    info.GetReturnValue().SetUndefined();
}

}

namespace {
void Initialize(v8::Handle<v8::Object> exports) {
    Nan::HandleScope scope;
    node_kytea::NodeKytea::Init(exports);
}
}

NODE_MODULE(kytea, Initialize)
