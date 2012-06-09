#include "node-xapian.h"

Persistent<FunctionTemplate> WritableDatabase::constructor_template;

void WritableDatabase::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("WritableDatabase"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "replace_document", ReplaceDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "commit", Commit);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "begin_transaction", BeginTransaction);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "commit_transaction", CommitTransaction);

  target->Set(String::NewSymbol("DB_OPEN"               ), Integer::New(Xapian::DB_OPEN               ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE"             ), Integer::New(Xapian::DB_CREATE             ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE_OR_OPEN"     ), Integer::New(Xapian::DB_CREATE_OR_OPEN     ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE_OR_OVERWRITE"), Integer::New(Xapian::DB_CREATE_OR_OVERWRITE), ReadOnly);

  target->Set(String::NewSymbol("WritableDatabase"), constructor_template->GetFunction());
}

Handle<Value> WritableDatabase::New(const Arguments& args) {
  HandleScope scope;

  bool aAsync = args.Length() == 3 && args[2]->IsFunction();
  if (args.Length() != +aAsync+2 || !args[0]->IsString() || !args[1]->IsInt32())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, number, [function])")));

  WritableDatabase* that = new WritableDatabase();
  that->Wrap(args.This());

  Open_data* aData = new Open_data(that, args[0]->ToString(), args[1]->Int32Value()); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aAsync, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> WritableDatabase::ReplaceDocument(const Arguments& args) {
  HandleScope scope;

  Document* aDoc;
  if (args.Length() < 3 || !args[0]->IsString() || !(aDoc = GetInstance<Document>(args[1])) || !args[2]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, Document, function)")));

  AddDocument_data* aData;
  try {
    aData = new AddDocument_data(args.This(), Local<Function>::Cast(args[2]), *aDoc->getDoc(), args[0]->ToString());
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(AddDocument_pool, EIO_PRI_DEFAULT, AddDocument_done, aData);

  return Undefined();
}

int WritableDatabase::AddDocument_pool(eio_req* req) {
  AddDocument_data* aData = (AddDocument_data*) req->data;

  try {
    if (aData->idterm.length())
      aData->docid = aData->object->mWdb->replace_document(*aData->idterm, aData->document);
    else
      aData->docid = aData->object->mWdb->add_document(aData->document);
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  aData->poolDone();
  return 0;
}

int WritableDatabase::AddDocument_done(eio_req* req) {
  HandleScope scope;

  AddDocument_data* aData = (AddDocument_data*) req->data;

  Handle<Value> argv[2];
  if (aData->error) {
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));
  } else {
    argv[0] = Null();
    argv[1] = Integer::New(aData->docid);
  }

  tryCallCatch(aData->callback, aData->object->handle_, aData->error ? 1 : 2, argv);

  delete aData;

  return 0;
}

Handle<Value> WritableDatabase::Commit(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 || !args[0]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (function)")));
  Commit_data* aData;
  try {
    aData = new Commit_data(args.This(), Local<Function>::Cast(args[0]), Commit_data::eCommit);
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(Commit_pool, EIO_PRI_DEFAULT, Commit_done, aData);

  return Undefined();
}

Handle<Value> WritableDatabase::BeginTransaction(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 || !args[0]->IsBoolean() || !args[1]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (boolean, function)")));
  Commit_data* aData;
  try {
    aData = new Commit_data(args.This(), Local<Function>::Cast(args[1]), Commit_data::eBeginTx, args[0]->BooleanValue());
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(Commit_pool, EIO_PRI_DEFAULT, Commit_done, aData);

  return Undefined();
}

Handle<Value> WritableDatabase::CommitTransaction(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 || !args[0]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (function)")));
  Commit_data* aData;
  try {
    aData = new Commit_data(args.This(), Local<Function>::Cast(args[0]), Commit_data::eCommitTx);
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(Commit_pool, EIO_PRI_DEFAULT, Commit_done, aData);

  return Undefined();
}

int WritableDatabase::Commit_pool(eio_req* req) {
  Commit_data* aData = (Commit_data*) req->data;

  try {
    switch (aData->type) {
    case Commit_data::eCommit:   aData->object->mWdb->commit();                        break;
    case Commit_data::eBeginTx:  aData->object->mWdb->begin_transaction(aData->flush); break;
    case Commit_data::eCommitTx: aData->object->mWdb->commit_transaction();            break;
    }
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  aData->poolDone();
  return 0;
}

int WritableDatabase::Commit_done(eio_req* req) {
  HandleScope scope;

  Commit_data* aData = (Commit_data*) req->data;

  Handle<Value> argv[1];
  if (aData->error)
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));

  tryCallCatch(aData->callback, aData->object->handle_, aData->error ? 1 : 0, argv);

  delete aData;

  return 0;
}
