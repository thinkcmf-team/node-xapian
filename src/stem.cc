#include "node-xapian.h"

Persistent<FunctionTemplate> Stem::constructor_template;

void Stem::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Stem"));

  //NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_database", SetDatabase);

  target->Set(String::NewSymbol("Stem"), constructor_template->GetFunction());
}

Handle<Value> Stem::New(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 1 || !args[0]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are (string)")));
  Stem* that;
  try {
    that = new Stem(*String::Utf8Value(args[0]));
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  that->Wrap(args.This());
  return args.This();
}
