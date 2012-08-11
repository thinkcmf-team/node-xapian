#include "node-xapian.h"

Persistent<FunctionTemplate> TermGenerator::constructor_template;

void TermGenerator::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("TermGenerator"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_database", SetDatabase);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_flags", SetFlags);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_stemmer", SetStemmer);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_document", SetDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_document", GetDocument);

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

void TermGenerator::SetGet_process(void* pData, void* pThat) {
  SetGet_data* data = (SetGet_data*) pData;
  TermGenerator* that = (TermGenerator *) pThat;

  switch (data->action) {
  case SetGet_data::eSetDatabase: that->mTg.set_database(data->db->getWdb());                  break;
  case SetGet_data::eSetStemmer:  that->mTg.set_stemmer(data->st->mStem);                      break;
  case SetGet_data::eSetDocument: that->mTg.set_document(*data->doc->getDoc());                break;
  case SetGet_data::eGetDocument: data->xdoc = new Xapian::Document(that->mTg.get_document()); break;
  default: assert(0);
  }
}

Handle<Value> TermGenerator::SetGet_convert(void* pData) {
  SetGet_data* data = (SetGet_data*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case SetGet_data::eSetDatabase:    
  case SetGet_data::eSetStemmer:
  case SetGet_data::eSetDocument:
    aResult = Undefined(); break;
  case SetGet_data::eGetDocument:
    Local<Value> aDoc[] = { External::New(data->xdoc) };
    aResult = Document::constructor_template->GetFunction()->NewInstance(1, aDoc);
    break;
  }

  delete data;
  return aResult;
}

static int kSetDatabase[] = { eObjectDatabase, -eFunction, eEnd };
Handle<Value> TermGenerator::SetDatabase(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  WritableDatabase* aDb;
  if (!checkArguments(kSetDatabase, args, aOpt) || !(aDb = GetInstance<WritableDatabase>(args[0])))
    return throwSignatureErr(kSetDatabase);

  SetGet_data* aData = new SetGet_data(ObjectWrap::Unwrap<TermGenerator>(args.This()), aDb); //deleted by SetGet_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<TermGenerator>(aOpt[0] >= 0, args, (void*)aData, SetGet_process, SetGet_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
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

static int kSetStemmer[] = { eObjectStem, -eFunction, eEnd };
Handle<Value> TermGenerator::SetStemmer(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  Stem* aSt;
  if (!checkArguments(kSetStemmer, args, aOpt) || !(aSt = GetInstance<Stem>(args[0])))
    return throwSignatureErr(kSetStemmer);

  SetGet_data* aData = new SetGet_data(ObjectWrap::Unwrap<TermGenerator>(args.This()), aSt); //deleted by SetGet_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<TermGenerator>(aOpt[0] >= 0, args, (void*)aData, SetGet_process, SetGet_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSetDocument[] = { eObjectDocument, -eFunction, eEnd };
Handle<Value> TermGenerator::SetDocument(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  Document* aDoc;
  if (!checkArguments(kSetDocument, args, aOpt) || !(aDoc = GetInstance<Document>(args[0])))
    return throwSignatureErr(kSetDocument);

  SetGet_data* aData = new SetGet_data(ObjectWrap::Unwrap<TermGenerator>(args.This()), aDoc); //deleted by SetGet_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<TermGenerator>(aOpt[0] >= 0, args, (void*)aData, SetGet_process, SetGet_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetDocument[] = { -eFunction, eEnd };
Handle<Value> TermGenerator::GetDocument(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDocument, args, aOpt))
    return throwSignatureErr(kGetDocument);

  SetGet_data* aData = new SetGet_data(ObjectWrap::Unwrap<TermGenerator>(args.This())); //deleted by SetGet_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] >= 0, args, (void*)aData, SetGet_process, SetGet_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}
