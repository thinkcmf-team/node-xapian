#include "node-xapian.h"

Persistent<FunctionTemplate> RSet::constructor_template;

void RSet::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("RSet"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "size", Size);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "empty", Empty);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_document", AddDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_document", RemoveDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "contains", Contains);

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
  eGetDescription, eSize, eEmpty, eAddDocument, eRemoveDocument, eContains
};

void RSet::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  RSet* that = (RSet *) pThat;

  switch (data->action) {
  case eGetDescription: data->retVal.setString(that->mRSet.get_description());            break;
  case eSize:           data->retVal.uint32 = that->mRSet.size();                         break;
  case eEmpty:          data->retVal.boolean = that->mRSet.empty();                       break;
  case eAddDocument:    that->mRSet.add_document(data->val[0].uint32);                    break;
  case eRemoveDocument: that->mRSet.remove_document(data->val[0].uint32);                 break;
  case eContains:       data->retVal.boolean = that->mRSet.contains(data->val[0].uint32); break;
  default: assert(0);
  }
}

Handle<Value> RSet::Generic_convert(void* pData) {
  GenericData* data = (GenericData*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case eGetDescription: 
    aResult = String::New(data->retVal.string->c_str()); break;
  case eSize:
    aResult = Uint32::New(data->retVal.uint32);          break;
  case eContains:
  case eEmpty:
    aResult = Boolean::New(data->retVal.boolean);        break;
  case eAddDocument:
  case eRemoveDocument:
    aResult = Undefined();                               break;
  }

  delete data;
  return aResult;
}

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> RSet::GetDescription(const Arguments& args) { return generic_start<RSet>(eGetDescription, args, kGetDescription); }

static int kSize[] = { -eFunction, eEnd };
Handle<Value> RSet::Size(const Arguments& args) { return generic_start<RSet>(eSize, args, kSize); }

static int kEmpty[] = { -eFunction, eEnd };
Handle<Value> RSet::Empty(const Arguments& args) { return generic_start<RSet>(eEmpty, args, kEmpty); }

static int kAddDocument[] = { eUint32, -eFunction, eEnd };
Handle<Value> RSet::AddDocument(const Arguments& args) { return generic_start<RSet>(eAddDocument, args, kAddDocument); }

static int kRemoveDocument[] = { eUint32, -eFunction, eEnd };
Handle<Value> RSet::RemoveDocument(const Arguments& args) { return generic_start<RSet>(eRemoveDocument, args, kRemoveDocument); }

static int kContains[] = { eUint32, -eFunction, eEnd };
Handle<Value> RSet::Contains(const Arguments& args) { return generic_start<RSet>(eContains, args, kContains); }
