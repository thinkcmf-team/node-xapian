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
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_collection_freq", GetCollectionFreq);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_value_freq", GetValueFreq);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_value_lower_bound", GetValueLowerBound);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_value_upper_bound", GetValueUpperBound);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_doclength_lower_bound", GetDoclengthLowerBound);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_doclength_upper_bound", GetDoclengthUpperBound);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_wdf_upper_bound", GetWdfUpperBound);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_doclength", GetDoclength);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_spelling_suggestion", GetSpellingSuggestion);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_metadata", GetMetadata);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_uuid", GetUuid);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "termlist", Termlist);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "allterms", Allterms);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "spellings", Spellings);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "synonyms", Synonyms);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "synonym_keys", SynonymKeys);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "metadata_keys", MetadataKeys);

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
    aResult = invoke<Database>(aOpt[1] >= 0, args, (void*)aData, Open_process, Open_convert);
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
  case Open_data::eNewDatabase: 
    that->mDb =  data->filename.length() == 0 ? new Xapian::Database() : new Xapian::Database(*data->filename); break;
  case Open_data::eNewWDatabase: 
    that->mDb = data->filename.length() == 0 ? new Xapian::WritableDatabase() : new Xapian::WritableDatabase(*data->filename, data->writeopts); break;
  case Open_data::eReopen:      that->mDb->reopen();                     break;
  case Open_data::eClose:       that->mDb->close();                      break;
  case Open_data::eKeepAlive:   that->mDb->keep_alive();                 break;
  case Open_data::eAddDatabase: that->mDb->add_database(*data->db->mDb); break;
  default: assert(0);
  }
}

