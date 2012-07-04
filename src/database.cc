#include "node-xapian.h"

Persistent<FunctionTemplate> Database::constructor_template;

void Database::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "reopen", Reopen);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "keep_alive", KeepAlive);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_database", AddDatabase);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_document", GetDocument);

  target->Set(String::NewSymbol("Database"), constructor_template->GetFunction());
}

int kNewDatabase[] = { -eString, -eFunction, eEnd };
Handle<Value> Database::New(const Arguments& args) {
  HandleScope scope;
  int aOpt[2];
  if (!checkArguments(kNewDatabase, args, aOpt))
    return throwSignatureErr(kNewDatabase);

  Database* that = new Database();
  that->Wrap(args.This());

  Open_data* aData = new Open_data(Open_data::eNewDatabase, that, aOpt[0] < 0 ? Handle<String>() : args[aOpt[0]]->ToString()); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[1] != -1, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Database::Open_process(void* pData, void* pThat) {
  Open_data* data = (Open_data*) pData;
  Database* that = (Database *) pThat;
  switch (data->action) {
  case Open_data::eNewDatabase: that->mDb =  data->filename.length() == 0 ? new Xapian::Database() : new Xapian::Database(*data->filename); break;
  case Open_data::eNewWDatabase: that->mDb =  
    data->filename.length() == 0 ? new Xapian::WritableDatabase() : new Xapian::WritableDatabase(*data->filename, data->writeopts); break;
  case Open_data::eReopen:      that->mDb->reopen();                     break;
  case Open_data::eClose:       that->mDb->close();                      break;
  case Open_data::eKeepAlive:   that->mDb->keep_alive();                 break;
  case Open_data::eAddDatabase: that->mDb->add_database(*data->db->mDb); break;
  }
}

Handle<Value> Database::Open_convert(void* pData) {
  Open_data* data = (Open_data*) pData;
  Database* that = data->that;
  delete data;

  switch (data->action) {
  case Open_data::eNewDatabase:
  case Open_data::eNewWDatabase: return that->handle_;

  case Open_data::eClose: 
  case Open_data::eReopen: 
  case Open_data::eKeepAlive:
  case Open_data::eAddDatabase:
  default:                       return Undefined();
  }

}

int kClose[] = { -eFunction, eEnd };
Handle<Value> Database::Close(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kClose, args, aOpt))
    return throwSignatureErr(kClose);

  Open_data* aData = new Open_data(Open_data::eClose); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

int kReopen[] = { -eFunction, eEnd };
Handle<Value> Database::Reopen(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kReopen, args, aOpt))
    return throwSignatureErr(kReopen);

  Open_data* aData = new Open_data(Open_data::eReopen); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

int kKeepAlive[] = { -eFunction, eEnd };
Handle<Value> Database::KeepAlive(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kKeepAlive, args, aOpt))
    return throwSignatureErr(kKeepAlive);

  Open_data* aData = new Open_data(Open_data::eKeepAlive); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

int kAddDatabase[] = { eObjDatabase, -eFunction, eEnd };
Handle<Value> Database::AddDatabase(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kAddDatabase, args, aOpt))
    return throwSignatureErr(kAddDatabase);

  Database* aDb = GetInstance<Database>(args[0]);
  Database* that = ObjectWrap::Unwrap<Database>(args.This());

  Open_data* aData = new Open_data(Open_data::eAddDatabase, that, aDb); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}


int kGetDocument[] = { eUint32, -eFunction, eEnd };
Handle<Value> Database::GetDocument(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDocument, args, aOpt))
    return throwSignatureErr(kGetDocument);

  GetDocument_data* aData = new GetDocument_data(args[0]->Uint32Value()); //deleted by Getdocument_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, GetDocument_process, GetDocument_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Database::GetDocument_process(void* pData, void* pThat) {
  GetDocument_data* data = (GetDocument_data*) pData;
  Database* that = (Database *) pThat;
  data->doc = new Xapian::Document(that->mDb->get_document(data->docid));
}

Handle<Value> Database::GetDocument_convert(void* pData) {
  GetDocument_data* data = (GetDocument_data*) pData;
  
  Local<Value> aDoc[] = { External::New(data->doc) };
  Handle<Value> aResult = Document::constructor_template->GetFunction()->NewInstance(1, aDoc);

  delete data;
  return aResult;
}
