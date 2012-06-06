
#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

#include "node-xapian.h"

using namespace v8;
using namespace node;

Persistent<String> kBusyMsg;

void tryCallCatch(Handle<Function> fn, Handle<Object> context, int argc, Handle<Value>* argv) {
  TryCatch try_catch;

  fn->Call(context, argc, argv);

  if (try_catch.HasCaught())
    FatalException(try_catch);
}

void sendToThreadPool(void* execute, void* done, void* data){
  eio_custom((eio_cb) execute, EIO_PRI_DEFAULT, (eio_cb) done, data);
}

class Mime2Text : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Mime2Text m2T;

protected:
  Mime2Text() : ObjectWrap(), m2T(), mBusy(false) {}

  ~Mime2Text() { }

  bool mBusy;

  friend struct AsyncOp<Mime2Text>;
  friend struct Main_data;

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> Convert(const Arguments& args);
  static int Convert_pool(eio_req* req);
  static int Convert_done(eio_req* req);
  struct Convert_data : AsyncOp<Mime2Text> {
    Convert_data(Handle<Object> ob, Handle<Function> cb, Handle<String> fi, Handle<Value> ty)
      : AsyncOp<Mime2Text>(ob, cb), filename(fi), type(ty->IsString() ? ty : Handle<Value>()) {}
    String::Utf8Value filename;
    String::Utf8Value type;
    Xapian::Mime2Text::Fields fields;
  };
};

static Handle<Value> AssembleDocument(const Arguments& args);
static int Main_pool(eio_req* req);
static int Main_done(eio_req* req);
struct Main_data : public AsyncOpBase {
  Main_data(Handle<Function> cb, Xapian::Document* doc, TermGenerator* tg, String::Utf8Value** tl, Mime2Text* m2t, Handle<Value> p, Handle<Value> m)
    : AsyncOpBase(cb), document(doc), termgen(tg), textlist(tl), mime2text(m2t), path(p), mimetype(m) {
    termgen->Ref();
    mime2text->Ref();
  }
  ~Main_data() {
    if (textlist) {
      for (int a=0; textlist[a]; ++a)
        delete textlist[a];
      delete [] textlist;
    }
    termgen->Unref();
    mime2text->Unref();
  }
  Xapian::Document* document;
  TermGenerator* termgen;
  String::Utf8Value** textlist;
  Mime2Text* mime2text;
  String::Utf8Value path;
  String::Utf8Value mimetype;
  Xapian::Mime2Text::Fields fields;
};

extern "C"
void init (Handle<Object> target) {
  HandleScope scope;
  kBusyMsg = Persistent<String>::New(String::New("object busy with async op"));
  Database::Init(target);
  WritableDatabase::Init(target);
  TermGenerator::Init(target);
  Stem::Init(target);
  Enquire::Init(target);
  Query::Init(target);
  Document::Init(target);
  Mime2Text::Init(target);
  target->Set(String::NewSymbol("assemble_document"), FunctionTemplate::New(AssembleDocument)->GetFunction());
}


Persistent<FunctionTemplate> Database::constructor_template;

void Database::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "reopen", Reopen);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_database", AddDatabase);

  target->Set(String::NewSymbol("Database"), constructor_template->GetFunction());
}

Handle<Value> Database::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 || !args[0]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are (string)")));

  Database* that = new Database();
  that->Wrap(args.This());

  eio_custom(Open_pool, EIO_PRI_DEFAULT, Open_done, new Open_data(args.This(), args[0]->ToString()));

  return args.This();
}

Handle<Value> Database::AddDatabase(const Arguments& args) {
  HandleScope scope;
  Database* aDb;
  if (args.Length() < 1 || !(aDb = GetInstance<Database>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Database)")));
  Database* that = ObjectWrap::Unwrap<Database>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));
  try {
  that->mDb->add_database(*aDb->mDb);
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  return Undefined();
}

Handle<Value> Database::Reopen(const Arguments& args) {
  HandleScope scope;

  Open_data* aData;
  try {
    aData = new Open_data(args.This(), Handle<String>());
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(Open_pool, EIO_PRI_DEFAULT, Open_done, aData);

  return Undefined();
}

int Database::Open_pool(eio_req* req) {
  Open_data* aData = (Open_data*) req->data;

  try {
  if (aData->object->mDb)
    aData->object->mDb->reopen();
  else
    aData->object->mDb = aData->writeopts ? new Xapian::WritableDatabase(*aData->filename, aData->writeopts) : new Xapian::Database(*aData->filename);
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  aData->poolDone();
  return 0;
}

int Database::Open_done(eio_req* req) {
  HandleScope scope;

  Open_data* aData = (Open_data*) req->data;

  Handle<Value> argv[1];
  if (aData->error)
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));

  aData->object->Emit(String::New("open"), aData->error ? 1 : 0, argv);

  delete aData;

  return 0;
}

