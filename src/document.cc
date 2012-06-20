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

Handle<Value> Document::GetValue(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 2 && args[1]->IsFunction();
  if (args.Length() != +aAsync+1 || !args[0]->IsUint32())
    return ThrowException(Exception::TypeError(String::New("arguments are (uint32, [function])")));

  Generic_data* aData = new Generic_data(Generic_data::eGetValue, args[0]->Uint32Value()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> Document::AddValue(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 3 && args[2]->IsFunction();
  if (args.Length() != +aAsync+2 || !args[0]->IsUint32() || !args[1]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are (uint32, string, [function])")));

  Generic_data* aData = new Generic_data(Generic_data::eAddValue, (char*)*args[1]->ToString(), args[0]->Uint32Value()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> Document::RemoveValue(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 2 && args[1]->IsFunction();
  if (args.Length() != +aAsync+1 || !args[0]->IsUint32())
    return ThrowException(Exception::TypeError(String::New("arguments are (uint32, [function])")));

  Generic_data* aData = new Generic_data(Generic_data::eRemoveValue, args[0]->Uint32Value()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> Document::ClearValues(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 1 && args[0]->IsFunction();
  if (args.Length() != +aAsync)
    return ThrowException(Exception::TypeError(String::New("arguments are ([function])")));

  Generic_data* aData = new Generic_data(Generic_data::eClearValues); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
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

Handle<Value> Document::SetData(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 2 && args[1]->IsFunction();
  if (args.Length() != +aAsync+1 || !args[0]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, [function])")));

  Generic_data* aData = new Generic_data(Generic_data::eSetData, (char*)*args[0]->ToString()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> Document::AddPosting(const Arguments& args) {
  HandleScope scope;
  bool aAsync = (args.Length() == 3 && args[2]->IsFunction()) || (args.Length() == 4 && args[3]->IsFunction());
  bool aHasWdfinc = (args.Length()>=3 && args[2]->IsUint32());
  if (args.Length() != +aAsync+aHasWdfinc+2 || !args[0]->IsString() || !args[1]->IsUint32())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, uint32, [uint32], [function])")));

  Generic_data* aData = new Generic_data(Generic_data::eAddPosting, (char*)*args[0]->ToString(), args[1]->Uint32Value(), aHasWdfinc?args[2]->Uint32Value():1); //deleted by Generic_convert on non error

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
  case Generic_data::eGetValue:        data->str = that->mDoc->get_value(data->val1);                 break;
  case Generic_data::eAddValue:        that->mDoc->add_value(data->val1, data->str);                  break;
  case Generic_data::eRemoveValue:     that->mDoc->remove_value(data->val1);                          break;
  case Generic_data::eClearValues:     that->mDoc->clear_values();                                    break;
  case Generic_data::eGetData:         data->str = that->mDoc->get_data();                            break;
  case Generic_data::eSetData:         that->mDoc->set_data(data->str);                               break;
  case Generic_data::eAddPosting:      that->mDoc->add_posting(data->str, data->val1, data->val2);    break;
  case Generic_data::eAddTerm:         that->mDoc->add_term(data->str, data->val1);                   break;
  case Generic_data::eAddBooleanTerm:  that->mDoc->add_boolean_term(data->str);                       break;
  case Generic_data::eRemovePosting:   that->mDoc->remove_posting(data->str, data->val1, data->val2); break;
  case Generic_data::eRemoveTerm:      that->mDoc->remove_term(data->str);                            break;
  case Generic_data::eClearTerms:      that->mDoc->clear_terms();                                     break;
  case Generic_data::eTermlistCount:   data->val1 = that->mDoc->termlist_count();                     break;
  case Generic_data::eValuesCount:     data->val1 = that->mDoc->values_count();                       break;
  case Generic_data::eGetDocid:        data->val1 = that->mDoc->get_docid();                          break;
  case Generic_data::eSerialise:       data->str = that->mDoc->serialise();                           break;
  case Generic_data::eGetDescription:  data->str = that->mDoc->get_description();                     break;
  }
}

Handle<Value> Document::Generic_convert(void* pData) {
  Generic_data* data = (Generic_data*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case Generic_data::eTermlistCount:
  case Generic_data::eValuesCount:
  case Generic_data::eGetDocid:       aResult = Integer::NewFromUnsigned(data->val1); break;
  case Generic_data::eGetValue:
  case Generic_data::eGetData:  
  case Generic_data::eSerialise:
  case Generic_data::eGetDescription: aResult = String::New(data->str.c_str());       break;
  case Generic_data::eAddValue:
  case Generic_data::eRemoveValue:
  case Generic_data::eClearValues:
  case Generic_data::eSetData:
  case Generic_data::eAddPosting:
  case Generic_data::eAddTerm:
  case Generic_data::eAddBooleanTerm:
  case Generic_data::eRemovePosting:
  case Generic_data::eRemoveTerm:
  case Generic_data::eClearTerms:     aResult = Undefined();                          break;
  }

  delete data;
  return aResult;
}
