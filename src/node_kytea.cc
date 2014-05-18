#include "node_kytea.h"
#include "node_kytea_async.h"
#include "macros.h"
#include <stdexcept>

using namespace v8;
using namespace node;
using namespace kytea;

namespace node_kytea {

Persistent<FunctionTemplate> NodeKytea::constructor_template;

void NodeKytea::Init(Handle<Object> exports) {
    NanScope();
    Local<FunctionTemplate> t = NanNew<v8::FunctionTemplate>(NodeKytea::New);
    t->SetClassName(NanNew<String>("Kytea"));
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "open", NodeKytea::Open);
    NODE_SET_PROTOTYPE_METHOD(t, "getWS", NodeKytea::getWS);
    NODE_SET_PROTOTYPE_METHOD(t, "getTags", NodeKytea::getTags);
    NODE_SET_PROTOTYPE_METHOD(t, "getAllTags", NodeKytea::getAllTags);

    NanAssignPersistent(constructor_template, t);
    exports->Set(NanNew<String>("Kytea"), t->GetFunction());
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
    NanScope();
    int word_num = words.size();
    StringUtil* util = kytea_->getStringUtil();
    for (int i = 0; i < word_num; i++) {
        kytea::KyteaWord& w = words[i];
        std::string surf = util->showString(w.surface);
        result->Set(NanNew<Integer>(i), NanNew<String>(surf.c_str(), surf.size()));
    }
}

void NodeKytea::MakeTagsResult(kytea::KyteaSentence::Words& words, Local<Array>& result, bool all) {
    NanScope();

    int word_num = words.size();
    StringUtil* util = kytea_->getStringUtil();
    kytea::KyteaConfig* config = kytea_->getConfig();

    for (int i = 0; i < word_num; i++) {
        kytea::KyteaWord& w = words[i];
        std::string surf = util->showString(w.surface);
        Local<Object> elm = NanNew<Object>();
        elm->Set(NanNew<String>("surf"), NanNew<String>(surf.c_str(), surf.size()));

        int tags_size = w.getNumTags();
        Local <Array> elm_tags = NanNew<Array>(tags_size);

        for (int j = 0; j < tags_size; j++) {
            if (config->getDoTag(j) == 0) {
                elm_tags->Set(NanNew<Integer>(j), NanNew<Array>(0));
            } else {
                const std::vector<KyteaTag>& tags = w.getTags(j);
                int tag_size = tags.size();
                if (!all) {
                    tag_size = 1;
                }
                Local <Array> tag_set = NanNew<Array>(tag_size);
                for (int k = 0; k < tag_size; k++) {
                    Local <Array> tag = NanNew<Array>(2);
                    std::string tag_str = util->showString(tags[k].first);
                    tag->Set(NanNew<Integer>(0), NanNew<String>(tag_str.c_str(), tag_str.size()));
                    tag->Set(NanNew<Integer>(1), NanNew<Number>(tags[k].second));
                    tag_set->Set(NanNew<Integer>(k), tag);
                }
                elm_tags->Set(NanNew<Integer>(j), tag_set);
            }
        }
        elm->Set(NanNew<String>("tags"), elm_tags);
        result->Set(NanNew<Integer>(i), elm);
    }
}

NAN_METHOD(NodeKytea::New) {
    NanScope();

    NodeKytea* obj = new NodeKytea();
    obj->Wrap(args.Holder());
    kytea::KyteaConfig* config = obj->kytea_->getConfig();

    if (args.Length() == 1) {
        REQ_OBJ_ARG(0);
        Local<Object> opt = args[0].As<Object>();
        ParseConfig(opt, config);
    } else if (args.Length() != 0) {
        return NanThrowError("Too many arguments");
    }

    config->setOnTraining(false);
    config->setDoTags(true);
    for (int i = 0; i < config->getNumTags(); i++) {
        config->setDoTag(i, true);
    }

    NanReturnValue(args.Holder());
}

void NodeKytea::ParseConfig(Handle<Object> opt, KyteaConfig *config) {
    NanScope();
    if (opt->Get(NanNew<String>("debug"))->ToBoolean()->IsTrue()) {
        config->setDebug(1);
    }
    CHK_OPT_INT(config, setTagMax, opt, "tagmax");
    CHK_OPT_STR(config, setDefaultTag, opt, "deftag");
    CHK_OPT_BOOL(config, setDoUnk, opt, "nounk", true);
    CHK_OPT_INT(config, setUnkBeam, opt, "unkbeam");
    CHK_OPT_STR(config, setUnkTag, opt, "unktag");

    if (opt->Has(NanNew<String>("notag"))) {
        if (opt->Get(NanNew<String>("notag"))->IsArray()) {
            Local <Array> notag = opt->Get(NanNew<String>("notag")).As<Array>();
            for (unsigned int i = 0; i < notag->Length(); i++) {
                if (notag->Get(NanNew<Integer>(i))->IsInt32()) {

                    unsigned int tag_index = notag->Get(NanNew<Integer>(i))->ToUint32()->Value();
                    if (tag_index < 1) {
                        NanThrowTypeError("Illegal setting for \"notag\" (must be 1 or greater)");
                    } else {
                        config->setDoTag(tag_index - 1, false);
                    }
                } else {
                    NanThrowTypeError("Option \"notag\" must be a array of integer");
                }
            }
        } else {
            NanThrowTypeError("Option \"notag\" must be a array of integer");
        }
    }
}

NAN_METHOD(NodeKytea::Open) {
    NanScope();

    REQ_STR_ARG(0);
    std::string filename = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new OpenWorker(callback, kt, filename));
    NanReturnUndefined();
}

NAN_METHOD(NodeKytea::getWS) {
    NanScope();

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new WSWorker(callback, kt, sentence));

    NanReturnUndefined();
}

NAN_METHOD(NodeKytea::getTags) {
    NanScope();

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new TagWorker(callback, kt, sentence));

    NanReturnUndefined();
}

NAN_METHOD(NodeKytea::getAllTags) {
    NanScope();

    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);

    NodeKytea* kt = Unwrap<NodeKytea>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new TagWorker(callback, kt, sentence, true));

    NanReturnUndefined();
}

}

namespace {
void Initialize(v8::Handle<v8::Object> exports) {
    NanScope();
    node_kytea::NodeKytea::Init(exports);
}
}

NODE_MODULE(kytea, Initialize)