Handle<Value> Database::Open_convert(void* pData) {
  Open_data* data = (Open_data*) pData;
  Database* that = data->that;

  switch (data->action) {
  case Open_data::eNewDatabase:
  case Open_data::eNewWDatabase: 
    return that->handle_;
  case Open_data::eClose: 
  case Open_data::eReopen: 
  case Open_data::eKeepAlive:
  case Open_data::eAddDatabase:  
    return Undefined();
  }

  delete data;
  return Undefined();
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
    aResult = invoke<Database>(aOpt[0] >= 0, args, (void*)aData, Open_process, Open_convert);
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
    aResult = invoke<Database>(aOpt[0] >= 0, args, (void*)aData, Open_process, Open_convert);
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
    aResult = invoke<Database>(aOpt[0] >= 0, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kAddDatabase[] = { eObjectDatabase, -eFunction, eEnd };
Handle<Value> Database::AddDatabase(const Arguments& args) {
  HandleScope scope;
  int aOpt[1];
  Database* aDb;
  if (!checkArguments(kAddDatabase, args, aOpt) || !(aDb = GetInstance<Database>(args[0])))
    return throwSignatureErr(kAddDatabase);

  Open_data* aData = new Open_data(Open_data::eAddDatabase, ObjectWrap::Unwrap<Database>(args.This()), aDb); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aOpt[0] >= 0, args, (void*)aData, Open_process, Open_convert);
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
    aResult = invoke<Enquire>(aOpt[0] >= 0, args, (void*)aData, GetDocument_process, GetDocument_convert);
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


enum { 
  eGetDescription, eHasPositions, eGetDoccount, eGetLastdocid, eGetAvlength, eGetTermfreq, eTermExists, eGetCollectionFreq,
  eGetValueFreq, eGetValueLowerBound, eGetValueUpperBound, eGetDoclengthLowerBound, eGetDoclengthUpperBound, eGetWdfUpperBound,
  eGetDoclength, eGetSpellingSuggestion, eGetMetadata, eGetUuid
};

void Database::Generic_process(void* pData, void* pThat) {
  GenericData* data = (GenericData*) pData;
  Database* that = (Database *) pThat;

  switch (data->action) {
  case eGetDescription:         data->retVal.setString(that->mDb->get_description());                                                  break;
  case eHasPositions:           data->retVal.boolean = that->mDb->has_positions();                                                      break;
  case eGetDoccount:            data->retVal.uint32 = that->mDb->get_doccount();                                                       break;
  case eGetLastdocid:           data->retVal.uint32 = that->mDb->get_lastdocid();                                                      break;
  case eGetAvlength:            data->retVal.dbl = that->mDb->get_avlength();                                                          break;
  case eGetTermfreq:            data->retVal.uint32 = that->mDb->get_termfreq(*data->val[0].string);                                   break;
  case eTermExists:             data->retVal.uint32 = that->mDb->term_exists(*data->val[0].string);                                    break;
  case eGetCollectionFreq:      data->retVal.uint32 = that->mDb->get_collection_freq(*data->val[0].string);                            break;
  case eGetValueFreq:           data->retVal.uint32 = that->mDb->get_value_freq(data->val[0].uint32);                                  break;
  case eGetValueLowerBound:     data->retVal.setString(that->mDb->get_value_lower_bound(data->val[0].uint32));                         break;
  case eGetValueUpperBound:     data->retVal.setString(that->mDb->get_value_upper_bound(data->val[0].uint32));                         break;
  case eGetDoclengthLowerBound: data->retVal.uint32 = that->mDb->get_doclength_lower_bound();                                          break;
  case eGetDoclengthUpperBound: data->retVal.uint32 = that->mDb->get_doclength_upper_bound();                                          break;
  case eGetWdfUpperBound:       data->retVal.uint32 = that->mDb->get_wdf_upper_bound (*data->val[0].string);                           break;
  case eGetDoclength:           data->retVal.uint32 = that->mDb->get_doclength(data->val[0].uint32);                                   break;
  case eGetSpellingSuggestion:  data->retVal.setString(that->mDb->get_spelling_suggestion(*data->val[0].string, data->val[1].uint32)); break;
  case eGetMetadata:            data->retVal.setString(that->mDb->get_metadata(*data->val[0].string));                                 break;
  case eGetUuid:                data->retVal.setString(that->mDb->get_uuid());                                                         break;
  default: assert(0);
  }
}

Handle<Value> Database::Generic_convert(void* pData) {
  GenericData* data = (GenericData*) pData;
  Handle<Value> aResult;

  switch (data->action) {
  case eGetUuid:
  case eGetValueUpperBound:
  case eGetValueLowerBound:
  case eGetDescription: 
  case eGetMetadata:
  case eGetSpellingSuggestion: 
    aResult = String::New(data->retVal.string->c_str()); break;
  case eTermExists:
  case eHasPositions:   
    aResult = Boolean::New(data->retVal.boolean);        break;
  case eGetWdfUpperBound:
  case eGetDoclengthUpperBound:
  case eGetDoclengthLowerBound:
  case eGetCollectionFreq:
  case eGetTermfreq:
  case eGetLastdocid:
  case eGetDoccount:   
  case eGetDoclength:
  case eGetValueFreq:  
    aResult = Uint32::New(data->retVal.uint32);          break;
  case eGetAvlength:    
    aResult = Number::New(data->retVal.dbl);             break;
  }

  delete data;
  return aResult;
}

static int kGetDescription[] = { -eFunction, eEnd };
Handle<Value> Database::GetDescription(const Arguments& args) { return generic_start<Database>(eGetDescription, args, kGetDescription); }

static int kHasPositions[] = { -eFunction, eEnd };
Handle<Value> Database::HasPositions(const Arguments& args) { return generic_start<Database>(eHasPositions, args, kHasPositions); }

static int kGetDoccount[] = { -eFunction, eEnd };
Handle<Value> Database::GetDoccount(const Arguments& args) { return generic_start<Database>(eGetDoccount, args, kGetDoccount); }

static int kGetLastdocid[] = { -eFunction, eEnd };
Handle<Value> Database::GetLastdocid(const Arguments& args) { return generic_start<Database>(eGetLastdocid, args, kGetLastdocid); }

static int kGetAvlength[] = { -eFunction, eEnd };
Handle<Value> Database::GetAvlength(const Arguments& args) { return generic_start<Database>(eGetAvlength, args, kGetAvlength); }

static int kGetTermfreq[] = { eString, -eFunction, eEnd };
Handle<Value> Database::GetTermfreq(const Arguments& args) { return generic_start<Database>(eGetTermfreq, args, kGetTermfreq); }

static int kTermExists[] = { eString, -eFunction, eEnd };
Handle<Value> Database::TermExists(const Arguments& args) { return generic_start<Database>(eTermExists, args, kTermExists); }

static int kGetCollectionFreq[] = { eString, -eFunction, eEnd };
Handle<Value> Database::GetCollectionFreq(const Arguments& args) { return generic_start<Database>(eGetCollectionFreq, args, kGetCollectionFreq); }

static int kGetValueFreq[] = { eUint32, -eFunction, eEnd };
Handle<Value> Database::GetValueFreq(const Arguments& args) { return generic_start<Database>(eGetValueFreq, args, kGetValueFreq); }

static int kGetValueLowerBound[] = { eUint32, -eFunction, eEnd };
Handle<Value> Database::GetValueLowerBound(const Arguments& args) { return generic_start<Database>(eGetValueLowerBound, args, kGetValueLowerBound); }

static int kGetValueUpperBound[] = { eUint32, -eFunction, eEnd };
Handle<Value> Database::GetValueUpperBound(const Arguments& args) { return generic_start<Database>(eGetValueUpperBound, args, kGetValueUpperBound); }

static int kGetDoclengthLowerBound[] = { -eFunction, eEnd };
Handle<Value> Database::GetDoclengthLowerBound(const Arguments& args) { return generic_start<Database>(eGetDoclengthLowerBound, args, kGetDoclengthLowerBound); }

static int kGetDoclengthUpperBound[] = { -eFunction, eEnd };
Handle<Value> Database::GetDoclengthUpperBound(const Arguments& args) { return generic_start<Database>(eGetDoclengthUpperBound, args, kGetDoclengthUpperBound); }

static int kGetWdfUpperBound[] = { eString, -eFunction, eEnd };
Handle<Value> Database::GetWdfUpperBound(const Arguments& args) { return generic_start<Database>(eGetWdfUpperBound, args, kGetWdfUpperBound); }

static int kGetDoclength[] = { eUint32, -eFunction, eEnd };
Handle<Value> Database::GetDoclength(const Arguments& args) { return generic_start<Database>(eGetDoclength, args, kGetDoclength); }

static int kGetSpellingSuggestion[] = { eString, -eUint32, -eFunction, eEnd };
static GenericData::Item kGetSpellingSuggestionDefault[1] = { (uint32_t)2 };
Handle<Value> Database::GetSpellingSuggestion(const Arguments& args) { return generic_start<Database>(eGetSpellingSuggestion, args, kGetSpellingSuggestion, kGetSpellingSuggestionDefault); }

static int kGetMetadata[] = { eString, -eFunction, eEnd };
Handle<Value> Database::GetMetadata(const Arguments& args) { return generic_start<Database>(eGetMetadata, args, kGetMetadata); }

static int kGetUuid[] = { -eFunction, eEnd };
Handle<Value> Database::GetUuid(const Arguments& args) { return generic_start<Database>(eGetUuid, args, kGetUuid); }


void Database::Termiterator_process(void* pData, void* pThat) {
  Termiterator_data* data = (Termiterator_data*) pData;
  Database* that = (Database *) pThat;

  Xapian::TermIterator aStartIterator;
  Xapian::TermIterator aEndIterator;

  switch (data->action) {
  case Termiterator_data::eTermlist:
    aStartIterator = that->mDb->termlist_begin(data->val);
    aEndIterator = that->mDb->termlist_end(data->val);
    break;
  case Termiterator_data::eAllterms:
    aStartIterator = that->mDb->allterms_begin();
    aEndIterator = that->mDb->allterms_end();
    break;
  case Termiterator_data::eAlltermsPrefix:
    aStartIterator = that->mDb->allterms_begin(data->str);
    aEndIterator = that->mDb->allterms_end(data->str);
    break;
  case Termiterator_data::eSpellings:
    aStartIterator = that->mDb->spellings_begin();
    aEndIterator = that->mDb->spellings_end();
    break;
  case Termiterator_data::eSynonyms:
    aStartIterator = that->mDb->synonyms_begin(data->str);
    aEndIterator = that->mDb->synonyms_end(data->str);
    break;
  case Termiterator_data::eSynonymKeys:
    aStartIterator = that->mDb->synonym_keys_begin(data->str);
    aEndIterator = that->mDb->synonym_keys_end(data->str);
    break;
  case Termiterator_data::eMetadataKeys:
    aStartIterator = that->mDb->metadata_keys_begin(data->str);
    aEndIterator = that->mDb->metadata_keys_end(data->str);
    break;
  default: assert(0);
  }

  Xapian::termcount aSize=0;
  for (Xapian::TermIterator aIt = aStartIterator; aIt != aEndIterator; aIt++)
    aSize++;

  if (aSize < data->first ) {
    data->size = 0;
    return;
  }

  aSize -= data->first;
  if (data->maxitems != 0 && data->maxitems < aSize)
    aSize = data->maxitems;
  data->tlist = new Termiterator_data::Item[aSize];


  Xapian::TermIterator aIt = aStartIterator;
  for (Xapian::termcount i = 0; i < data->first; ++i)  ++aIt;

  for (data->size = 0; aIt != aEndIterator && data->size < aSize; ++data->size, ++aIt) {
    data->tlist[data->size].tname = *aIt;
    data->tlist[data->size].wdf = aIt.get_wdf();
    data->tlist[data->size].termfreq = aIt.get_termfreq();
    data->tlist[data->size].description = aIt.get_description();
  }
}

Handle<Value> Database::Termiterator_convert(void* pData) {
  Termiterator_data* data = (Termiterator_data*) pData;

  Local<Array> aList(Array::New(data->size));
  for (Xapian::termcount a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    aO->Set(String::NewSymbol("tname"      ), String::New(data->tlist[a].tname.c_str()      ));
    aO->Set(String::NewSymbol("wdf"        ), Uint32::New(data->tlist[a].wdf                ));
    aO->Set(String::NewSymbol("termfreq"   ), Uint32::New(data->tlist[a].termfreq           ));
    aO->Set(String::NewSymbol("description"), String::New(data->tlist[a].description.c_str()));
    aList->Set(a, aO);
  }
  delete data;
  return aList;
}

static int kTermlist[] = { eUint32, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::Termlist(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kTermlist, args, aOpt))
    return throwSignatureErr(kTermlist);

  Termiterator_data* aData = new Termiterator_data(Termiterator_data::eTermlist, args[0]->Uint32Value(), aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kAllterms[] = { -eString, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::Allterms(const Arguments& args) {
  HandleScope scope;
  int aOpt[4];
  if (!checkArguments(kAllterms, args, aOpt))
    return throwSignatureErr(kTermlist);

  Termiterator_data* aData;

  if (aOpt[0] >= 0)
    aData = new Termiterator_data(Termiterator_data::eAlltermsPrefix, *String::Utf8Value(args[0]), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value(), aOpt[2] < 0 ? 0 : args[aOpt[2]]->Uint32Value()); //deleted by Termiterator_convert on non error
  else
    aData = new Termiterator_data(Termiterator_data::eAllterms, aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value(), aOpt[2] < 0 ? 0 : args[aOpt[2]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[3] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSpellings[] = { -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::Spellings(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kSpellings, args, aOpt))
    return throwSignatureErr(kSpellings);

  Termiterator_data* aData = new Termiterator_data(Termiterator_data::eSpellings, aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSynonyms[] = { eString, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::Synonyms(const Arguments& args) {
  HandleScope scope;
  int aOpt[3];
  if (!checkArguments(kSynonyms, args, aOpt))
    return throwSignatureErr(kSynonyms);

  Termiterator_data* aData = new Termiterator_data(Termiterator_data::eSynonyms, *String::Utf8Value(args[0]), aOpt[0] < 0 ? 0 : args[aOpt[0]]->Uint32Value(), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[2] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kSynonymKeys[] = { -eString, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::SynonymKeys(const Arguments& args) {
  HandleScope scope;
  int aOpt[4];
  if (!checkArguments(kSynonymKeys, args, aOpt))
    return throwSignatureErr(kSynonymKeys);

  Termiterator_data* aData = new Termiterator_data(Termiterator_data::eSynonymKeys, aOpt[0] < 0 ? "" : *String::Utf8Value(args[aOpt[0]]), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value(), aOpt[2] < 0 ? 0 : args[aOpt[2]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[3] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

static int kMetadataKeys[] = { -eString, -eUint32, -eUint32, -eFunction, eEnd };
Handle<Value> Database::MetadataKeys(const Arguments& args) {
  HandleScope scope;
  int aOpt[4];
  if (!checkArguments(kMetadataKeys, args, aOpt))
    return throwSignatureErr(kMetadataKeys);

  Termiterator_data* aData = new Termiterator_data(Termiterator_data::eMetadataKeys, aOpt[0] < 0 ? "" : *String::Utf8Value(args[aOpt[0]]), aOpt[1] < 0 ? 0 : args[aOpt[1]]->Uint32Value(), aOpt[2] < 0 ? 0 : args[aOpt[2]]->Uint32Value()); //deleted by Termiterator_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Enquire>(aOpt[3] >= 0, args, (void*)aData, Termiterator_process, Termiterator_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}
