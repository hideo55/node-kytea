#ifndef NODE_KYTEA_MACROS_H
#define NODE_KYTEA_MACROS_H

#define REQ_ARG_COUNT_AND_TYPE(I, TYPE) \
  if (args.Length() < (I + 1) ) { \
      return ThrowException(Exception::RangeError(String::New("A least " #I " arguments are required"))); \
  } else if (!args[I]->Is##TYPE()) { \
      return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a " #TYPE))); \
  }

#define REQ_FUN_ARG(I, VAR) \
  REQ_ARG_COUNT_AND_TYPE(I, Function) \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_STR_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, String)

#define REQ_OBJ_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, Object)

#define CHK_OPT_BOOL(CONFVAR, CONFNAME, OPTVAR, OPTNAME) \
    if( OPTVAR->Has(String::New(OPTNAME)) ){ \
        Local<Value> _tmpval = OPTVAR->Get(String::New(OPTNAME)); \
        if(_tmpval->IsBoolean()){ \
            bool _tmp = _tmpval->ToBoolean()->Value(); \
            CONFVAR->CONFNAME(_tmpval); \
        }else{ \
            ThrowException(Exception::TypeError(String::New("Option " #OPTNAME " must be a Boolean"))); \
        } \
    }

#define CHK_OPT_STR(CONFVAR, CONFNAME, OPTVAR, OPTNAME) \
    if( OPTVAR->Has(String::New(OPTNAME)) ){ \
        Local<Value> _tmpval = OPTVAR->Get(String::New(OPTNAME)); \
        if(_tmpval->IsString()){ \
            std::string _tmp = *v8::String::Utf8Value(_tmpval->ToString()); \
            CONFVAR->CONFNAME(_tmp.c_str()); \
        }else{ \
            ThrowException(Exception::TypeError(String::New("Option " #OPTNAME " must be a String"))); \
        } \
    }

#define CHK_OPT_INT(CONFVAR, CONFNAME, OPTVAR, OPTNAME) \
    if( OPTVAR->Has(String::New(OPTNAME)) ){ \
        Local<Value> _tmpval = OPTVAR->Get(String::New(OPTNAME)); \
        if(_tmpval->IsUint32()){ \
            unsigned int _tmp = _tmpval->ToUint32()->Value(); \
            CONFVAR->CONFNAME(_tmp); \
        }else{ \
            ThrowException(Exception::TypeError(String::New("Option " #OPTNAME " must be a Integer"))); \
        } \
    }


#define TRY_CATCH_CALL(context, callback, argc, argv)                          \
    TryCatch try_catch;                                                        \
    (callback)->Call((context), (argc), (argv));                               \
    if (try_catch.HasCaught()) {                                               \
        FatalException(try_catch);                                             \
    }

#endif /* NODE_KYTEA_MACROS_H */
