#ifndef _XAPIAN_OP_H_
#define _XAPIAN_OP_H_

#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

void tryCallCatch(Handle<Function> fn, Handle<Object> context, int argc, Handle<Value>* argv);

template <class T>
T* GetInstance(Handle<Value> val) {
  if (val->IsObject() && T::constructor_template->HasInstance(val->ToObject()))
    return ObjectWrap::Unwrap<T>(val->ToObject());
  return NULL;
}

void sendToThreadPool(void* execute, void* done, void* data);

extern Persistent<String> kBusyMsg;

struct AsyncOpBase {
  AsyncOpBase(Handle<Function> cb)
    : callback(), error(NULL) {
    callback = Persistent<Function>::New(cb);
    ev_ref(EV_DEFAULT_UC);
  }
  virtual ~AsyncOpBase() {
    if (error) delete error;
    ev_unref(EV_DEFAULT_UC);
    callback.Dispose();
  }
  Persistent<Function> callback;
  Xapian::Error* error;
};

typedef Xapian::Error* (*FuncProcess) (void *data, void *that);
typedef Handle<Value> (*FuncConvert) (void *data);

template <class T>
struct AsyncOp : public AsyncOpBase {
  AsyncOp(Handle<Object> ob, Handle<Function> cb) //temporary untill all the methods using AsyncOp are refactored
    : AsyncOpBase(cb), object(ObjectWrap::Unwrap<T>(ob)) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  AsyncOp(T* ob, Handle<Function> cb, void* dt, FuncProcess pr, FuncConvert cv)
    : AsyncOpBase(cb), object(ob), data(dt), process(pr), convert(cv) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  virtual ~AsyncOp() { object->Unref(); }
  void poolDone() { object->mBusy = false; }
  T* object;
  void* data;
  FuncProcess process;
  FuncConvert convert;
};

#define DECLARE_POOLS(classn) \
static int function_pool(eio_req *req) {\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  aAsOp->error=aAsOp->process(aAsOp->data, aAsOp->object);\
  aAsOp->poolDone();\
  return 0;\
}\
static int function_done(eio_req *req) {\
  HandleScope scope;\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  Handle<Value> aArgv[2];\
  if (aAsOp->error) {\
    aArgv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));\
  } else {\
    aArgv[0] = Null();\
    aArgv[1] = aAsOp->convert(aAsOp->data);\
  }\
  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, aArgv);\
  delete aAsOp;\
  return 0;\
}\
static Handle<Value> do_async(const Arguments& args,void *data, FuncProcess process, FuncConvert convert) {\
  AsyncOp<classn> *aAsOp=NULL;\
  classn *that=ObjectWrap::Unwrap<classn>(args.This());\
  if (that->mBusy) {\
    throw Exception::Error(kBusyMsg);\
  }\
  aAsOp = new AsyncOp<classn>(that, Local<Function>::Cast(args[2]),data,process,convert);\
  sendToThreadPool((void*)function_pool, (void*)function_done, aAsOp);\
  return Undefined();\
}\
static Handle<Value> do_sync(const Arguments& args,void *data, FuncProcess process, FuncConvert convert) {\
  Xapian::Error* aError=NULL;\
  classn *that=ObjectWrap::Unwrap<classn>(args.This());\
  if (that->mBusy) {\
    throw Exception::Error(kBusyMsg);\
  }\
  aError = process(data,that);\
  if (aError!=NULL) {\
    std::string aErrorStr=aError->get_msg();\
    delete aError;\
    throw Exception::Error(String::New(aErrorStr.c_str()));\
  }\
  return convert(data);\
}\
static Handle<Value> do_all(bool async, const Arguments& args, void *data, FuncProcess process, FuncConvert convert) {\
  return async ? \
    do_async(args, data, process, convert) : \
    do_sync(args, data, process, convert); \
}

#endif