Persistent<FunctionTemplate> WritableDatabase::constructor_template;

void WritableDatabase::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->Inherit(Database::constructor_template);
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

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsInt32())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, number)")));

  WritableDatabase* that = new WritableDatabase();
  that->Wrap(args.This());

  eio_custom(Open_pool, EIO_PRI_DEFAULT, Open_done, new Open_data(args.This(), args[0]->ToString(), args[1]->Int32Value()));

  return args.This();
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



Persistent<FunctionTemplate> Query::constructor_template;

void Query::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Query"));

  //NODE_SET_PROTOTYPE_METHOD(constructor_template, "fn", Fn);

  target->Set(String::NewSymbol("Query"), constructor_template->GetFunction());

  Handle<Object> aO = constructor_template->GetFunction();
  aO->Set(String::NewSymbol("OP_AND"         ), Integer::New(Xapian::Query::OP_AND         ), ReadOnly);
  aO->Set(String::NewSymbol("OP_OR"          ), Integer::New(Xapian::Query::OP_OR          ), ReadOnly);
  aO->Set(String::NewSymbol("OP_AND_NOT"     ), Integer::New(Xapian::Query::OP_AND_NOT     ), ReadOnly);
  aO->Set(String::NewSymbol("OP_XOR"         ), Integer::New(Xapian::Query::OP_XOR         ), ReadOnly);
  aO->Set(String::NewSymbol("OP_AND_MAYBE"   ), Integer::New(Xapian::Query::OP_AND_MAYBE   ), ReadOnly);
  aO->Set(String::NewSymbol("OP_FILTER"      ), Integer::New(Xapian::Query::OP_FILTER      ), ReadOnly);
  aO->Set(String::NewSymbol("OP_NEAR"        ), Integer::New(Xapian::Query::OP_NEAR        ), ReadOnly);
  aO->Set(String::NewSymbol("OP_PHRASE"      ), Integer::New(Xapian::Query::OP_PHRASE      ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_RANGE" ), Integer::New(Xapian::Query::OP_VALUE_RANGE ), ReadOnly);
  aO->Set(String::NewSymbol("OP_SCALE_WEIGHT"), Integer::New(Xapian::Query::OP_SCALE_WEIGHT), ReadOnly);
  aO->Set(String::NewSymbol("OP_ELITE_SET"   ), Integer::New(Xapian::Query::OP_ELITE_SET   ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_GE"    ), Integer::New(Xapian::Query::OP_VALUE_GE    ), ReadOnly);
  aO->Set(String::NewSymbol("OP_VALUE_LE"    ), Integer::New(Xapian::Query::OP_VALUE_LE    ), ReadOnly);
  aO->Set(String::NewSymbol("OP_SYNONYM"     ), Integer::New(Xapian::Query::OP_SYNONYM     ), ReadOnly);
}

Handle<Value> Query::New(const Arguments& args) {
  HandleScope scope;
  int aN;
  std::vector<std::string> aList;
  for (aN = 1; aN < args.Length() && args[aN]->IsString(); ++aN)
    aList.push_back(*String::Utf8Value(args[aN]));
  if (args.Length() < 2 || !args[0]->IsInt32() || aN < args.Length())
    return ThrowException(Exception::TypeError(String::New("arguments are (Query.op, string ...)")));
  Query* that;
  const char* aDesc;
  try {
  that = new Query((Xapian::Query::op)args[0]->Int32Value(), aList.begin(), aList.end());
  aDesc = that->mQry.get_description().c_str();
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  args.This()->Set(String::NewSymbol("description"), String::New(aDesc));
  that->Wrap(args.This());
  return args.This();
}

Persistent<FunctionTemplate> Document::constructor_template;

void Document::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Document"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_data", GetData);

  target->Set(String::NewSymbol("Document"), constructor_template->GetFunction());
}

Handle<Value> Document::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() && !args[0]->IsExternal())
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));

  Document* that = new Document(args.Length() ? (Xapian::Document*) External::Unwrap(args[0]) : new Xapian::Document);
  that->Wrap(args.This());

  return args.This();
}

Handle<Value> Document::GetData(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 || !args[0]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (function)")));
  GetData_data* aData;
  try {
    aData = new GetData_data(args.This(), Local<Function>::Cast(args[0]));
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(GetData_pool, EIO_PRI_DEFAULT, GetData_done, aData);

  return Undefined();
}

int Document::GetData_pool(eio_req* req) {
  GetData_data* aData = (GetData_data*) req->data;

  try {
  aData->data = aData->object->mDoc->get_data();
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  aData->poolDone();
  return 0;
}

int Document::GetData_done(eio_req* req) {
  HandleScope scope;

  GetData_data* aData = (GetData_data*) req->data;

  Handle<Value> argv[2];
  if (aData->error) {
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));
  } else {
    argv[0] = Null();
    argv[1] = String::New(aData->data.c_str());
  }

  tryCallCatch(aData->callback, aData->object->handle_, aData->error ? 1 : 2, argv);

  delete aData;

  return 0;
}

