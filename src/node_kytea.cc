#define BUILDING_NODE_EXTENSION
#include "node_kytea.h"
#include "macros.h"

using namespace node_kytea;

NodeKyTea::NodeKyTea() {
}

NodeKyTea::~NodeKyTea() {
    delete kytea;
}

void NodeKyTea::Init(Handle<Object> target) {
    HandleScope scope;
    Local < FunctionTemplate > t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("NodeKyTea"));
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "readModel", ReadModel);
    NODE_SET_PROTOTYPE_METHOD(t, "analyze", Analyze);

    Persistent < Function > constructor = Persistent<Function>::New(t->GetFunction());
    target->Set(String::NewSymbol("KyTea"), constructor);
}

Handle<Value> NodeKyTea::New(const Arguments& args) {
    HandleScope scope;

    KyteaConfig* config = new KyteaConfig();

    if (args.Length() == 0) {
        config->setDebug(0);
    } else {
        Local < Object > arg = args[0]->ToObject();
        if (arg->Get(String::New("debug"))->ToBoolean()->IsTrue()) {
            config->setDebug(1);
        } else {
            config->setDebug(0);
        }
    }

    NodeKyTea* obj = new NodeKyTea();
    obj->kytea = new Kytea(config);

    obj->Wrap(args.This());
    return args.This();
}

Handle<Value> NodeKyTea::ReadModel(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string filename = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    NodeKyTea* kt = Unwrap<NodeKyTea> (args.This());
    kt->kytea->getConfig()->setOnTraining(false);
    ModelIoBaton* baton = new ModelIoBaton(kt, cb, filename);
    int status = uv_queue_work(uv_default_loop(), &baton->request, Work_ReadModel, Work_AfterReadModel);
    return args.This();
}

void NodeKyTea::Work_ReadModel(uv_work_t* req) {
    ModelIoBaton* baton = static_cast<ModelIoBaton*> (req->data);
    NodeKyTea* kt = baton->kt;
    try {
        kt->kytea->getConfig()->setDoWS(false);
        kt->kytea->readModel(baton->filename.c_str());
    } catch (std::exception &e) {
        baton->status = ST_FAIL;
        baton->message = e.what();
    }
}

void NodeKyTea::Work_AfterReadModel(uv_work_t* req) {
    HandleScope scope;
    ModelIoBaton* baton = static_cast<ModelIoBaton*> (req->data);
    NodeKyTea* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[1];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local<Value>::New(Null());
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 1, argv);
    }

    delete baton;
}

Handle<Value> NodeKyTea::Analyze(const Arguments& args) {
    HandleScope scope;
    REQ_STR_ARG(0);
    std::string sentence = *String::Utf8Value(args[0]->ToString());
    REQ_FUN_ARG(1, cb);
    NodeKyTea* kt = Unwrap<NodeKyTea> (args.This());
    ExecBaton* baton = new ExecBaton(kt, cb, sentence);
    int status = uv_queue_work(uv_default_loop(), &baton->request, Work_Analyze, Work_AfterAnalyze);
    return args.This();
}

void NodeKyTea::Work_Analyze(uv_work_t* req) {
    ExecBaton* baton = static_cast<ExecBaton*> (req->data);
    NodeKyTea* kt = baton->kt;
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

void NodeKyTea::Work_AfterAnalyze(uv_work_t* req) {
    HandleScope scope;
    ExecBaton* baton = static_cast<ExecBaton*> (req->data);
    NodeKyTea* kt = baton->kt;
    std::string msg = baton->message;

    Local < Value > argv[2];
    if (baton->status != ST_OK) {
        Local < Value > exception = Exception::Error(v8::String::New(msg.c_str()));
        argv[0] = exception;
    } else {
        argv[0] = Local<Value>::New(Null());
        KyteaSentence::Words words = baton->words;
        int word_num = words.size();
        Local <Array> res(Array::New(word_num));
        StringUtil* util = kt->kytea->getStringUtil();
        for (int i = 0; i < word_num; i++) {
            std::string w = util->showString(words[i].surf);
            Local < Object > elm = Object::New();
            elm->Set(String::New("word"), String::New(w.c_str(), w.size()));

            int tags_size = words[i].tags.size();
            Local < Array > tags(Array::New());


            for (int j = 0; j < tags_size; j++) {
                int tag_size = words[i].tags[j].size();
                for (int k = 0; k < tag_size; k++) {
                    Local < Object > tag(Object::New());
                    std::string tag_str = util->showString(words[i].tags[j][k].first);
                    tag->Set(String::New("tag"), String::New(tag_str.c_str(), tag_str.size()));
                    tag->Set(String::New("reliability"), Number::New(words[i].tags[j][k].second));
                    tags->Set(Integer::New(j), tag);
                }
            }


            elm->Set(String::New("tags"), tags);
            res->Set(Integer::New(i), elm);
        }

        argv[1] = res;
    }

    if (!baton->callback.IsEmpty() && baton->callback->IsFunction()) {
        TRY_CATCH_CALL(kt->handle_, baton->callback, 2, argv);
    }

    delete baton;
}

void Initialize(Handle<Object> target) {
    HandleScope scope;
    NodeKyTea::Init(target);
}

NODE_MODULE(kytea, Initialize);
