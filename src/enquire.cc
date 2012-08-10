#include "node-xapian.h"


Persistent<FunctionTemplate> Enquire::constructor_template;



void Enquire::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Enquire"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_query", SetQuery);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_query", GetQuery);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset", GetMset);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset_sync", GetMset);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_parameters", SetParameters);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_matching_terms", GetMatchingTerms);

  target->Set(String::NewSymbol("Enquire"), constructor_template->GetFunction());

  Handle<Object> aO = constructor_template->GetFunction();
  aO->Set(String::NewSymbol("ASCENDING" ), Integer::New(Xapian::Enquire::ASCENDING ), ReadOnly);
  aO->Set(String::NewSymbol("DESCENDING"), Integer::New(Xapian::Enquire::DESCENDING), ReadOnly);
  aO->Set(String::NewSymbol("DONT_CARE" ), Integer::New(Xapian::Enquire::DONT_CARE ), ReadOnly);

  aO->Set(String::NewSymbol("RELEVANCE" ), Integer::New(-1), ReadOnly);
}

Handle<Value> Enquire::New(const Arguments& args) {
  HandleScope scope;
  Database* aDb;
  if (args.Length() < 1 || !(aDb = GetInstance<Database>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Database)")));
  Enquire* that = new Enquire(aDb->getDb());
  that->Wrap(args.This());
  return args.This();
}

Handle<Value> Enquire::SetQuery(const Arguments& args) {
  HandleScope scope;
  Query* aQ;
  uint32_t aQlen = 0;
  if (args.Length() > 0 && (aQ = GetInstance<Query>(args[0]))) {
    if (args.Length() == 2 && args[1]->IsUint32())
      aQlen = args[1]->Uint32Value();
    else if (args.Length() != 1)
      return ThrowException(Exception::TypeError(String::New("arguments are (Query, [uint32])")));
  }
  else
    return ThrowException(Exception::TypeError(String::New("arguments are (Query, [uint32])")));

  Enquire* that = ObjectWrap::Unwrap<Enquire>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));
  try {
    that->mEnq.set_query(aQ->mQry, aQlen);
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  return Undefined();
}

Handle<Value> Enquire::GetQuery(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 0)
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));

  Enquire* that = ObjectWrap::Unwrap<Enquire>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));

  Xapian::Query aQuery;
  try {
    aQuery = that->mEnq.get_query();
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  Local<Value> aQueryParam[] = { External::New(&aQuery) };
  Handle<Object> aResult = Query::constructor_template->GetFunction()->NewInstance(1, aQueryParam);
 
  return scope.Close(aResult);
}

Handle<Value> Enquire::GetDescription(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 0)
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));

  Enquire* that = ObjectWrap::Unwrap<Enquire>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));

  Handle<Value> aResult;

  try {
    aResult = String::New(that->mEnq.get_description().c_str());
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  return scope.Close(aResult);
}


