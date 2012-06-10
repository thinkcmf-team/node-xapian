#include "node-xapian.h"

Persistent<FunctionTemplate> WritableDatabase::constructor_template;

void WritableDatabase::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("WritableDatabase"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_document", AddDocument);
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
  bool aAsync = args.Length() == 3 && args[2]->IsFunction();
  if (args.Length() != +aAsync+2 || !(args[0]->IsString() || args[0]->IsUint32()) || !(aDoc = GetInstance<Document>(args[1])))
    return ThrowException(Exception::TypeError(String::New("arguments are (string, Document, [function]) or (uint32, Document, [function])")));

  ReplaceDocument_data* aData;

  if (args[0]->IsString()) {
    if (args[0]->ToString()->Length())
      aData = new ReplaceDocument_data(*aDoc->getDoc(),args[0]->ToString()); //deleted by ReplaceDocument_convert on non error
    else
      aData = new ReplaceDocument_data(*aDoc->getDoc()); //deleted by ReplaceDocument_convert on non error
  }
  else
    aData = new ReplaceDocument_data(*aDoc->getDoc(),args[0]->Uint32Value()); //deleted by ReplaceDocument_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aAsync, args, (void*)aData, ReplaceDocument_process, ReplaceDocument_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

Handle<Value> WritableDatabase::AddDocument(const Arguments& args) {
  HandleScope scope;

  Document* aDoc;
  bool aAsync = args.Length() == 2 && args[1]->IsFunction();
  if (args.Length() != +aAsync+1 || !(aDoc = GetInstance<Document>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Document, [function])")));

  ReplaceDocument_data* aData = new ReplaceDocument_data(*aDoc->getDoc()); //deleted by ReplaceDocument_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aAsync, args, (void*)aData, ReplaceDocument_process, ReplaceDocument_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void WritableDatabase::ReplaceDocument_process(void* pData, void* pThat) {
  ReplaceDocument_data* data = (ReplaceDocument_data*) pData;
  WritableDatabase* that = (WritableDatabase *) pThat;

  switch (data->action) {
    case ReplaceDocument_data::eAdd:
      data->docid = that->mWdb->add_document(data->document);
      break;
    case ReplaceDocument_data::eRepleceTerm:
      data->docid = that->mWdb->replace_document(*data->idterm, data->document);
      break;
    case ReplaceDocument_data::eReplaceDocId:
      that->mWdb->replace_document(data->docid, data->document);
      break;
  }
}

Handle<Value> WritableDatabase::ReplaceDocument_convert(void* pData) {
  ReplaceDocument_data* data = (ReplaceDocument_data*) pData;

  Handle<Value> aResult;

  switch (data->action) {
    case ReplaceDocument_data::eAdd:
    case ReplaceDocument_data::eRepleceTerm:
      aResult = Integer::NewFromUnsigned(data->docid);
      break;
    case ReplaceDocument_data::eReplaceDocId:
      aResult = Undefined();
      break;
  }

  delete data;
  return aResult;
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
