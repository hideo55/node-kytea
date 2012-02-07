#define BUILDING_NODE_EXTENSION
#include "analyzer.h"
#include "macros.h"

using namespace node_kytea;
using namespace v8;
using namespace node;
using namespace kytea;

void Analyzer::Init(Handle<Object> target) {
    HandleScope scope;
    Local < FunctionTemplate > t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("Analyzer"));
    t->InstanceTemplate()->SetInternalFieldCount(3);

    NODE_SET_PROTOTYPE_METHOD(t, "getWS", getWS);
    NODE_SET_PROTOTYPE_METHOD(t, "getTags", getTags);
    NODE_SET_PROTOTYPE_METHOD(t, "getAllTags", getAllTags);

    Persistent < Function > constructor = Persistent<Function>::New(t->GetFunction());
    target->Set(String::NewSymbol("Analyzer"), constructor);
}

Analyzer::Analyzer() :
    isModelLoaded(false) {
}

Analyzer::~Analyzer() {
    delete kytea;
}

Handle<Value> Analyzer::New(const Arguments& args) {
    HandleScope scope;

    REQ_STR_ARG(0);
    std::string filename = *String::Utf8Value(args[0]->ToString());

    Local < Function > cb;

    Analyzer* obj = new Analyzer();
    obj->kytea = new Kytea();

    kytea::KyteaConfig* config = obj->kytea->getConfig();

    if (args.Length() == 2) {
        REQ_ARG_COUNT_AND_TYPE(1, Function);
        cb = Local<Function>::Cast(args[1]);
    } else if (args.Length() == 3) {
        REQ_OBJ_ARG(1);
        Local < Object > opt = Local<Object>::Cast(args[1]);
        ParseConfig(opt, config);
        REQ_ARG_COUNT_AND_TYPE(2, Function);
        cb = Local<Function>::Cast(args[2]);
    } else {
        ThrowException(Exception::RangeError(String::New("Invalid Argument")));
    }
    config->setOnTraining(false);
    config->setDoWS(true);
    config->setDoTags(true);
    for (int i = 0; i < config->getNumTags(); i++)
        config->setDoTag(i, true);

    obj->Wrap(args.This());

    ReadBaton* baton = new ReadBaton(obj, cb, filename);
    int status = uv_queue_work(uv_default_loop(), &baton->request, Work_ReadModel, Work_AfterReadModel);

    return args.This();
}

void Analyzer::ParseConfig(Handle<Object> opt, KyteaConfig *config) {
    //debug flag
    if (opt->Get(String::New("debug"))->ToBoolean()->IsTrue()) {
        config->setDebug(1);
    }
    CHK_OPT_INT(config, setTagMax, opt, "tagmax");
    CHK_OPT_STR(config, setDefaultTag, opt, "deftag");
}

void Analyzer::Work_ReadModel(uv_work_t* req) {
    ReadBaton* baton = static_cast<ReadBaton*> (req->data);
    Analyzer* kt = baton->kt;
    try {
        kt->kytea->readModel(baton->filename.c_str());
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

void Analyzer::Work_AfterReadModel(uv_work_t* req) {
    HandleScope scope;
    ReadBaton* baton = static_cast<ReadBaton*> (req->data);
    Analyzer* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[1];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        kt->isModelLoaded = true;
        argv[0] = Local<Value>::New(Null());
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 1, argv);
    }

    delete baton;
}

Handle<Value> Analyzer::getWS(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    Analyzer* kt = Unwrap<Analyzer> (args.This());
    if (kt->isModelLoaded) {
        WsBaton* baton = new WsBaton(kt, cb, sentence);
        int status = uv_queue_work(uv_default_loop(), &baton->request, Work_WS, Work_AfterWS);
    } else {
        ThrowException(Exception::Error(v8::String::New("Model in not loaded")));
    }
    return args.This();
}

void Analyzer::Work_WS(uv_work_t* req) {
    WsBaton* baton = static_cast<WsBaton*> (req->data);
    Analyzer* kt = baton->kt;
    try {
        StringUtil* util = kt->kytea->getStringUtil();
        KyteaConfig* config = kt->kytea->getConfig();
        KyteaSentence sentence(util->mapString(baton->sentence));
        kt->kytea->calculateWS(sentence);
        baton->words = sentence.words;
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

void Analyzer::Work_AfterWS(uv_work_t* req) {
    HandleScope scope;
    WsBaton* baton = static_cast<WsBaton*> (req->data);
    Analyzer* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[2];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local<Value>::New(Null());
        KyteaSentence::Words words = baton->words;
        int word_num = words.size();
        Local < Array > res(Array::New(word_num));
        StringUtil* util = kt->kytea->getStringUtil();
        for (int i = 0; i < word_num; i++) {
            kytea::KyteaWord& w = words[i];
            std::string surf = util->showString(w.surf);
            res->Set(Integer::New(i), String::New(surf.c_str(), surf.size()));
        }
        argv[1] = res;
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 2, argv);
    }

    delete baton;
}

Handle<Value> Analyzer::getTags(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    Analyzer* kt = Unwrap<Analyzer> (args.This());
    if (kt->isModelLoaded) {
        TagsBaton* baton = new TagsBaton(kt, cb, sentence);
        int status = uv_queue_work(uv_default_loop(), &baton->request, Work_Tags, Work_AfterTags);
    } else {
        ThrowException(Exception::Error(v8::String::New("Model in not loaded")));
    }
    return args.This();
}

Handle<Value> Analyzer::getAllTags(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    Analyzer* kt = Unwrap<Analyzer> (args.This());
    if (kt->isModelLoaded) {
        TagsBaton* baton = new TagsBaton(kt, cb, sentence, true);
        int status = uv_queue_work(uv_default_loop(), &baton->request, Work_Tags, Work_AfterTags);
    } else {
        ThrowException(Exception::Error(v8::String::New("Model in not loaded")));
    }
    return args.This();
}

void Analyzer::Work_Tags(uv_work_t* req) {
    TagsBaton* baton = static_cast<TagsBaton*> (req->data);
    Analyzer* kt = baton->kt;
    try {
        StringUtil* util = kt->kytea->getStringUtil();
        KyteaConfig* config = kt->kytea->getConfig();
        KyteaSentence sentence(util->mapString(baton->sentence));
        kt->kytea->calculateWS(sentence);

        for (int i = 0; i < config->getNumTags(); i++)
            kt->kytea->calculateTags(sentence, i);

        baton->words = sentence.words;
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

void Analyzer::Work_AfterTags(uv_work_t* req) {
    HandleScope scope;
    TagsBaton* baton = static_cast<TagsBaton*> (req->data);
    Analyzer* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[2];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local<Value>::New(Null());
        KyteaSentence::Words words = baton->words;
        int word_num = words.size();
        Local < Array > res(Array::New(word_num));
        StringUtil* util = kt->kytea->getStringUtil();

        std::string keyname[2] = {"pos","pron"};

        for (int i = 0; i < word_num; i++) {
            kytea::KyteaWord& w = words[i];
            std::string surf = util->showString(w.surf);
            Local < Object > elm = Object::New();
            elm->Set(String::New("surf"), String::New(surf.c_str(), surf.size()));

            int tags_size = w.getNumTags();

            for (int j = 0; j < tags_size; j++) {
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
                elm->Set(String::New(keyname[j].c_str()), tag_set);
            }
            res->Set(Integer::New(i), elm);
        }

        argv[1] = res;
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 2, argv);
    }

    delete baton;
}
