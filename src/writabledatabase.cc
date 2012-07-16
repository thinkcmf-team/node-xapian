#include "node-xapian.h"

Persistent<FunctionTemplate> WritableDatabase::constructor_template;

void WritableDatabase::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("WritableDatabase"));

  constructor_template->Inherit(Database::constructor_template);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_document", AddDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "replace_document", ReplaceDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "commit", Commit);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "begin_transaction", BeginTransaction);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "commit_transaction", CommitTransaction);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "cancel_transaction", CancelTransaction);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "delete_document", DeleteDocument);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_spelling", AddSpelling);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_spelling", RemoveSpelling);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_synonym", AddSynonym);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove_synonym", RemoveSynonym);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "clear_synonyms", ClearSynonyms);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_metadata", SetMetadata);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_description", GetDescription);

  target->Set(String::NewSymbol("DB_OPEN"               ), Integer::New(Xapian::DB_OPEN               ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE"             ), Integer::New(Xapian::DB_CREATE             ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE_OR_OPEN"     ), Integer::New(Xapian::DB_CREATE_OR_OPEN     ), ReadOnly);
  target->Set(String::NewSymbol("DB_CREATE_OR_OVERWRITE"), Integer::New(Xapian::DB_CREATE_OR_OVERWRITE), ReadOnly);

  target->Set(String::NewSymbol("WritableDatabase"), constructor_template->GetFunction());
}

static int kNew[] = { -eString, -eInt32, -eFunction, eEnd };
Handle<Value> WritableDatabase::New(const Arguments& args) {
  HandleScope scope;

  int aOpt[3];
  if (!checkArguments(kNew, args, aOpt))
    return throwSignatureErr(kNew);
  if (!(aOpt[0] == aOpt[1] || (aOpt[0] != -1 && aOpt[1] != -1)))
    return ThrowException(Exception::TypeError(String::New("arguments are ([function]) or (string, number, [function])")));

  WritableDatabase* that = new WritableDatabase();
  that->Wrap(args.This());

  Open_data* aData = new Open_data(Open_data::eNewWDatabase, that, aOpt[0] < 0 ? Handle<String>() : args[aOpt[0]]->ToString(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Int32Value()); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[2] != -1, args, (void*)aData, Open_process, Open_convert);
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
  if (args.Length() != +aAsync+2 || !(args[0]->IsString() || args[0]->IsUint32() || args[0]->IsNull()) || !(aDoc = GetInstance<Document>(args[1])))
    return ThrowException(Exception::TypeError(String::New("arguments are (string, Document, [function]) or (uint32, Document, [function])")));

  ReplaceDocument_data* aData;

  if (args[0]->IsNull())
    aData = new ReplaceDocument_data(*aDoc->getDoc()); //deleted by ReplaceDocument_convert on non error
  else if (args[0]->IsString())
    aData = new ReplaceDocument_data(*aDoc->getDoc(),args[0]->ToString()); //deleted by ReplaceDocument_convert on non error
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

static int kAddDocument[] = { eObjectDocument, -eFunction, eEnd };
Handle<Value> WritableDatabase::AddDocument(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  Document* aDoc;
  if (!checkArguments(kAddDocument, args, aOpt) || !(aDoc = GetInstance<Document>(args[0])))
    return throwSignatureErr(kAddDocument);

  ReplaceDocument_data* aData = new ReplaceDocument_data(*aDoc->getDoc()); //deleted by ReplaceDocument_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, ReplaceDocument_process, ReplaceDocument_convert);
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
  case ReplaceDocument_data::eAdd:          data->docid = that->mWdb->add_document(data->document);                    break;
  case ReplaceDocument_data::eReplaceTerm:  data->docid = that->mWdb->replace_document(*data->idterm, data->document); break;
  case ReplaceDocument_data::eReplaceDocId: that->mWdb->replace_document(data->docid, data->document);                 break;
  }
}

