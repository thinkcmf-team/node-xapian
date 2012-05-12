#ifndef _XAPIAN_OP_
#define _XAPIAN_OP_

#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

static void tryCallCatch(Handle<Function> fn, Handle<Object> context, int argc, Handle<Value>* argv) {
  TryCatch try_catch;

  fn->Call(context, argc, argv);

  if (try_catch.HasCaught())
    FatalException(try_catch);
}

template <class T>
static T* GetInstance(Handle<Value> val) {
  if (val->IsObject() && T::constructor_template->HasInstance(val->ToObject()))
    return ObjectWrap::Unwrap<T>(val->ToObject());
  return NULL;
}

static void sendToThreadPool(void* execute, void* done, void* data){
  eio_custom((eio_cb) execute, EIO_PRI_DEFAULT, (eio_cb) done, data);
}

static Persistent<String> kBusyMsg;

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

template <class T>
struct AsyncOp : public AsyncOpBase {
  AsyncOp(Handle<Object> ob, Handle<Function> cb)
    : AsyncOpBase(cb), object(ObjectWrap::Unwrap<T>(ob)) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  virtual ~AsyncOp() { object->Unref(); }
  void poolDone() { object->mBusy = false; }
  T* object;
};

struct OpInfo {
  OpInfo(void* data): data(data), op(NULL){}
  OpInfo(void* data, void *asop): data(data), op(asop) {}
  void* data;
  void* op;
};


#define DECLARE_POOLS(func,classn) \
static int func##_pool(eio_req *req) {\
  OpInfo* aInfo = (OpInfo*) req->data;\
  func##_data *aData = (func##_data*)aInfo->data;\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)aInfo->op;\
  aAsOp->error=func##_process(aData, aAsOp->object);\
  aAsOp->poolDone();\
  return 0;\
}\
static int func##_done(eio_req *req) {\
  HandleScope scope;\
  OpInfo* aInfo = (OpInfo*) req->data;\
  func##_data *aData = (func##_data*)aInfo->data;\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)aInfo->op;\
  Handle<Value> argv[2];\
  if (aAsOp->error) {\
    argv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));\
  } else {\
    argv[0] = Null();\
    argv[1] = func##_convert(aData);\
  }\
  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, argv);\
  delete aData;\
  delete aAsOp;\
  delete aInfo;\
  return 0;\
}\
static Handle<Value> func##_do_async(const Arguments& args,func##_data *&aData)\
{\
  HandleScope scope;\
  OpInfo *aInfo=NULL;\
  AsyncOp<classn> *aAsOp=NULL;\
  try {\
    if (ObjectWrap::Unwrap<classn>(args.This())->mBusy)\
    {\
      if (aData) delete aData;\
      return ThrowException(Exception::Error(kBusyMsg));\
    }\
    aAsOp = new AsyncOp<classn>(args.This(), Local<Function>::Cast(args[2]));\
    aInfo=new OpInfo(aData,aAsOp);\
  } catch (Local<Value> ex) {\
    if (aData) delete aData;\
    if (aAsOp) delete aAsOp;\
    if (aInfo) delete aInfo;\
    return ThrowException(ex);\
  }\
  sendToThreadPool((void*)func##_pool, (void*)func##_done, aInfo);\
  return Undefined();\
}\
static Handle<Value> func##_do_sync(const Arguments& args,func##_data *&aData)\
{\
  HandleScope scope;\
  Xapian::Error* error=NULL;\
  try {\
    classn *pThis=ObjectWrap::Unwrap<classn>(args.This());\
    if (pThis->mBusy)\
    {\
      if (aData) delete aData;\
      return ThrowException(Exception::Error(kBusyMsg));\
    }\
    error = func##_process(aData,pThis);\
  } catch (Local<Value> ex) {\
    if (aData) delete aData;\
    return ThrowException(ex);\
  }\
  if (error!=NULL)\
  {\
    std::string errorStr=error->get_msg();\
    if (aData) delete aData;\
    delete error;\
    return ThrowException(Exception::Error(String::New(errorStr.c_str())));\
  }\
  Handle<Value> result=func##_convert(aData);\
  if (aData) delete aData;\
  return scope.Close(result);\
}


#endif
