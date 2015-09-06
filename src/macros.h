#ifndef NODE_KYTEA_MACROS_H
#define NODE_KYTEA_MACROS_H

#include "nan.h"

#define REQ_ARG_COUNT_AND_TYPE(I, TYPE) \
  if (info.Length() < (I + 1) ) { \
      std::stringstream __ss; \
      __ss << "A least " << I + 1 << " arguments are required"; \
      return Nan::ThrowRangeError(__ss.str().c_str()); \
  } else if (!info[I]->Is##TYPE()) { \
      std::stringstream __ss; \
      __ss << "Argument " << I + 1 << " must be a " #TYPE; \
      return Nan::ThrowTypeError(__ss.str().c_str()); \
  }

#define REQ_FUN_ARG(I, VAR) \
  REQ_ARG_COUNT_AND_TYPE(I, Function) \
  Local<Function> VAR = info[I].As<Function>();

#define REQ_STR_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, String)

#define REQ_OBJ_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, Object)

#define CHK_OPT_BOOL(CONFVAR, CONFNAME, OPTVAR, OPTNAME, REVERSE) \
    if( OPTVAR->Has(Nan::New<v8::String>(OPTNAME).ToLocalChecked()) ){ \
        Local<Value> _tmpval = OPTVAR->Get(Nan::New<v8::String>(OPTNAME).ToLocalChecked()); \
        if(_tmpval->IsBoolean()){ \
            bool _tmp = _tmpval->ToBoolean()->Value(); \
            if(REVERSE) _tmp = !(_tmp); \
            CONFVAR->CONFNAME(_tmp); \
        }else{ \
            Nan::ThrowTypeError("Option " #OPTNAME " must be a Boolean"); \
        } \
    }

#define CHK_OPT_STR(CONFVAR, CONFNAME, OPTVAR, OPTNAME) \
    if( OPTVAR->Has(Nan::New<v8::String>(OPTNAME).ToLocalChecked()) ){ \
        Local<Value> _tmpval = OPTVAR->Get(Nan::New<v8::String>(OPTNAME).ToLocalChecked()); \
        if(_tmpval->IsString()){ \
            std::string _tmp = *v8::String::Utf8Value(_tmpval->ToString()); \
            CONFVAR->CONFNAME(_tmp.c_str()); \
        }else{ \
            Nan::ThrowTypeError("Option " #OPTNAME " must be a String"); \
        } \
    }

#define CHK_OPT_INT(CONFVAR, CONFNAME, OPTVAR, OPTNAME) \
    if( OPTVAR->Has(Nan::New<v8::String>(OPTNAME).ToLocalChecked()) ){ \
        Local<Value> _tmpval = OPTVAR->Get(Nan::New<v8::String>(OPTNAME).ToLocalChecked()); \
        if(_tmpval->IsUint32()){ \
            unsigned int _tmp = _tmpval->ToUint32()->Value(); \
            CONFVAR->CONFNAME(_tmp); \
        }else{ \
            Nan::ThrowTypeError("Option " #OPTNAME " must be a Integer"); \
        } \
    }


#define TRY_CATCH_CALL(context, callback, argc, argv)                          \
    TryCatch try_catch;                                                        \
    (callback)->Call((context), (argc), (argv));                               \
    if (try_catch.HasCaught()) {                                               \
        FatalException(try_catch);                                             \
    }

#endif /* NODE_KYTEA_MACROS_H */