Handle<Value> WritableDatabase::ReplaceDocument_convert(void* pData) {
  ReplaceDocument_data* data = (ReplaceDocument_data*) pData;

  Handle<Value> aResult;

  switch (data->action) {
  case ReplaceDocument_data::eAdd:
  case ReplaceDocument_data::eReplaceTerm:  aResult = Integer::NewFromUnsigned(data->docid); break;
  case ReplaceDocument_data::eReplaceDocId: aResult = Undefined();                           break;
  }

  delete data;
  return aResult;
}

static int kCommit[] = { -eFunction, eEnd };
Handle<Value> WritableDatabase::Commit(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kCommit, args, aOpt))
    return throwSignatureErr(kCommit);

  Generic_data* aData = new Generic_data(Generic_data::eCommit); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kBeginTransaction[] = { -eBoolean, -eFunction, eEnd };
Handle<Value> WritableDatabase::BeginTransaction(const Arguments& args) {
  HandleScope scope;
  int aOpt[2];
  if (!checkArguments(kBeginTransaction, args, aOpt))
    return throwSignatureErr(kBeginTransaction);

  Generic_data* aData = new Generic_data(Generic_data::eBeginTx, aOpt[0] < 0 ? true : args[aOpt[0]]->BooleanValue()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[1] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kCommitTransaction[] = { -eFunction, eEnd };
Handle<Value> WritableDatabase::CommitTransaction(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kCommitTransaction, args, aOpt))
    return throwSignatureErr(kCommitTransaction);

  Generic_data* aData = new Generic_data(Generic_data::eCommitTx); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kCancelTransaction[] = { -eFunction, eEnd };
Handle<Value> WritableDatabase::CancelTransaction(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kCancelTransaction, args, aOpt))
    return throwSignatureErr(kCancelTransaction);

  Generic_data* aData = new Generic_data(Generic_data::eCancelTx); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kDeleteDocumentDid[] = { eUint32, -eFunction, eEnd };
