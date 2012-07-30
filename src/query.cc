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
  if (args.Length() !=1)
    return ThrowException(Exception::TypeError(String::New("arguments are (QueryObject | string)")));
  Query* that;
  const char* aDesc;
  try {
    that = new Query(Parse(args[0]));
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
  eGetLength, eEmpty, eSerialise, eGetDescription
};


static int kGetLength[] = { -eFunction, eEnd };
Handle<Value> Query::GetLength(const Arguments& args) { return generic_start<Query>(eGetLength, args, kGetLength); }

static int kEmpty[] = { -eFunction, eEnd };
Handle<Value> Query::Empty(const Arguments& args) { return generic_start<Query>(eEmpty, args, kEmpty); }

static int kSerialise[] = { -eFunction, eEnd };
Handle<Value> Query::Serialise(const Arguments& args) { return generic_start<Query>(eSerialise, args, kSerialise); }

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> Query::GetDescription(const Arguments& args) { return generic_start<Query>(eGetDescription, args, kGetDescription); }


void Query::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  Query* that = (Query *) pThat;

  switch (data->action) {
  case eGetLength:      data->retVal.uint32 = that->mQry.get_length();        break;
  case eEmpty:          data->retVal.boolean = that->mQry.empty();            break;
  case eSerialise:      data->retVal.setString(that->mQry.serialise());       break;
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
  case eSerialise:
  case eGetDescription: 
    aResult = String::New(data->retVal.string->c_str());     break;
  }

  delete data;
  return aResult;
}



