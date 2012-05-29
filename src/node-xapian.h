#ifndef _NODE_XAPIAN_H_
#define _NODe_XAPIAN_H_

#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

void tryCallCatch(Handle<Function> fn, Handle<Object> context, int argc, Handle<Value>* argv);

template <class T>
T* GetInstance(Handle<Value> val) {
  if (val->IsObject() && T::constructor_template->HasInstance(val->ToObject()))
    return ObjectWrap::Unwrap<T>(val->ToObject());
  return NULL;
}

void sendToThreadPool(void* execute, void* done, void* data);

extern Persistent<String> kBusyMsg;

struct AsyncOpBase {
  AsyncOpBase(Handle<Function> cb)
    : callback(), error(NULL) {
    callback = Persistent<Function>::New(cb);
    ev_ref(EV_DEFAULT_UC);
  }
  virtual ~AsyncOpBase() {
    if (error) delete error;
    ev_unref(EV_DEFAULT_UC);
    callback.Dispose();
  }
  Persistent<Function> callback;
  Xapian::Error* error;
};

typedef Xapian::Error* (*FuncProcess) (void *data, void *that);
typedef Handle<Value> (*FuncConvert) (void *data);

template <class T>
struct AsyncOp : public AsyncOpBase {
  AsyncOp(Handle<Object> ob, Handle<Function> cb) //temporary untill all the methods using AsyncOp are refactored
    : AsyncOpBase(cb), object(ObjectWrap::Unwrap<T>(ob)) {
    if (object->mBusy)
      throw Exception::Error(kBusyMsg);
    object->mBusy = true;
    object->Ref();
  }
  AsyncOp(T* ob, Handle<Function> cb, void* dt, FuncProcess pr, FuncConvert cv)
    : AsyncOpBase(cb), object(ob), data(dt), process(pr), convert(cv) {
    object->mBusy = true;
    object->Ref();
  }
  virtual ~AsyncOp() { object->Unref(); }
  void poolDone() { object->mBusy = false; }
  T* object;
  void* data;
  FuncProcess process;
  FuncConvert convert;
};

#define DECLARE_POOLS(classn) \
static int function_pool(eio_req *req) {\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  aAsOp->error=aAsOp->process(aAsOp->data, aAsOp->object);\
  aAsOp->poolDone();\
  return 0;\
}\
static int function_done(eio_req *req) {\
  HandleScope scope;\
  AsyncOp<classn> *aAsOp = (AsyncOp<classn>*)req->data;\
  Handle<Value> aArgv[2];\
  if (aAsOp->error) {\
    aArgv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));\
  } else {\
    aArgv[0] = Null();\
    aArgv[1] = aAsOp->convert(aAsOp->data);\
  }\
  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, aArgv);\
  delete aAsOp;\
  return 0;\
}\
static Handle<Value> do_all(bool async, const Arguments& args, void *data, FuncProcess process, FuncConvert convert) {\
  classn *that=ObjectWrap::Unwrap<classn>(args.This());\
  if (that->mBusy) \
    throw Exception::Error(kBusyMsg);\
  if (async) {\
    AsyncOp<classn> *aAsOp = new AsyncOp<classn>(that, Local<Function>::Cast(args[2]),data,process,convert);\
    sendToThreadPool((void*)function_pool, (void*)function_done, aAsOp);\
    return Undefined();\
  } else {\
    Xapian::Error* aError = process(data,that);\
    if (aError!=NULL) {\
      std::string aErrorStr=aError->get_msg();\
      delete aError;\
      throw Exception::Error(String::New(aErrorStr.c_str()));\
    }\
    return convert(data);\
  }\
}

class Enquire : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

protected:
  Enquire(const Xapian::Database& iDb) : ObjectWrap(), mEnq(iDb), mBusy(false) {}

  ~Enquire() {
  }

  Xapian::Enquire mEnq;
  bool mBusy;

  friend struct AsyncOp<Enquire>;
  DECLARE_POOLS(Enquire);

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> SetQuery(const Arguments& args);

  struct GetMset_data {
    GetMset_data(uint32_t fi, uint32_t mx): first(fi), maxitems(mx), set(NULL) {}
    ~GetMset_data() { if (set) delete [] set; }
    Xapian::doccount first, maxitems;
    struct Mset_item {
      Xapian::docid id;
      Xapian::Document* document;
      Xapian::doccount rank, collapse_count;
      Xapian::weight weight;
      std::string collapse_key, description;
      Xapian::percent percent;
    };
    Mset_item* set;
    int size;
  };
  static Handle<Value> GetMset(const Arguments& args);
  static Xapian::Error* GetMset_process(void *data, void *that);
  static Handle<Value> GetMset_convert(void *data);
};

