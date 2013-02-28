#include "node_kytea.h"
#include "macros.h"

using namespace node_kytea;
using namespace v8;
using namespace node;
using namespace kytea;

namespace std {
using namespace tr1;
}

void NodeKytea::Init(Handle<Object> target) {
    HandleScope scope;
    Local < FunctionTemplate > t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("Kytea"));
    t->InstanceTemplate()->SetInternalFieldCount(4);

    NODE_SET_PROTOTYPE_METHOD(t, "getWS", getWS);
    NODE_SET_PROTOTYPE_METHOD(t, "getTags", getTags);
    NODE_SET_PROTOTYPE_METHOD(t, "getAllTags", getAllTags);

    Persistent < Function > constructor = Persistent < Function > ::New(t->GetFunction());
    target->Set(String::NewSymbol("Kytea"), constructor);
}

NodeKytea::NodeKytea() :
        isModelLoaded(false) {
}

NodeKytea::~NodeKytea() {
}

Handle<Value> NodeKytea::New(const Arguments& args) {
    HandleScope scope;

    REQ_STR_ARG(0);
    std::string filename = *String::Utf8Value(args[0]->ToString());

    Local < Function > cb;

    NodeKytea* obj = new NodeKytea();
    obj->Wrap(args.Holder());
    std::shared_ptr < Kytea > ptr(new Kytea());
    obj->kytea = ptr;
    obj->util = ptr->getStringUtil();

    kytea::KyteaConfig* config = obj->kytea->getConfig();

    if (args.Length() == 2) {
        REQ_ARG_COUNT_AND_TYPE(1, Function);
        cb = Local < Function > ::Cast(args[1]);
    } else if (args.Length() == 3) {
        REQ_OBJ_ARG(1);
        Local < Object > opt = Local < Object > ::Cast(args[1]);
        ParseConfig(opt, config);
        REQ_ARG_COUNT_AND_TYPE(2, Function);
        cb = Local < Function > ::Cast(args[2]);
    } else {
        ThrowException(Exception::RangeError(String::New("Invalid Argument")));
    }

    config->setOnTraining(false);
    config->setDoTags(true);
    for (int i = 0; i < config->getNumTags(); i++) {
        config->setDoTag(i, true);
    }

    ReadBaton* baton = new ReadBaton(obj, cb, filename);
    uv_queue_work(uv_default_loop(), &baton->request, Work_ReadModel, Work_AfterReadModel);

    return scope.Close(args.Holder());
}

void NodeKytea::ParseConfig(Handle<Object> opt, KyteaConfig *config) {
    if (opt->Get(String::New("debug"))->ToBoolean()->IsTrue()) {
        config->setDebug(1);
    }
    CHK_OPT_INT(config, setTagMax, opt, "tagmax");
    CHK_OPT_STR(config, setDefaultTag, opt, "deftag");
    CHK_OPT_BOOL(config, setDoUnk, opt, "nounk", true);
    CHK_OPT_INT(config, setUnkBeam, opt, "unkbeam");
    CHK_OPT_STR(config, setUnkTag, opt, "unktag");

    if (opt->Has(String::New("notag"))) {
        if (opt->Get(String::New("notag"))->IsArray()) {
            Local < Array > notag = Array::Cast(*(opt->Get(String::New("notag"))));
            for (unsigned int i = 0; i < notag->Length(); i++) {
                if (notag->Get(Integer::New(i))->IsInt32()) {

                    unsigned int tag_index = notag->Get(Integer::New(i))->ToUint32()->Value();
                    if (tag_index < 1) {
                        ThrowException(
                                Exception::TypeError(
                                        String::New("Illegal setting for \"notag\" (must be 1 or greater)")));
                    } else {
                        config->setDoTag(tag_index - 1, false);
                    }
                } else {
                    ThrowException(Exception::TypeError(String::New("Option \"notag\" must be a array of integer")));
                }
            }
        } else {
            ThrowException(Exception::TypeError(String::New("Option \"notag\" must be a array of integer")));
        }
    }
}

