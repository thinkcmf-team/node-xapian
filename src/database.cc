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
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "has_positions", HasPositions);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_doccount", GetDoccount);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_lastdocid", GetLastdocid);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_avlength", GetAvlength);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_termfreq", GetTermfreq);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "term_exists", TermExists);

  target->Set(String::NewSymbol("Database"), constructor_template->GetFunction());
}

static int kNewDatabase[] = { -eString, -eFunction, eEnd };
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

static int kClose[] = { -eFunction, eEnd };
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

static int kReopen[] = { -eFunction, eEnd };
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

static int kKeepAlive[] = { -eFunction, eEnd };
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

static int kAddDatabase[] = { eObjDatabase, -eFunction, eEnd };
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


static int kGetDocument[] = { eUint32, -eFunction, eEnd };
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

void Database::Generic_process(void* pData, void* pThat) {
  Generic_data* data = (Generic_data*) pData;
  Database* that = (Database *) pThat;

  switch (data->action) {
  case Generic_data::eGetDescription: data->str1 = that->mDb->get_description();        break;
  case Generic_data::eHasPositions:   data->val1 = that->mDb->has_positions();          break;
  case Generic_data::eGetDoccount:    data->val1 = that->mDb->get_doccount();           break;
  case Generic_data::eGetLastdocid:   data->val1 = that->mDb->get_lastdocid();          break;
  case Generic_data::eGetAvlength:    data->vald1 = that->mDb->get_avlength();          break;
  case Generic_data::eGetTermfreq:    data->val1 = that->mDb->get_termfreq(data->str1); break;
  case Generic_data::eTermExists:     data->val1 = that->mDb->term_exists(data->str1);  break;
  }
}

Handle<Value> Database::Generic_convert(void* pData) {
  Generic_data* data = (Generic_data*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case Generic_data::eGetDescription: aResult = String::New(data->str1.c_str()); break;

  case Generic_data::eTermExists:
  case Generic_data::eHasPositions:   aResult = Boolean::New(data->val1);        break;

  case Generic_data::eGetTermfreq:
  case Generic_data::eGetLastdocid:
  case Generic_data::eGetDoccount:    aResult = Uint32::New(data->val1);         break;

  case Generic_data::eGetAvlength:    aResult = Number::New(data->vald1);        break;
  }

  delete data;
  return aResult;
}

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> Database::GetDescription(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDescription, args, aOpt))
    return throwSignatureErr(kGetDescription);

  Generic_data* aData = new Generic_data(Generic_data::eGetDescription); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kHasPositions[] = { -eFunction, eEnd };
Handle<Value> Database::HasPositions(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kHasPositions, args, aOpt))
    return throwSignatureErr(kHasPositions);

  Generic_data* aData = new Generic_data(Generic_data::eHasPositions); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetDoccount[] = { -eFunction, eEnd };
Handle<Value> Database::GetDoccount(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDoccount, args, aOpt))
    return throwSignatureErr(kGetDoccount);

  Generic_data* aData = new Generic_data(Generic_data::eGetDoccount); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetLastdocid[] = { -eFunction, eEnd };
Handle<Value> Database::GetLastdocid(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetLastdocid, args, aOpt))
    return throwSignatureErr(kGetLastdocid);

  Generic_data* aData = new Generic_data(Generic_data::eGetLastdocid); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetAvlength[] = { -eFunction, eEnd };
Handle<Value> Database::GetAvlength(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetAvlength, args, aOpt))
    return throwSignatureErr(kGetAvlength);

  Generic_data* aData = new Generic_data(Generic_data::eGetAvlength); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetTermfreq[] = { eString, -eFunction, eEnd };
Handle<Value> Database::GetTermfreq(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetTermfreq, args, aOpt))
    return throwSignatureErr(kGetTermfreq);

  Generic_data* aData = new Generic_data(Generic_data::eGetTermfreq, *String::Utf8Value(args[0])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kTermExists[] = { eString, -eFunction, eEnd };
Handle<Value> Database::TermExists(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kTermExists, args, aOpt))
    return throwSignatureErr(kTermExists);

  Generic_data* aData = new Generic_data(Generic_data::eTermExists, *String::Utf8Value(args[0])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}
