#include "node_kytea_async.h"
#include <cstring>
#include <stdexcept>

using namespace v8;

namespace node_kytea {

// KyteaWorker
KyteaWorker::KyteaWorker(Nan::Callback* callback, NodeKytea* kytea) :
        Nan::AsyncWorker(callback), kytea_(kytea) {
}

// OpenWorker
OpenWorker::OpenWorker(Nan::Callback* callback, NodeKytea* kytea, std::string& filename) :
        KyteaWorker(callback, kytea), filename_(filename) {
}

void OpenWorker::Execute() {
    try {
        kytea_->openModel(filename_);
    } catch (std::runtime_error& e) {
        SetErrorMessage(e.what());
    }
}

// WSWorker
WSWorker::WSWorker(Nan::Callback* callback, NodeKytea* kytea, std::string& text) :
        KyteaWorker(callback, kytea), text_(text) {
}

void WSWorker::Execute() {
    try {
        kytea_->calculateWS(text_, words_);
    } catch (std::runtime_error& e) {
        SetErrorMessage(e.what());
    }
}

void WSWorker::HandleOKCallback() {
    Nan::HandleScope scope;
    Local<Array> result = Nan::New<Array>(words_.size());
    kytea_->MakeWsResult(words_, result);
    std::string v = *String::Utf8Value(result->Get(Nan::New<Integer>(0))->ToString());
    Local<Value> argv[] = {Nan::Null(), result};
    callback->Call(2, argv);
}

// TagWorker
TagWorker::TagWorker(Nan::Callback* callback, NodeKytea* kytea, std::string& text, bool all) :
        KyteaWorker(callback, kytea), text_(text), all_(all) {

}

void TagWorker::Execute() {
    try {
        kytea_->calculateTags(text_, words_);
    } catch (std::runtime_error& e) {
        SetErrorMessage(e.what());
    }
}

void TagWorker::HandleOKCallback() {
    Nan::HandleScope scope;
    Local<Array> result = Nan::New<Array>(words_.size());
    kytea_->MakeTagsResult(words_, result, all_);
    Local<Value> argv[] = {Nan::Null(), result};
    callback->Call(2, argv);
}

}
