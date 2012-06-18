#include "node-xapian.h"

Persistent<FunctionTemplate> Query::constructor_template;

void Query::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Query"));

  //NODE_SET_PROTOTYPE_METHOD(constructor_template, "fn", Fn);

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

//TODO: add JS query spec
static Xapian::Query Parse(Handle<Value> obj) {

  if (obj->IsString()) {
    return Xapian::Query(*String::Utf8Value(obj));
  }

  if (!obj->IsObject())
    throw Exception::TypeError(String::New("QueryObject invalid, not object or string"));
  Local<Object> aObj = obj->ToObject();
  Local<String> aKey, aKey2, aKey3;
  Local<Value> aVal, aVal2, aVal3;

  if (aObj->Has(aKey = String::New("tname"))) {
    aVal = aObj->Get(aKey);
    if (!aVal->IsString())
      throw Exception::TypeError(String::New("QueryObject.tname is string"));
    unsigned aWqf = 1, aPos = 0;
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
    Xapian::Query::op op = (Xapian::Query::op)aVal->Int32Value();

    if (aObj->Has(aKey = String::New("queries"))) {

      unsigned aParameter = 0;
      if (aObj->Has(aKey2 = String::New("parameter"))) {
        aVal2 = aObj->Get(aKey2);
        if (!aVal2->IsUint32())
          throw Exception::TypeError(String::New("QueryObject.parameter is uint32"));
        aParameter = aVal2->Uint32Value();
      }
      if (!aVal->IsArray())
        throw Exception::TypeError(String::New("QueryObject.queries is array"));
      std::vector<Xapian::Query> aList;
      aVal = aObj->Get(aKey);
      Handle<Array> aArr = Handle<Array>::Cast(aVal);
      for (unsigned i = 0; i < aArr->Length(); i++)
        aList.push_back(Parse(aArr->Get(Int32::New(i))));
      return Xapian::Query((Xapian::Query::op)op, aList.begin(), aList.end(), aParameter);
    }

    if (aObj->Has(aKey = String::New("left"))) {
      aVal = aObj->Get(aKey);
      if (!aVal->IsString())
        throw Exception::TypeError(String::New("QueryObject.left is string"));
      if (!aObj->Has(aKey2 = String::New("right")))
        throw Exception::TypeError(String::New("QueryObject has left but not right"));
      aVal2 = aObj->Get(aKey2);
      if (!aVal2->IsString())
        throw Exception::TypeError(String::New("QueryObject.right is string"));
      return Xapian::Query((Xapian::Query::op)op, std::string(*String::Utf8Value(aVal)), std::string(*String::Utf8Value(aVal2)));
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