Persistent<FunctionTemplate> Mime2Text::constructor_template;

void Mime2Text::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Mime2Text"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "convert", Convert);

  target->Set(String::NewSymbol("Mime2Text"), constructor_template->GetFunction());
}

Handle<Value> Mime2Text::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length())
    return ThrowException(Exception::TypeError(String::New("arguments are ()")));

  Mime2Text* that = new Mime2Text();
  that->Wrap(args.This());

  return args.This();
}

Handle<Value> Mime2Text::Convert(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3 || !args[0]->IsString() || (!args[1]->IsString() && !args[1]->IsNull()) || !args[2]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, string|null, function)")));
  Convert_data* aData;
  try {
    aData = new Convert_data(args.This(), Local<Function>::Cast(args[2]), args[0]->ToString(), args[1]);
    aData->poolDone(); // concurrent access ok
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }

  eio_custom(Convert_pool, EIO_PRI_DEFAULT, Convert_done, aData);

  return Undefined();
}

int Mime2Text::Convert_pool(eio_req* req) {
  Convert_data* aData = (Convert_data*) req->data;

  try {
  int aStatus = aData->object->m2T.convert(*aData->filename, aData->type.length() ? *aData->type : NULL, &aData->fields);
  if (aStatus != Xapian::Mime2Text::Status_OK) {
    std::string aMsg("Mime2Text::convert error: ");
    aMsg += (char) (aStatus + '0');
    throw Xapian::InternalError(aMsg);
  }
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  return 0;
}

int Mime2Text::Convert_done(eio_req* req) {
  HandleScope scope;

  Convert_data* aData = (Convert_data*) req->data;

  Handle<Value> argv[2];
  if (aData->error) {
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));
  } else {
    argv[0] = Null();
    Local<Object> aO(Object::New());
    aO->Set(String::NewSymbol("title"   ), String::New(aData->fields.title.c_str()));
    aO->Set(String::NewSymbol("author"  ), String::New(aData->fields.author.c_str()));
    aO->Set(String::NewSymbol("keywords"), String::New(aData->fields.keywords.c_str()));
    aO->Set(String::NewSymbol("sample"  ), String::New(aData->fields.sample.c_str()));
    aO->Set(String::NewSymbol("body"    ), String::New(aData->fields.dump.c_str()));
    aO->Set(String::NewSymbol("md5"     ), String::New(aData->fields.md5.c_str()));
    aO->Set(String::NewSymbol("mimetype"), String::New(aData->fields.mimetype.c_str()));
    aO->Set(String::NewSymbol("command" ), String::New(aData->fields.command.c_str()));
    argv[1] = aO;
  }

  tryCallCatch(aData->callback, aData->object->handle_, aData->error ? 1 : 2, argv);

  delete aData;

  return 0;
}


/*
document input object: {
  // all members optional; at least one required
  id_term: string, // boolean term; if found in index, replace/delete that document
  data: string, // pass to Document::set_data()
  text: [ string/buffer, ... ], // pass to TermGenerator::index_text()
  file: { path: string, mime_t: string, ... }, // invoke format converter library, then index_text()
  terms: { term: wdfinc, ... }, // pass to Document::add_term()
  values: { slot: value, ... } // pass to Document::add_value()
}
*/