static int kDeleteDocumentTerm[] = { eString, -eFunction, eEnd };
Handle<Value> WritableDatabase::DeleteDocument(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kDeleteDocumentDid, args, aOpt) && !checkArguments(kDeleteDocumentTerm, args, aOpt))
    return ThrowException(Exception::TypeError(String::New("arguments are (uint32, [function]) or (string, [function])")));

  Generic_data* aData;

  if (args[0]->IsUint32())
    aData = new Generic_data(Generic_data::eDeleteDocumentDid, args[0]->Uint32Value()); //deleted by Generic_convert on non error
  else
    aData = new Generic_data(Generic_data::eDeleteDocumentTerm, *String::Utf8Value(args[0])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kAddSpelling[] = { eString, -eUint32, -eFunction, eEnd };
Handle<Value> WritableDatabase::AddSpelling(const Arguments& args) {
  HandleScope scope;
  int aOpt[2];
  if (!checkArguments(kAddSpelling, args, aOpt))
    return throwSignatureErr(kAddSpelling);

  Generic_data* aData = new Generic_data(Generic_data::eAddSpelling, *String::Utf8Value(args[0]), aOpt[0] < 0 ? 1 : args[aOpt[0]]->Uint32Value()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[1] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kRemoveSpelling[] = { eString, -eUint32, -eFunction, eEnd };
Handle<Value> WritableDatabase::RemoveSpelling(const Arguments& args) {
  HandleScope scope;
  int aOpt[2];
  if (!checkArguments(kRemoveSpelling, args, aOpt))
    return throwSignatureErr(kRemoveSpelling);

  Generic_data* aData = new Generic_data(Generic_data::eRemoveSpelling, *String::Utf8Value(args[0]), aOpt[0] < 0 ? 1 : args[aOpt[0]]->Uint32Value()); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[1] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kAddSynonym[] = { eString, eString, -eFunction, eEnd };
Handle<Value> WritableDatabase::AddSynonym(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kAddSynonym, args, aOpt))
    return throwSignatureErr(kAddSynonym);

  Generic_data* aData = new Generic_data(Generic_data::eAddSynonym, *String::Utf8Value(args[0]), *String::Utf8Value(args[1])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kRemoveSynonym[] = { eString, eString, -eFunction, eEnd };
Handle<Value> WritableDatabase::RemoveSynonym(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kRemoveSynonym, args, aOpt))
    return throwSignatureErr(kRemoveSynonym);

  Generic_data* aData = new Generic_data(Generic_data::eRemoveSynonym, *String::Utf8Value(args[0]), *String::Utf8Value(args[1])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kClearSynonyms[] = { eString, -eFunction, eEnd };
Handle<Value> WritableDatabase::ClearSynonyms(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kClearSynonyms, args, aOpt))
    return throwSignatureErr(kClearSynonyms);

  Generic_data* aData = new Generic_data(Generic_data::eClearSynonyms, *String::Utf8Value(args[0])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSetMetadata[] = { eString, eString, -eFunction, eEnd };
Handle<Value> WritableDatabase::SetMetadata(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kSetMetadata, args, aOpt))
    return throwSignatureErr(kSetMetadata);

  Generic_data* aData = new Generic_data(Generic_data::eSetMetadata, *String::Utf8Value(args[0]), *String::Utf8Value(args[1])); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> WritableDatabase::GetDescription(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  if (!checkArguments(kGetDescription, args, aOpt))
    return throwSignatureErr(kGetDescription);

  Generic_data* aData = new Generic_data(Generic_data::eGetDescription); //deleted by Generic_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] != -1, args, (void*)aData, Generic_process, Generic_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void WritableDatabase::Generic_process(void* pData, void* pThat) {
  Generic_data* data = (Generic_data*) pData;
  WritableDatabase* that = (WritableDatabase *) pThat;

  switch (data->action) {
  case Generic_data::eCommit:             that->mWdb->commit();                                break;
  case Generic_data::eBeginTx:            that->mWdb->begin_transaction(data->val1);           break;
  case Generic_data::eCommitTx:           that->mWdb->commit_transaction();                    break;
  case Generic_data::eCancelTx:           that->mWdb->cancel_transaction();                    break;
  case Generic_data::eDeleteDocumentDid:  that->mWdb->delete_document(data->val1);             break;
  case Generic_data::eDeleteDocumentTerm: that->mWdb->delete_document(data->str1);             break;
  case Generic_data::eAddSpelling:        that->mWdb->add_spelling(data->str1, data->val1);    break;
  case Generic_data::eRemoveSpelling:     that->mWdb->add_spelling(data->str1, data->val1);    break;
  case Generic_data::eAddSynonym:         that->mWdb->add_synonym(data->str1, data->str2);     break;
  case Generic_data::eRemoveSynonym:      that->mWdb->remove_synonym(data->str1, data->str2);  break;
  case Generic_data::eClearSynonyms:      that->mWdb->clear_synonyms(data->str1);              break;
  case Generic_data::eSetMetadata:        that->mWdb->set_metadata(data->str1, data->str2);    break;
  case Generic_data::eGetDescription:     that->mWdb->get_description();                       break;
  }
}

Handle<Value> WritableDatabase::Generic_convert(void* pData) {
  Generic_data* data = (Generic_data*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case Generic_data::eGetDescription:
  case Generic_data::eSetMetadata:
  case Generic_data::eClearSynonyms:
  case Generic_data::eRemoveSynonym:
  case Generic_data::eAddSynonym:
  case Generic_data::eRemoveSpelling:
  case Generic_data::eAddSpelling:
  case Generic_data::eDeleteDocumentTerm:
  case Generic_data::eDeleteDocumentDid:
  case Generic_data::eCancelTx:
  case Generic_data::eCommitTx:
  case Generic_data::eBeginTx:
  case Generic_data::eCommit: aResult = Undefined(); break;
  }
  delete data;
  return Undefined();
}
