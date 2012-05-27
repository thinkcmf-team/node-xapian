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
  AsyncOp(Handle<Object> ob, Handle<Function> cb) //temporary untill all the methods using AsyncOp are refactored
    : AsyncOpBase(cb), object(ObjectWrap::Unwrap<T>(ob)) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  AsyncOp(Handle<Object> ob, Handle<Function> cb, void* dt)
    : AsyncOpBase(cb), object(ObjectWrap::Unwrap<T>(ob)), data(dt) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  virtual ~AsyncOp() { object->Unref(); }
  void poolDone() { object->mBusy = false; }
  T* object;
  void* data;
};

#define DECLARE_POOLS(func,classn) \
static int func##_pool(eio_req *req) {\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  aAsOp->error=func##_process((func##_data*)aAsOp->data, aAsOp->object);\
  aAsOp->poolDone();\
  return 0;\
}\
static int func##_done(eio_req *req) {\
  HandleScope scope;\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  func##_data *aData = (func##_data*)aAsOp->data;\
  Handle<Value> aArgv[2];\
  if (aAsOp->error) {\
    aArgv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));\
  } else {\
    aArgv[0] = Null();\
    aArgv[1] = func##_convert(aData);\
  }\
  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, aArgv);\
  delete aData;\
  delete aAsOp;\
  return 0;\
}\
static Handle<Value> func##_do_async(const Arguments& args,func##_data *&data) {\
  AsyncOp<classn> *aAsOp=NULL;\
  try {\
    if (ObjectWrap::Unwrap<classn>(args.This())->mBusy) {\
      if (data) delete data;\
      return ThrowException(Exception::Error(kBusyMsg));\
    }\
    aAsOp = new AsyncOp<classn>(args.This(), Local<Function>::Cast(args[2]),data);\
  } catch (Local<Value> ex) {\
    if (data) delete data;\
    if (aAsOp) delete aAsOp;\
    return ThrowException(ex);\
  }\
  sendToThreadPool((void*)func##_pool, (void*)func##_done, aAsOp);\
  return Undefined();\
}\
static Handle<Value> func##_do_sync(const Arguments& args,func##_data *&data) {\
  Xapian::Error* aError=NULL;\
  try {\
    classn *that=ObjectWrap::Unwrap<classn>(args.This());\
    if (that->mBusy) {\
      if (data) delete data;\
      return ThrowException(Exception::Error(kBusyMsg));\
    }\
    aError = func##_process(data,that);\
  } catch (Local<Value> ex) {\
    if (data) delete data;\
    return ThrowException(ex);\
  }\
  if (aError!=NULL) {\
    std::string aErrorStr=aError->get_msg();\
    if (data) delete data;\
    delete aError;\
    return ThrowException(Exception::Error(String::New(aErrorStr.c_str())));\
  }\
  Handle<Value> aResult=func##_convert(data);\
  if (data) delete data;\
  return aResult;\
}


#endif
