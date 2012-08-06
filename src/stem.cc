#include "node-xapian.h"

Persistent<FunctionTemplate> Stem::constructor_template;

void Stem::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Stem"));

  //NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_database", SetDatabase);

  target->Set(String::NewSymbol("Stem"), constructor_template->GetFunction());
}

static int kNewStem[] = { -eString, eEnd };
Handle<Value> Stem::New(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kNewStem, args, aOpt))
    return throwSignatureErr(kNewStem);

  Stem* that;
  try {
    that = aOpt[0] < 0 ? new Stem() : new Stem(*String::Utf8Value(args[aOpt[0]]));
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  that->Wrap(args.This());
  return args.This();
}

static int kGetDescription[] = { eEnd };
Handle<Value> Stem::GetDescription(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDescription, args, aOpt))
    return throwSignatureErr(kGetDescription);

  Stem* that = ObjectWrap::Unwrap<Stem>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));

  Handle<Value> aResult;

  try {
    aResult = String::New(that->mStem.get_description().c_str());
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  return scope.Close(aResult);
}
