#include "node-xapian.h"

Persistent<FunctionTemplate> Document::constructor_template;

void Document::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Document"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_value", GetValue);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_value", AddValue);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_value", RemoveValue);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "clear_values", ClearValues);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_data", GetData);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_data", SetData);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_posting", AddPosting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_term", AddTerm);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_boolean_term", AddBooleanTerm);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_posting", RemovePosting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_term", RemoveTerm);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "clear_terms", ClearTerms);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "termlist_count", TermlistCount);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "values_count", ValuesCount);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_docid", GetDocid);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "serialise", Serialise);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "unserialise", Unserialise);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "termlist", Termlist);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "values", Values);

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

enum { 
  eGetValue, eAddValue, eRemoveValue, eClearValues, eGetData, eSetData, eAddPosting, eAddTerm, eAddBooleanTerm,
  eRemovePosting, eRemoveTerm, eClearTerms, eTermlistCount, eValuesCount, eGetDocid, eSerialise, eGetDescription
};

static int kGetValue[] = { eUint32, -eFunction, eEnd };
Handle<Value> Document::GetValue(const Arguments& args) { return generic_start<Document>(eGetValue, args, kGetValue); }

static int kAddValue[] = { eUint32, eString, -eFunction, eEnd };
Handle<Value> Document::AddValue(const Arguments& args) { return generic_start<Document>(eAddValue, args, kAddValue); }

static int kRemoveValue[] = { eUint32, -eFunction, eEnd };
Handle<Value> Document::RemoveValue(const Arguments& args) { return generic_start<Document>(eRemoveValue, args, kRemoveValue); }

static int kClearValues[] = { -eFunction, eEnd };
Handle<Value> Document::ClearValues(const Arguments& args) { return generic_start<Document>(eClearValues, args, kClearValues); }

static int kGetData[] = { -eFunction, eEnd };
Handle<Value> Document::GetData(const Arguments& args) { return generic_start<Document>(eGetData, args, kGetData); }

static int kSetData[] = { eString, -eFunction, eEnd };
Handle<Value> Document::SetData(const Arguments& args) { return generic_start<Document>(eSetData, args, kSetData); }

static int kAddPosting[] = { eString, eUint32, -eUint32, -eFunction, eEnd };
static GenericData::Item kAddPostingDefault[1] = { (uint32_t)1 };
Handle<Value> Document::AddPosting(const Arguments& args) { return generic_start<Document>(eAddPosting, args, kAddPosting, kAddPostingDefault); }

static int kAddTerm[] = { eString, -eUint32, -eFunction, eEnd };
static GenericData::Item kAddTermDefault[1] = { (uint32_t)1 };
Handle<Value> Document::AddTerm(const Arguments& args) { return generic_start<Document>(eAddTerm, args, kAddTerm, kAddTermDefault); }

static int kAddBooleanTerm[] = { eString, -eFunction, eEnd };
Handle<Value> Document::AddBooleanTerm(const Arguments& args) { return generic_start<Document>(eAddBooleanTerm, args, kAddBooleanTerm); }

static int kRemovePosting[] = { eString, eUint32, -eUint32, -eFunction, eEnd };
static GenericData::Item kRemovePostingDefault[1] = { (uint32_t)1 };
Handle<Value> Document::RemovePosting(const Arguments& args) { return generic_start<Document>(eRemovePosting, args, kRemovePosting, kRemovePostingDefault); }

static int kRemoveTerm[] = { eString, -eFunction, eEnd };
Handle<Value> Document::RemoveTerm(const Arguments& args) { return generic_start<Document>(eRemoveTerm, args, kRemoveTerm); }

static int kClearTerms[] = { -eFunction, eEnd };
Handle<Value> Document::ClearTerms(const Arguments& args) { return generic_start<Document>(eClearTerms, args, kClearTerms); }