Handle<Value> Enquire::GetMset(const Arguments& args) {
  HandleScope scope;
  bool aAsync = args.Length() == 3 && args[2]->IsFunction();
  if (args.Length() != +aAsync+2 || !args[0]->IsUint32() || !args[1]->IsUint32())
    return ThrowException(Exception::TypeError(String::New("arguments are (number, number, [function])")));

  GetMset_data* aData = new GetMset_data(args[0]->Uint32Value(), args[1]->Uint32Value()); //deleted by GetMset_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aAsync, args, (void*)aData, GetMset_process, GetMset_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}


void Enquire::GetMset_process(void* pData, void* pThat) {
  GetMset_data* data = (GetMset_data*) pData;
  Enquire* that = (Enquire *) pThat;
  Xapian::MSet aSet = that->mEnq.get_mset(data->first, data->maxitems);
  data->set = new GetMset_data::Item[aSet.size()];
  data->size = 0;
  for (Xapian::MSetIterator a = aSet.begin(); a != aSet.end(); ++a, ++data->size) {
    data->set[data->size].id = *a;
    data->set[data->size].document = new Xapian::Document(a.get_document());
    data->set[data->size].rank = a.get_rank();
    data->set[data->size].collapse_count = a.get_collapse_count();
    data->set[data->size].weight = a.get_weight();
    data->set[data->size].collapse_key = a.get_collapse_key();
    data->set[data->size].description = a.get_description();
    data->set[data->size].percent = a.get_percent();
  }
}

Handle<Value> Enquire::GetMset_convert(void* pData) {
  GetMset_data* data = (GetMset_data*) pData;
  Local<Array> aList(Array::New(data->size));
  Local<Function> aCtor(Document::constructor_template->GetFunction());
  for (int a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    Local<Value> aDoc[] = { External::New(data->set[a].document) };
    aO->Set(String::NewSymbol("document"      ), aCtor->NewInstance(1, aDoc));
    aO->Set(String::NewSymbol("id"            ), Uint32::New(data->set[a].id                  ));
    aO->Set(String::NewSymbol("rank"          ), Uint32::New(data->set[a].rank                ));
    aO->Set(String::NewSymbol("collapse_count"), Uint32::New(data->set[a].collapse_count      ));
    aO->Set(String::NewSymbol("weight"        ), Number::New(data->set[a].weight              ));
    aO->Set(String::NewSymbol("collapse_key"  ), String::New(data->set[a].collapse_key.c_str()));
    aO->Set(String::NewSymbol("description"   ), String::New(data->set[a].description.c_str() ));
    aO->Set(String::NewSymbol("percent"       ),  Int32::New(data->set[a].percent             ));
    aList->Set(a, aO);
  }
  delete data;
  return aList;
}

Handle<Value> Enquire::SetParameters(const Arguments& args) {
  HandleScope scope;
  if (args.Length() != 1 || !args[0]->IsObject())
    return ThrowException(Exception::TypeError(String::New("arguments are (ParametersObject)")));

  Enquire* that = ObjectWrap::Unwrap<Enquire>(args.This());

  Local<Object> aObj, aObj2 = args[0]->ToObject();
  Local<String> aKey, aKey2;
  Local<Value> aVal, aVal2;

  if (aObj->Has(aKey = String::New("collapse_key"))) {
    uint32_t aKeyValue, aMaxValue = 1;

    aVal = aObj->Get(aKey);
    if (!aVal->IsObject())
      return ThrowException(Exception::TypeError(String::New("ParametersObject.collapse_key is Object")));
    aObj2 = aVal->ToObject();
    if (!aObj2->Has(aKey2 = String::New("key")))
      return ThrowException(Exception::TypeError(String::New("ParametersObject.collapse_key.key is missing")));
    aVal2 = aObj2->Get(aKey2);
    if (!aVal2->IsUint32())
      return ThrowException(Exception::TypeError(String::New("QueryObject.collapse_key.key is uint32")));
    aKeyValue = aVal2->Uint32Value();

    if (aObj2->Has(aKey2 = String::New("max"))) {
      aVal2 = aObj2->Get(aKey2);
      if (!aVal2->IsUint32())
        return ThrowException(Exception::TypeError(String::New("QueryObject.collapse_key.max is uint32")));
      aMaxValue = aVal2->Uint32Value();
    }
    try {
      that->mEnq.set_collapse_key(aKeyValue, aMaxValue);
    } catch (const Xapian::Error& err) {
      return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
    }
  }

  if (aObj->Has(aKey = String::New("docid_order"))) {
    aVal = aObj->Get(aKey);
    if (!aVal->IsUint32())
      return ThrowException(Exception::TypeError(String::New("QueryObject.docid_order is docid_order")));
    try {
      that->mEnq.set_docid_order((Xapian::Enquire::docid_order) aVal->Uint32Value());
    } catch (const Xapian::Error& err) {
      return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
    }
  }

  if (aObj->Has(aKey = String::New("cutoff"))) {
    uint32_t aPercentValue;
    double aWeight = 0;

    aVal = aObj->Get(aKey);
    if (!aVal->IsObject())
      return ThrowException(Exception::TypeError(String::New("ParametersObject.cutoff is Object")));
    aObj2 = aVal->ToObject();
    if (!aObj2->Has(aKey2 = String::New("percent")))
      return ThrowException(Exception::TypeError(String::New("ParametersObject.cutoff.percent is missing")));
    aVal2 = aObj2->Get(aKey2);
    if (!aVal2->IsUint32())
      return ThrowException(Exception::TypeError(String::New("QueryObject.cutoff.percent is uint32")));
    aPercentValue = aVal2->Uint32Value();

    if (aObj2->Has(aKey2 = String::New("weight"))) {
      aVal2 = aObj2->Get(aKey2);
      if (!aVal2->IsNumber())
        return ThrowException(Exception::TypeError(String::New("QueryObject.cutoff.weight is number")));
      aWeight = aVal2->NumberValue();
    }
    try {
      that->mEnq.set_cutoff(aPercentValue, aWeight);
    } catch (const Xapian::Error& err) {
      return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
    }
  }

  if (aObj->Has(aKey = String::New("sort"))) {
    aVal = aObj->Get(aKey);
    if (!aVal->IsArray())
      return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is Array")));

    Handle<Array> aArr = Handle<Array>::Cast(aVal);
    if (aArr->Length() > 2)
      return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));

    enum { eRelevance, eValue, eNone };
    int aSort[2] = { eNone, eNone };
    uint32_t aSortKey;
    bool aReverse = false;

    for (unsigned i = 0; i < aArr->Length(); i++) {
      aVal =  aArr->Get(i);
      if (aVal->IsUint32()) {
        aSort[i] = eValue;
        aSortKey = aVal->Uint32Value();
      } else if (aVal->IsInt32() && aVal->Int32Value() == -1)
        aSort[i] = eRelevance;
      else if (aVal->IsObject()) {
        aSort[i] = eValue;
        aObj2 = aVal->ToObject();
        if (!aObj2->Has(aKey2 = String::New("sort_key")))
          return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
        aVal2 = aObj2->Get(aKey2);
        if (!aVal2->IsUint32())
          return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
        aSortKey = aVal2->Uint32Value();
        if (!aObj2->Has(aKey2 = String::New("reverse")))
          return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
        aVal2 = aObj2->Get(aKey2);
        if (!aVal2->IsBoolean())
          return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
        aReverse = aVal2->BooleanValue();
      } else
        return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
    }
    try {
      if (aSort[0] == eRelevance && aSort[1] == eNone) 
        that->mEnq.set_sort_by_relevance();
      else if (aSort[0] == eValue && aSort[1] == eNone)
        that->mEnq.set_sort_by_value(aSortKey, aReverse);
      else if (aSort[0] == eValue && aSort[1] == eRelevance)
        that->mEnq.set_sort_by_value_then_relevance(aSortKey, aReverse);
      else if (aSort[0] == eRelevance && aSort[1] == eValue)
        that->mEnq.set_sort_by_relevance_then_value(aSortKey, aReverse);
      else
        return ThrowException(Exception::TypeError(String::New("ParametersObject.sort is invalid")));
    } catch (const Xapian::Error& err) {
      return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
    }
  }

  return scope.Close(Undefined());
}

