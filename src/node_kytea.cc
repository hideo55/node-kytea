#define BUILDING_NODE_EXTENSION
#include "analyzer.h"
#include "macros.h"

using namespace node_kytea;

namespace {
void Initialize(v8::Handle<v8::Object> target) {
    v8::HandleScope scope;
    Analyzer::Init(target);
}
}

NODE_MODULE(kytea, Initialize);