static int kTermlistCount[] = { -eFunction, eEnd };
Handle<Value> Document::TermlistCount(const Arguments& args) { return generic_start<Document>(eTermlistCount, args, kTermlistCount); }

static int kValuesCount[] = { -eFunction, eEnd };
Handle<Value> Document::ValuesCount(const Arguments& args) { return generic_start<Document>(eValuesCount, args, kValuesCount); }

static int kGetDocid[] = { -eFunction, eEnd };
Handle<Value> Document::GetDocid(const Arguments& args) { return generic_start<Document>(eGetDocid, args, kGetDocid); }

static int kSerialise[] = { -eFunction, eEnd };
Handle<Value> Document::Serialise(const Arguments& args) { return generic_start<Document>(eSerialise, args, kSerialise); }

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> Document::GetDescription(const Arguments& args) { return generic_start<Document>(eGetDescription, args, kGetDescription); }

void Document::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  Document* that = (Document *) pThat;

  switch (data->action) {
  case eGetValue:        data->retVal.setString(that->mDoc->get_value(data->val[0].uint32));                         break;
  case eAddValue:        that->mDoc->add_value(data->val[0].uint32, *data->val[1].string);                           break;
  case eRemoveValue:     that->mDoc->remove_value(data->val[0].uint32);                                              break;
  case eClearValues:     that->mDoc->clear_values();                                                                 break;
  case eGetData:         data->retVal.setString(that->mDoc->get_data());                                             break;
  case eSetData:         that->mDoc->set_data(*data->val[0].string);                                                 break;
  case eAddPosting:      that->mDoc->add_posting(*data->val[0].string, data->val[1].uint32, data->val[2].uint32);    break;
  case eAddTerm:         that->mDoc->add_term(*data->val[0].string, data->val[1].uint32);                            break;
  case eAddBooleanTerm:  that->mDoc->add_boolean_term(*data->val[0].string);                                         break;
  case eRemovePosting:   that->mDoc->remove_posting(*data->val[0].string, data->val[1].uint32, data->val[2].uint32); break;
  case eRemoveTerm:      that->mDoc->remove_term(*data->val[0].string);                                              break;
  case eClearTerms:      that->mDoc->clear_terms();                                                                  break;
  case eTermlistCount:   data->retVal.uint32 = that->mDoc->termlist_count();                                         break;
  case eValuesCount:     data->retVal.uint32 = that->mDoc->values_count();                                           break;
  case eGetDocid:        data->retVal.uint32 = that->mDoc->get_docid();                                              break;
  case eSerialise:       data->retVal.setString(that->mDoc->serialise());                                            break;
  case eGetDescription:  data->retVal.setString(that->mDoc->get_description());                                      break;
  default: assert(0);
  }
}

Handle<Value> Document::Generic_convert(void* pData) {
  GenericData* data = (GenericData*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case eTermlistCount:
  case eValuesCount:
  case eGetDocid:       
    aResult = Integer::NewFromUnsigned(data->retVal.uint32); break;
  case eGetValue:
  case eGetData:  
  case eSerialise:
  case eGetDescription: 
    aResult = String::New(data->retVal.string->c_str());       break;
  case eAddValue:
  case eRemoveValue:
  case eClearValues:
  case eSetData:
  case eAddPosting:
  case eAddTerm:
  case eAddBooleanTerm:
  case eRemovePosting:
  case eRemoveTerm:
  case eClearTerms:     
    aResult = Undefined();                          break;
  }

  delete data;
  return aResult;
}


static int kUnserialise[] = { eString, -eFunction, eEnd };
Handle<Value> Document::Unserialise(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kUnserialise, args, aOpt))
    return throwSignatureErr(kUnserialise);

  Unserialise_data* aData = new Unserialise_data(*String::Utf8Value(args[0])); //deleted by Unserialise_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] >= 0, args, (void*)aData, Unserialise_process, Unserialise_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Document::Unserialise_process(void* pData, void* pThat) {
  Unserialise_data* data = (Unserialise_data*) pData;
  data->doc = new Xapian::Document(Xapian::Document::unserialise(data->str));
}

