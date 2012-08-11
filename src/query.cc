#include "node-xapian.h"

Persistent<FunctionTemplate> Query::constructor_template;

void Query::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Query"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_length", GetLength);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "empty", Empty);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "serialise", Serialise);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_terms", GetTerms);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "unserialise", Unserialise);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "match_all", MatchAll);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "match_nothing", MatchNothing);

  target->Set(String::NewSymbol("Query"), constructor_template->GetFunction());

  Handle<Object> aO = constructor_template->GetFunction();
  aO->Set(String::NewSymbol("OP_AND"         ), Integer::New(Xapian::Query::OP_AND         ), ReadOnly);
  aO->Set(String::NewSymbol("OP_OR"          ), Integer::New(Xapian::Query::OP_OR          ), ReadOnly);
  aO->Set(String::NewSymbol("OP_AND_NOT"     ), Integer::New(Xapian::Query::OP_AND_NOT     ), ReadOnly);
  aO->Set(String::NewSymbol("OP_XOR"         ), Integer::New(Xapian::Query::OP_XOR         ), ReadOnly);
  aO->Set(String::NewSymbol("OP_AND_MAYBE"   ), Integer::New(Xapian::Query::OP_AND_MAYBE   ), ReadOnly);
  aO->Set(String::NewSymbol("OP_FILTER"      ), Integer::New(Xapian::Query::OP_FILTER      ), ReadOnly);
  aO->Set(String::NewSymbol("OP_NEAR"        ), Integer::New(Xapian::Query::OP_NEAR        ), ReadOnly);
  aO->Set(String::NewSymbol("OP_PHRASE"      ), Integer::New(Xapian::Query::OP_PHRASE      ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_RANGE" ), Integer::New(Xapian::Query::OP_VALUE_RANGE ), ReadOnly);
  aO->Set(String::NewSymbol("OP_SCALE_WEIGHT"), Integer::New(Xapian::Query::OP_SCALE_WEIGHT), ReadOnly);
  aO->Set(String::NewSymbol("OP_ELITE_SET"   ), Integer::New(Xapian::Query::OP_ELITE_SET   ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_GE"    ), Integer::New(Xapian::Query::OP_VALUE_GE    ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_LE"    ), Integer::New(Xapian::Query::OP_VALUE_LE    ), ReadOnly);
  aO->Set(String::NewSymbol("OP_SYNONYM"     ), Integer::New(Xapian::Query::OP_SYNONYM     ), ReadOnly);
}

/*
JS QueryObject:

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#f396e213df0d8bcffa473a75ebf228d6
string

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#f396e213df0d8bcffa473a75ebf228d6
{tname: string, wqf: uint32=1, pos: uint32=0}

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#a2edfc5e6ae285afb9fde26d096c5ed8
{op: string, queries: [QueryObject, ...], parameter: number=0}
 
http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#cd51f1304501311c8dde31dc54c36048
{op: string, left: string, right: string}

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#4004922547fec06087dc8a63ac484f9b
{op: string, query: QueryObject, parameter: number}

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#27acaeb8ab8a0c111805a0139e3dc35b
{op: string, slot: uint32, begin: string, end: string}

http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html#15e26a7bf7de2699377cad9e9448a482
{op: string, slot: uint32, value: string}
*/

static Xapian::Query Parse(Handle<Value> obj) {

  if (obj->IsString())
    return Xapian::Query(*String::Utf8Value(obj));

  if (!obj->IsObject())
    throw Exception::TypeError(String::New("QueryObject invalid, not object or string"));
  Local<Object> aObj = obj->ToObject();
  Local<String> aKey, aKey2;
  Local<Value> aVal, aVal2;

  if (aObj->Has(aKey = String::New("tname"))) {
    aVal = aObj->Get(aKey);
    if (!aVal->IsString())
      throw Exception::TypeError(String::New("QueryObject.tname is string"));
    Xapian::termcount aWqf = 1;
    Xapian::termpos aPos = 0;
    if (aObj->Has(aKey2 = String::New("wqf"))) {
      aVal2 = aObj->Get(aKey2);
      if (!aVal2->IsUint32())
        throw Exception::TypeError(String::New("QueryObject.wqf is uint32"));
      aWqf = aVal2->Uint32Value();
    }
    if (aObj->Has(aKey2 = String::New("pos"))) {
      aVal2 = aObj->Get(aKey2);
      if (!aVal2->IsUint32())
        throw Exception::TypeError(String::New("QueryObject.wqf is uint32"));
      aPos = aVal2->Uint32Value();
    }
    return Xapian::Query(*String::Utf8Value(aVal), aWqf, aPos);
  }

  if (aObj->Has(aKey = String::New("op"))) {
    aVal = aObj->Get(aKey);
    if (!aVal->IsInt32())
      throw Exception::TypeError(String::New("QueryObject.op is invalid"));
    Xapian::Query::op aOp = (Xapian::Query::op)aVal->Int32Value();

    if (aObj->Has(aKey = String::New("queries"))) {
      Xapian::termcount aParameter = 0;
      if (aObj->Has(aKey2 = String::New("parameter"))) {
        aVal2 = aObj->Get(aKey2);
        if (!aVal2->IsUint32())
          throw Exception::TypeError(String::New("QueryObject.parameter is uint32"));
        aParameter = aVal2->Uint32Value();
      }
      aVal = aObj->Get(aKey);
      if (!aVal->IsArray())
        throw Exception::TypeError(String::New("QueryObject.queries is array"));
      std::vector<Xapian::Query> aList;
      Handle<Array> aArr = Handle<Array>::Cast(aVal);
      for (unsigned i = 0; i < aArr->Length(); i++)
        aList.push_back(Parse(aArr->Get(i)));
      return Xapian::Query(aOp, aList.begin(), aList.end(), aParameter);
    }

    if (aObj->Has(aKey = String::New("left"))) {
      aVal = aObj->Get(aKey);
      if (!aVal->IsString())
        throw Exception::TypeError(String::New("QueryObject.left is string"));
      if (!aObj->Has(aKey2 = String::New("right")))
        throw Exception::TypeError(String::New("QueryObject.right is missing"));
      aVal2 = aObj->Get(aKey2);
      if (!aVal2->IsString())
        throw Exception::TypeError(String::New("QueryObject.right is string"));
      return Xapian::Query(aOp, std::string(*String::Utf8Value(aVal)), std::string(*String::Utf8Value(aVal2)));
    }

    if (aObj->Has(aKey = String::New("query"))) {
      aVal = aObj->Get(aKey);
      if (!aVal->IsObject())
        throw Exception::TypeError(String::New("QueryObject.query is QueryObject"));
      double aParameter = 0;
      if (aObj->Has(aKey2 = String::New("parameter"))) {
        aVal2 = aObj->Get(aKey2);
        if (!aVal2->IsNumber())
          throw Exception::TypeError(String::New("QueryObject.parameter is number"));
        aParameter = aVal2->NumberValue();
      }
      return Xapian::Query(aOp, Parse(aVal), aParameter);
    }

    if (aObj->Has(aKey = String::New("slot"))) {
      aVal = aObj->Get(aKey);
      if (!aVal->IsUint32())
        throw Exception::TypeError(String::New("QueryObject.slot is uint32"));
      Xapian::valueno aSlot = aVal2->Uint32Value();

      if (aObj->Has(aKey = String::New("begin"))) {
        aVal = aObj->Get(aKey);
        if (!aVal->IsString())
          throw Exception::TypeError(String::New("QueryObject.begin is string"));
        if (!aObj->Has(aKey2 = String::New("end")))
          throw Exception::TypeError(String::New("QueryObject.end is missing"));
        aVal2 = aObj->Get(aKey2);
        if (!aVal2->IsString())
          throw Exception::TypeError(String::New("QueryObject.end is string"));
        return Xapian::Query(aOp, aSlot, *String::Utf8Value(aVal), *String::Utf8Value(aVal2));
      }

      if (aObj->Has(aKey = String::New("value"))) {
        aVal = aObj->Get(aKey);
        if (!aVal->IsString())
          throw Exception::TypeError(String::New("QueryObject.value is string"));
        return Xapian::Query(aOp, aSlot, *String::Utf8Value(aVal));
      }
    }
  }

  throw Exception::TypeError(String::New("QueryObject invalid"));
}

Handle<Value> Query::New(const Arguments& args) {
  HandleScope scope;
  if (args.Length() > 1)
    return ThrowException(Exception::TypeError(String::New("arguments are (() | QueryObject | string)")));
  Query* that;
  const char* aDesc;

  try {
    if (args.Length() == 1 && !args[0]->IsExternal())
      that = new Query(Parse(args[0]));
    else if (args.Length() == 1 && args[0]->IsExternal())
      that = new Query(*((Xapian::Query*) External::Unwrap(args[0])));
    else
      that = new Query(Xapian::Query());
    aDesc = that->mQry.get_description().c_str();
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  } catch (Handle<Value> ex) {
    return ThrowException(ex);
  }
  args.This()->Set(String::NewSymbol("description"), String::New(aDesc));
  that->Wrap(args.This());
  return args.This();

}

enum { 
  eGetLength, eEmpty, eGetDescription
};


static int kGetLength[] = { -eFunction, eEnd };
Handle<Value> Query::GetLength(const Arguments& args) { return generic_start<Query>(eGetLength, args, kGetLength); }

static int kEmpty[] = { -eFunction, eEnd };
Handle<Value> Query::Empty(const Arguments& args) { return generic_start<Query>(eEmpty, args, kEmpty); }

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> Query::GetDescription(const Arguments& args) { return generic_start<Query>(eGetDescription, args, kGetDescription); }


void Query::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  Query* that = (Query *) pThat;

  switch (data->action) {
  case eGetLength:      data->retVal.uint32 = that->mQry.get_length();        break;
  case eEmpty:          data->retVal.boolean = that->mQry.empty();            break;
  case eGetDescription: data->retVal.setString(that->mQry.get_description()); break;
  default: assert(0);
  }
}