class Database : public EventEmitter {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Database& getDb() { return *mDb; }

protected:
  Database() : EventEmitter(), mDb(NULL), mBusy(false) {}

  virtual ~Database() {
    if (mDb) {
      mDb->close();
      delete mDb;
    }
  }

  union {
    Xapian::Database* mDb;
    Xapian::WritableDatabase* mWdb;
  };
  bool mBusy;

  friend struct AsyncOp<Database>;

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> AddDatabase(const Arguments& args);

  static Handle<Value> Reopen(const Arguments& args);
  static int Open_pool(eio_req *req);
  static int Open_done(eio_req *req);
  struct Open_data : AsyncOp<Database> {
    Open_data(Handle<Object> ob, Handle<String> file, int wop=0)
      : AsyncOp<Database>(ob, Handle<Function>()), filename(file), writeopts(wop) {}
    String::Utf8Value filename;
    int writeopts;
  };
};


class WritableDatabase : public Database {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::WritableDatabase& getWdb() { return *mWdb; }

protected:
  WritableDatabase() : Database() { }

  ~WritableDatabase() { }

  friend struct AsyncOp<WritableDatabase>;

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> ReplaceDocument(const Arguments& args);
  static int AddDocument_pool(eio_req *req);
  static int AddDocument_done(eio_req *req);
  struct AddDocument_data : AsyncOp<WritableDatabase> {
    AddDocument_data(Handle<Object> ob, Handle<Function> cb, const Xapian::Document& doc, Handle<String> id)
      : AsyncOp<WritableDatabase>(ob, cb), document(doc), idterm(id) {}
    const Xapian::Document& document;
    Xapian::docid docid;
    String::Utf8Value idterm;
  };

  static Handle<Value> Commit(const Arguments& args);
  static Handle<Value> BeginTransaction(const Arguments& args);
  static Handle<Value> CommitTransaction(const Arguments& args);

  static int Commit_pool(eio_req *req);
  static int Commit_done(eio_req *req);
  struct Commit_data : AsyncOp<WritableDatabase> {
    Commit_data(Handle<Object> ob, Handle<Function> cb, int op, bool fl=false)
      : AsyncOp<WritableDatabase>(ob, cb), type(op), flush(fl) {}
    enum { eCommit, eBeginTx, eCommitTx };
    int type;
    bool flush;
  };
};

class TermGenerator : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::TermGenerator mTg;

protected:
  TermGenerator() : ObjectWrap(), mTg() { }

  ~TermGenerator() { }

  friend struct Main_data;

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> SetDatabase(const Arguments& args);
  static Handle<Value> SetFlags(const Arguments& args);
  static Handle<Value> SetStemmer(const Arguments& args);
};

class Stem : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Stem mStem;

protected:
  Stem(const char* iLang) : ObjectWrap(), mStem(iLang) { }

  ~Stem() { }

  static Handle<Value> New(const Arguments& args);
};


class Query : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Query mQry;

protected:
  template <class T>
  Query(Xapian::Query::op o, T a, T b) : ObjectWrap(), mQry(o, a, b) {}

  ~Query() {}

  static Handle<Value> New(const Arguments& args);

  //static Handle<Value> Fn(const Arguments& args);
};

class Document : public ObjectWrap {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Document* getDoc() {
    return mDoc;
  }

protected:
  Document(Xapian::Document* iDoc) : ObjectWrap(), mDoc(iDoc), mBusy(false) {}

  ~Document() {
    delete mDoc;
  }

  Xapian::Document* mDoc;
  bool mBusy;

  friend struct AsyncOp<Document>;

  static Handle<Value> New(const Arguments& args);


  static Handle<Value> GetData(const Arguments& args);
  static int GetData_pool(eio_req *req);
  static int GetData_done(eio_req *req);
  struct GetData_data : AsyncOp<Document> {
    GetData_data(Handle<Object> ob, Handle<Function> cb)
      : AsyncOp<Document>(ob, cb) {}
    std::string data;
  };
};


#endif