Handle<Value> Document::Unserialise_convert(void* pData) {
  Unserialise_data* data = (Unserialise_data*) pData;
  
  Local<Value> aDoc[] = { External::New(data->doc) };
  Handle<Value> aResult = Document::constructor_template->GetFunction()->NewInstance(1, aDoc);

  delete data;
  return aResult;
}

static int kTermlist[] = { -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Document::Termlist(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kTermlist, args, aOpt))
    return throwSignatureErr(kTermlist);

  Termlist_data* aData = new Termlist_data(aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termlist_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termlist_process, Termlist_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Document::Termlist_process(void* pData, void* pThat) {
  Termlist_data* data = (Termlist_data*) pData;
  Document* that = (Document *) pThat;

  if (that->mDoc->termlist_count() < data->first) {
    data->size = 0;
    return;
  }

  Xapian::termcount aSize  = that->mDoc->termlist_count() - data->first;
  if (data->maxitems != 0 && data->maxitems < aSize)
    aSize = data->maxitems;
  data->tlist = new Termlist_data::Item[aSize];

  Xapian::TermIterator aIt = that->mDoc->termlist_begin();
  for (Xapian::termcount i = 0; i < data->first; ++i)  ++aIt;

  for (data->size = 0; aIt != that->mDoc->termlist_end() && data->size < aSize; ++data->size, ++aIt) {
    data->tlist[data->size].tname = *aIt;
    data->tlist[data->size].wdf = aIt.get_wdf();
    data->tlist[data->size].description = aIt.get_description();
  }
}

Handle<Value> Document::Termlist_convert(void* pData) {
 
  Termlist_data* data = (Termlist_data*) pData;
  Local<Array> aList(Array::New(data->size));
  for (Xapian::termcount a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    aO->Set(String::NewSymbol("tname"      ), String::New(data->tlist[a].tname.c_str()      ));
    aO->Set(String::NewSymbol("wdf"        ), Uint32::New(data->tlist[a].wdf                ));
    aO->Set(String::NewSymbol("description"), String::New(data->tlist[a].description.c_str()));
    aList->Set(a, aO);
  }
  delete data;
  return aList;
}

static int kValues[] = { -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Document::Values(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kValues, args, aOpt))
    return throwSignatureErr(kValues);

  Values_data* aData = new Values_data(aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Values_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Values_process, Values_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Document::Values_process(void* pData, void* pThat) {
  Values_data* data = (Values_data*) pData;
  Document* that = (Document *) pThat;

  Xapian::termcount aSize  = that->mDoc->values_count() - data->first;
  if (data->maxitems != 0 && data->maxitems < aSize)
    aSize = data->maxitems;
  data->vlist = new Values_data::Item[aSize];

  Xapian::ValueIterator aIt = that->mDoc->values_begin();
  for (Xapian::termcount i = 0; i < data->first; ++i)  ++aIt;

  for (data->size = 0; aIt != that->mDoc->values_end() && data->size < aSize; ++data->size, ++aIt) {
    data->vlist[data->size].value = *aIt;
    data->vlist[data->size].valueno = aIt.get_valueno();
    data->vlist[data->size].description = aIt.get_description();
  }
}

Handle<Value> Document::Values_convert(void* pData) {
 
  Values_data* data = (Values_data*) pData;
  Local<Array> aList(Array::New(data->size));
  for (Xapian::termcount a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    aO->Set(String::NewSymbol("value"      ), String::New(data->vlist[a].value.c_str()      ));
    aO->Set(String::NewSymbol("valueno"    ), Uint32::New(data->vlist[a].valueno            ));
    aO->Set(String::NewSymbol("description"), String::New(data->vlist[a].description.c_str()));
    aList->Set(a, aO);
  }
  delete data;
  return aList;
}
