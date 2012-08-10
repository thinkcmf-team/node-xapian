#include "node-xapian.h"

Persistent<FunctionTemplate> RSet::constructor_template;

void RSet::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("RSet"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);

  target->Set(String::NewSymbol("RSet"), constructor_template->GetFunction());
}

static int kNewRSet[] = { eEnd };
Handle<Value> RSet::New(const Arguments& args) {
  HandleScope scope;
  int aOpt[0];
  if (!checkArguments(kNewRSet, args, aOpt))
    return throwSignatureErr(kNewRSet);

  RSet* that;
  try {
    that = new RSet();
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  that->Wrap(args.This());
  return args.This();
}

enum { 
  eGetDescription
};

void RSet::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  RSet* that = (RSet *) pThat;

  switch (data->action) {
  case eGetDescription:         data->retVal.setString(that->mRSet.get_description()); break;
  default: assert(0);
  }
}

Handle<Value> RSet::Generic_convert(void* pData) {
  GenericData* data = (GenericData*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case eGetDescription: 
    aResult = String::New(data->retVal.string->c_str()); break;
  }

  delete data;
  return aResult;
}

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> RSet::GetDescription(const Arguments& args) { return generic_start<RSet>(eGetDescription, args, kGetDescription); }

