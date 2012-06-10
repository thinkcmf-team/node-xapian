#include "node-xapian.h"

Persistent<FunctionTemplate> TermGenerator::constructor_template;

void TermGenerator::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("TermGenerator"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_database", SetDatabase);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_flags", SetFlags);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_stemmer", SetStemmer);

  target->Set(String::NewSymbol("TermGenerator"), constructor_template->GetFunction());

  Handle<Object> aO = constructor_template->GetFunction();
  aO->Set(String::NewSymbol("FLAG_SPELLING"), Integer::New(Xapian::TermGenerator::FLAG_SPELLING), ReadOnly);
}

Handle<Value> TermGenerator::New(const Arguments& args) {
  HandleScope scope;
  if (args.Length())
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));
  TermGenerator* that = new TermGenerator;
  that->Wrap(args.This());
  return args.This();
}

Handle<Value> TermGenerator::SetDatabase(const Arguments& args) {
  HandleScope scope;
  WritableDatabase* aDb;
  if (args.Length() < 1 || !(aDb = GetInstance<WritableDatabase>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Database)")));

  TermGenerator* that = ObjectWrap::Unwrap<TermGenerator>(args.This());
  try {
    that->mTg.set_database(aDb->getWdb());
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  return Undefined();
}

Handle<Value> TermGenerator::SetFlags(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 1 || !args[0]->IsInt32())
    return ThrowException(Exception::TypeError(String::New("arguments are (integer)")));

  TermGenerator* that = ObjectWrap::Unwrap<TermGenerator>(args.This());
  try {
    that->mTg.set_flags((Xapian::TermGenerator::flags)args[0]->Int32Value());
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  return Undefined();
}

Handle<Value> TermGenerator::SetStemmer(const Arguments& args) {
  HandleScope scope;
  Stem* aSt;
  if (args.Length() < 1 || !(aSt = GetInstance<Stem>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Stem)")));

  TermGenerator* that = ObjectWrap::Unwrap<TermGenerator>(args.This());
  try {
    that->mTg.set_stemmer(aSt->mStem);
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  return Undefined();
}