void NodeKytea::Work_ReadModel(uv_work_t* req) {
    ReadBaton* baton = static_cast<ReadBaton*>(req->data);
    NodeKytea* kt = baton->kt;
    try {
        kt->kytea->readModel(baton->filename.c_str());
    } catch (std::runtime_error& e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

#if NODE_VERSION_AT_LEAST(0,9,4)
void NodeKytea::Work_AfterReadModel(uv_work_t* req, int status) {
#else
void NodeKytea::Work_AfterReadModel(uv_work_t* req) {
#endif
    HandleScope scope;
    std::auto_ptr<ReadBaton> baton = std::auto_ptr < ReadBaton > (static_cast<ReadBaton*>(req->data));
    NodeKytea* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[1];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        kt->isModelLoaded = true;
        argv[0] = Local < Value > ::New(Null());
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 1, argv);
    }
}

Handle<Value> NodeKytea::getWS(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    NodeKytea* kt = Unwrap < NodeKytea > (args.Holder());
    if (kt->isModelLoaded) {
        kytea::KyteaConfig* config = kt->kytea->getConfig();
        config->setDoWS(true);
        WsBaton* baton = new WsBaton(kt, cb, sentence);
        uv_queue_work(uv_default_loop(), &baton->request, Work_WS, Work_AfterWS);
    } else {
        const unsigned int argc = 1;
        Local < Value > argv[1];
        argv[0] = Exception::Error(v8::String::New("Model in not loaded"));
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(args.Holder());
}

void NodeKytea::Work_WS(uv_work_t* req) {
    WsBaton* baton = static_cast<WsBaton*>(req->data);
    NodeKytea* kt = baton->kt;
    try {
        StringUtil* util = kt->kytea->getStringUtil();
        KyteaString word_str = util->mapString(baton->sentence);
        KyteaSentence sentence(word_str, util->normalize(word_str));
        kt->kytea->calculateWS(sentence);
        baton->words = sentence.words;
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

#if NODE_VERSION_AT_LEAST(0,9,4)
void NodeKytea::Work_AfterWS(uv_work_t* req, int status) {
#else
void NodeKytea::Work_AfterWS(uv_work_t* req) {
#endif
    HandleScope scope;
    std::auto_ptr<WsBaton> baton = std::auto_ptr < WsBaton > (static_cast<WsBaton*>(req->data));
    NodeKytea* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[2];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local < Value > ::New(Null());
        KyteaSentence::Words words = baton->words;
        int word_num = words.size();
        Local < Array > res(Array::New(word_num));
        StringUtil* util = kt->kytea->getStringUtil();
        for (int i = 0; i < word_num; i++) {
            kytea::KyteaWord& w = words[i];
            std::string surf = util->showString(w.surface);
            res->Set(Integer::New(i), String::New(surf.c_str(), surf.size()));
        }
        argv[1] = res;
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 2, argv);
    }
}

Handle<Value> NodeKytea::getTags(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    NodeKytea* kt = Unwrap < NodeKytea > (args.Holder());
    if (kt->isModelLoaded) {
        kytea::KyteaConfig* config = kt->kytea->getConfig();
        config->setDoWS(true);
        config->setDoTags(true);
        TagsBaton* baton = new TagsBaton(kt, cb, sentence);
        uv_queue_work(uv_default_loop(), &baton->request, Work_Tags, Work_AfterTags);
    } else {
        const unsigned int argc = 1;
        Local < Value > argv[1];
        argv[0] = Exception::Error(v8::String::New("Model in not loaded"));
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(args.Holder());
}

Handle<Value> NodeKytea::getAllTags(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    NodeKytea* kt = Unwrap < NodeKytea > (args.Holder());
    if (kt->isModelLoaded) {
        kytea::KyteaConfig* config = kt->kytea->getConfig();
        config->setDoWS(true);
        config->setDoTags(true);
        TagsBaton* baton = new TagsBaton(kt, cb, sentence, true);
        uv_queue_work(uv_default_loop(), &baton->request, Work_Tags, Work_AfterTags);
    } else {
        const unsigned int argc = 1;
        Local < Value > argv[1];
        argv[0] = Exception::Error(v8::String::New("Model in not loaded"));
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(args.Holder());
}

void NodeKytea::Work_Tags(uv_work_t* req) {
    TagsBaton* baton = static_cast<TagsBaton*>(req->data);
    NodeKytea* kt = baton->kt;
    try {
        StringUtil* util = kt->kytea->getStringUtil();
        KyteaConfig* config = kt->kytea->getConfig();
        KyteaString word_str = util->mapString(baton->sentence);
        KyteaSentence sentence(word_str, util->normalize(word_str));
        kt->kytea->calculateWS(sentence);

        for (int i = 0; i < config->getNumTags(); i++) {
            if (config->getDoTag(i))
                kt->kytea->calculateTags(sentence, i);
        }

        baton->words = sentence.words;
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

#if NODE_VERSION_AT_LEAST(0,9,4)
void NodeKytea::Work_AfterTags(uv_work_t* req, int status) {
#else
void NodeKytea::Work_AfterTags(uv_work_t* req) {
#endif
    HandleScope scope;
    std::auto_ptr<TagsBaton> baton = std::auto_ptr < TagsBaton > (static_cast<TagsBaton*>(req->data));
    NodeKytea* kt = baton->kt;
    KyteaConfig* config = kt->kytea->getConfig();
    std::string msg = baton->message;

    Local < Value > argv[2];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local < Value > ::New(Null());
        KyteaSentence::Words words = baton->words;
        int word_num = words.size();
        Local < Array > res(Array::New(word_num));
        StringUtil* util = kt->kytea->getStringUtil();

        for (int i = 0; i < word_num; i++) {
            kytea::KyteaWord& w = words[i];
            std::string surf = util->showString(w.surface);
            Local < Object > elm = Object::New();
            elm->Set(String::New("surf"), String::New(surf.c_str(), surf.size()));

            int tags_size = w.getNumTags();
            Local < Array > elm_tags = Array::New(tags_size);

            for (int j = 0; j < tags_size; j++) {
                if (config->getDoTag(j) == 0) {
                    elm_tags->Set(Integer::New(j), Array::New(0));
                } else {
                    const std::vector<KyteaTag>& tags = w.getTags(j);
                    int tag_size = tags.size();
                    if (!baton->all) {
                        tag_size = 1;
                    }
                    Local < Array > tag_set(Array::New(tag_size));
                    for (int k = 0; k < tag_size; k++) {
                        Local < Array > tag(Array::New(2));
                        std::string tag_str = util->showString(tags[k].first);
                        tag->Set(Integer::New(0), String::New(tag_str.c_str(), tag_str.size()));
                        tag->Set(Integer::New(1), Number::New(tags[k].second));
                        tag_set->Set(Integer::New(k), tag);
                    }
                    elm_tags->Set(Integer::New(j), tag_set);
                }
            }
            elm->Set(String::New("tags"), elm_tags);
            res->Set(Integer::New(i), elm);
        }

        argv[1] = res;
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 2, argv);
    }

}

namespace {
void Initialize(v8::Handle<v8::Object> target) {
    v8::HandleScope scope;
    NodeKytea::Init(target);
}
}

NODE_MODULE(kytea, Initialize)
