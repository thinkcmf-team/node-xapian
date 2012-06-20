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
  bool aAsync = args.Length() == 1 && args[0]->IsFunction();
  if (args.Length() != +aAsync)
    return ThrowException(Exception::TypeError(String::New("arguments are ([function])")));

  Generic_data* aData = new Generic_data(Generic_data::eGetData); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Document::Generic_process(void* pData, void* pThat) {
  Generic_data* data = (Generic_data*) pData;
  Document* that = (Document *) pThat;

  switch (data->action) {
  case Generic_data::eGetData: data->str = that->mDoc->get_data(); break;
  }
}

Handle<Value> Document::Generic_convert(void* pData) {
  Generic_data* data = (Generic_data*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case Generic_data::eGetData:  aResult = String::New(data->str.c_str()); break;
  }

  delete data;
  return aResult;
}