static Handle<Value> AssembleDocument(const Arguments& args) {
  HandleScope scope;

  TermGenerator* aTg;
  Mime2Text* aM2t;
  if (args.Length() < 4 || !(aTg = GetInstance<TermGenerator>(args[0])) || !(aM2t = GetInstance<Mime2Text>(args[1]))
   || !args[2]->IsObject() || !args[3]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (TermGenerator, Mime2Text, object, function)")));

  Local<Object> aO = args[2]->ToObject();
  Local<String> aKey;
  Local<Value> aVal, aPath, aMime;
  String::Utf8Value** aTextList = NULL;
  Xapian::Document aDoc;
  try {
    if (aO->Has(aKey = String::New("id_term"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsString())
        return ThrowException(Exception::TypeError(String::New("input object id_term not a string")));
      aDoc.add_boolean_term(*String::Utf8Value(aVal));
    }
    if (aO->Has(aKey = String::New("data"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsString())
        return ThrowException(Exception::TypeError(String::New("input object data not a string")));
      aDoc.set_data(*String::Utf8Value(aVal));
    }
    if (aO->Has(aKey = String::New("file"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsObject())
        return ThrowException(Exception::TypeError(String::New("input object file not an object")));
      Local<Object> aFile = aVal->ToObject();
      aPath = aFile->Get(String::New("path"));
      if (!aPath->IsString())
        return ThrowException(Exception::TypeError(String::New("input object file.path not a string")));
      if (aFile->Has(aKey = String::New("mime_t"))) {
        aMime = aFile->Get(aKey);
        if (!aMime->IsString())
          return ThrowException(Exception::TypeError(String::New("input object file.mime_t not a string")));
      }
    }
    if (aO->Has(aKey = String::New("terms"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsObject())
        return ThrowException(Exception::TypeError(String::New("input object terms not an object")));
      Local<Object> aTerms = aVal->ToObject();
      Local<Array> aNames = aTerms->GetPropertyNames();
      for (uint32_t a = 0; a < aNames->Length(); ++a) {
        aVal = aTerms->Get(aKey = aNames->Get(a)->ToString());
        if (aVal->IsUint32())
          aDoc.add_term(*String::Utf8Value(aKey), aVal->Uint32Value());
      }
    }
    if (aO->Has(aKey = String::New("values"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsObject())
        return ThrowException(Exception::TypeError(String::New("input object values not an object")));
      Local<Object> aValues = aVal->ToObject();
      Local<Array> aNames = aValues->GetPropertyNames();
      for (uint32_t a = 0; a < aNames->Length(); ++a) {
        aVal = aNames->Get(a);
        if (aVal->IsUint32()) {
          uint32_t aSlot = aVal->Uint32Value();
          aVal = aValues->Get(aSlot);
          if (aVal->IsString())
            aDoc.add_value(aSlot, *String::Utf8Value(aVal));
        }
      }
    }
    if (aO->Has(aKey = String::New("text"))) {
      aVal = aO->Get(aKey);
      if (!aVal->IsArray())
        return ThrowException(Exception::TypeError(String::New("input object text not an array")));
      Local<Array> aAry = Local<Array>::Cast(aVal);
      aTextList = new String::Utf8Value*[aAry->Length()+1];
      for (uint32_t a = 0; a < aAry->Length(); ++a)
        aTextList[a] = new String::Utf8Value(aAry->Get(a));
      aTextList[aAry->Length()] = NULL;
    }
    if (aVal.IsEmpty())
      return ThrowException(Exception::TypeError(String::New("input object has no relevant members")));
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }

  Main_data* aData = new Main_data(Local<Function>::Cast(args[3]), new Xapian::Document(aDoc), aTg, aTextList, aM2t, aPath, aMime);

  eio_custom(Main_pool, EIO_PRI_DEFAULT, Main_done, aData);

  return Undefined();
}

static int Main_pool(eio_req* req) {
  Main_data* aData = (Main_data*) req->data;

  try {
  aData->termgen->mTg.set_document(*aData->document);
  for (int a = 0; aData->textlist && aData->textlist[a]; ++a) {
    aData->termgen->mTg.index_text(Xapian::Utf8Iterator(**aData->textlist[a], aData->textlist[a]->length()));
    aData->termgen->mTg.increase_termpos();
  }
  if (aData->path.length()) {
    int aStatus = aData->mime2text->m2T.convert(*aData->path, aData->mimetype.length() ? *aData->mimetype : NULL, &aData->fields);
    if (aStatus != Xapian::Mime2Text::Status_OK) {
      std::string aMsg("Mime2Text::convert error: ");
      aMsg += (char) (aStatus + '0');
      throw Xapian::InternalError(aMsg);
    }
    if (!aData->fields.title.empty()) {
      aData->termgen->mTg.index_text(aData->fields.title);
      aData->termgen->mTg.increase_termpos();
    }
    if (!aData->fields.author.empty()) {
      aData->termgen->mTg.index_text(aData->fields.author);
      aData->termgen->mTg.increase_termpos();
    }
    if (!aData->fields.keywords.empty()) {
      aData->termgen->mTg.index_text(aData->fields.keywords);
      aData->termgen->mTg.increase_termpos();
    }
    if (!aData->fields.dump.empty()) {
      aData->termgen->mTg.index_text(aData->fields.dump);
      aData->termgen->mTg.increase_termpos();
    }
  }
  } catch (const Xapian::Error& err) {
    aData->error = new Xapian::Error(err);
  }

  return 0;
}

static int Main_done(eio_req* req) {
  HandleScope scope;

  Main_data* aData = (Main_data*) req->data;

  Handle<Value> argv[2];
  if (aData->error) {
    argv[0] = Exception::Error(String::New(aData->error->get_msg().c_str()));
    delete aData->document;
  } else {
    argv[0] = Null();
    Local<Value> aDoc[] = { External::New(aData->document) };
    argv[1] = Document::constructor_template->GetFunction()->NewInstance(1, aDoc);
  }

  tryCallCatch(aData->callback, Context::GetCurrent()->Global(), aData->error ? 1 : 2, argv);

  delete aData;

  return 0;
}