Handle<Value> Query::Generic_convert(void* pData) {
  GenericData* data = (GenericData*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case eGetLength:
    aResult = Integer::NewFromUnsigned(data->retVal.uint32); break;
  case eEmpty:
    aResult = Boolean::New(data->retVal.boolean);            break;
  case eGetDescription: 
    aResult = String::New(data->retVal.string->c_str());     break;
  }

  delete data;
  return aResult;
}

void Query::Termiterator_process(void* pData, void* pThat) {
  Termiterator_data* data = (Termiterator_data*) pData;
  Query* that = (Query *) pThat;

  Xapian::TermIterator aStartIterator = that->mQry.get_terms_begin();
  Xapian::TermIterator aEndIterator = that->mQry.get_terms_end();

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

Handle<Value> Query::Termiterator_convert(void* pData) {
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

static int kGetTerms[] = { -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Query::GetTerms(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kGetTerms, args, aOpt))
    return throwSignatureErr(kGetTerms);

  Termiterator_data* aData = new Termiterator_data(aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSerialise[] = { eEnd };
Handle<Value> Query::Serialise(const Arguments& args) {
  HandleScope scope;
  int aOpt[0];
  if (!checkArguments(kSerialise, args, aOpt))
    return throwSignatureErr(kSerialise);

  Query* that = ObjectWrap::Unwrap<Query>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));

  std::string aSQuery;

  try {
    aSQuery = that->mQry.serialise();
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
 
  return scope.Close(Buffer::New((char *)aSQuery.c_str(), aSQuery.length())->handle_);
}

static int kUnserialise[] = { eBuffer, eEnd };
Handle<Value> Query::Unserialise(const Arguments& args) {
  HandleScope scope;
  int aOpt[0];
  if (!checkArguments(kUnserialise, args, aOpt) || !(Buffer::HasInstance(args[0])))
    return throwSignatureErr(kUnserialise);

  Handle<Object> aBuf = args[0]->ToObject();
  std::string aSQuery(Buffer::Data(aBuf), Buffer::Length(aBuf));

  Xapian::Query aQuery;

  try {
    aQuery = Xapian::Query::unserialise(aSQuery);
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  Local<Value> aQueryParam[] = { External::New(&aQuery) };
  Handle<Object> aResult = Query::constructor_template->GetFunction()->NewInstance(1, aQueryParam);
 
  return scope.Close(aResult);
}

static int kMatchAll[] = { eEnd };
Handle<Value> Query::MatchAll(const Arguments& args) {
  HandleScope scope;
  int aOpt[0];
  if (!checkArguments(kMatchAll, args, aOpt))
    return throwSignatureErr(kMatchAll);

  Xapian::Query aQuery;

  try {
    aQuery = Xapian::Query::MatchAll;
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  Local<Value> aQueryParam[] = { External::New(&aQuery) };
  Handle<Object> aResult = Query::constructor_template->GetFunction()->NewInstance(1, aQueryParam);
 
  return scope.Close(aResult);
}

static int kMatchNothing[] = { eEnd };
Handle<Value> Query::MatchNothing(const Arguments& args) {
  HandleScope scope;
  int aOpt[0];
  if (!checkArguments(kMatchNothing, args, aOpt))
    return throwSignatureErr(kMatchNothing);

  Xapian::Query aQuery;

  try {
    aQuery = Xapian::Query::MatchNothing;
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  Local<Value> aQueryParam[] = { External::New(&aQuery) };
  Handle<Object> aResult = Query::constructor_template->GetFunction()->NewInstance(1, aQueryParam);
 
  return scope.Close(aResult);
}

