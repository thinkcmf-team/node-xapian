#include "node-xapian.h"

Persistent<FunctionTemplate> Document::constructor_template;

void Document::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Document"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_data", GetData);

  target->Set(String::NewSymbol("Document"), constructor_template->GetFunction());
}

Handle<Value> Document::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() && !args[0]->IsExternal())
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));

  Document* that = new Document(args.Length() ? (Xapian::Document*) External::Unwrap(args[0]) : new Xapian::Document);
  that->Wrap(args.This());

  return args.This();
}

Handle<Value> Document::GetData(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 || !args[0]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (function)")));
  GetData_data* aData;
  try {
    aData = new GetData_data(args.This(), Local<Function>::Cast(args[0]));
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(GetData_pool, EIO_PRI_DEFAULT, GetData_done, aData);

  return Undefined();
}

int Document::GetData_pool(eio_req* req) {
  GetData_data* aData = (GetData_data*) req->data;

  try {
  aData->data = aData->object->mDoc->get_data();
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  aData->poolDone();
  return 0;
}

int Document::GetData_done(eio_req* req) {
  HandleScope scope;

  GetData_data* aData = (GetData_data*) req->data;

  Handle<Value> argv[2];
  if (aData->error) {
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));
  } else {
    argv[0] = Null();
    argv[1] = String::New(aData->data.c_str());
  }

  tryCallCatch(aData->callback, aData->object->handle_, aData->error ? 1 : 2, argv);

  delete aData;

  return 0;
}