void Enquire::Termiterator_process(void* pData, void* pThat) {
  Termiterator_data* data = (Termiterator_data*) pData;
  Enquire* that = (Enquire *) pThat;

  Xapian::TermIterator aStartIterator = that->mEnq.get_matching_terms_begin(data->val);;
  Xapian::TermIterator aEndIterator = that->mEnq.get_matching_terms_end(data->val);

  Xapian::termcount aSize=0;
  for (Xapian::TermIterator aIt = aStartIterator; aIt != aEndIterator; aIt++)
    aSize++;

  if (aSize < data->first ) {
    data->size = 0;
    return;
  }

  aSize -= data->first;
  if (data->maxitems != 0 && data->maxitems < aSize)
    aSize = data->maxitems;
  data->tlist = new Termiterator_data::Item[aSize];


  Xapian::TermIterator aIt = aStartIterator;
  for (Xapian::termcount i = 0; i < data->first; ++i)  ++aIt;

  for (data->size = 0; aIt != aEndIterator && data->size < aSize; ++data->size, ++aIt) {
    data->tlist[data->size].tname = *aIt;
    data->tlist[data->size].wdf = aIt.get_wdf();
    data->tlist[data->size].termfreq = aIt.get_termfreq();
    data->tlist[data->size].description = aIt.get_description();
  }
}

Handle<Value> Enquire::Termiterator_convert(void* pData) {
  Termiterator_data* data = (Termiterator_data*) pData;

  Local<Array> aList(Array::New(data->size));
  for (Xapian::termcount a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    aO->Set(String::NewSymbol("tname"      ), String::New(data->tlist[a].tname.c_str()      ));
    aO->Set(String::NewSymbol("wdf"        ), Uint32::New(data->tlist[a].wdf                ));
    aO->Set(String::NewSymbol("termfreq"   ), Uint32::New(data->tlist[a].termfreq           ));
    aO->Set(String::NewSymbol("description"), String::New(data->tlist[a].description.c_str()));
    aList->Set(a, aO);
  }
  delete data;
  return aList;
}

static int kGetMatchingTerms[] = { eUint32, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Enquire::GetMatchingTerms(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kGetMatchingTerms, args, aOpt))
    return throwSignatureErr(kGetMatchingTerms);

  Termiterator_data* aData = new Termiterator_data(args[0]->Uint32Value(), aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}
