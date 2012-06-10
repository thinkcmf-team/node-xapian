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

typedef void (*FuncProcess) (void* data, void* that);
typedef Handle<Value> (*FuncConvert) (void* data);

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
    object->Ref();
  }
  virtual ~AsyncOp() { object->Unref(); }
  void poolDone() { object->mBusy = false; } //temporary untill all the methods using AsyncOp are refactored
  T* object;
  void* data;
  FuncProcess process;
  FuncConvert convert;
};

template<class T>
int async_pool(eio_req* req) {
  AsyncOp<T>* aAsOp = (AsyncOp<T>*)req->data;
  try {
    (*aAsOp->process)(aAsOp->data, aAsOp->object);
  } catch (const Xapian::Error& err) {
    aAsOp->error = new Xapian::Error(err);
  }
  aAsOp->object->mBusy = false;
  return 0;
}

template<class T>
int async_done(eio_req* req) {
  HandleScope scope;
  AsyncOp<T>* aAsOp = (AsyncOp<T>*)req->data;
  Handle<Value> aArgv[2];
  if (aAsOp->error) {
    aArgv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));
  } else {
    aArgv[0] = Null();
    aArgv[1] = (*aAsOp->convert)(aAsOp->data);
  }
  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, aArgv);
  delete aAsOp;
  return 0;
}

template<class T>
Handle<Value> invoke(bool async, const Arguments& args, void* data, FuncProcess process, FuncConvert convert) {
  T* that = ObjectWrap::Unwrap<T>(args.This());
  if (that->mBusy) 
    throw Exception::Error(kBusyMsg);
  if (async) {
    that->mBusy = true;
    AsyncOp<T>* aAsOp = new AsyncOp<T>(that, Local<Function>::Cast(args[args.Length()-1]), data, process, convert);
    int (*aPool)(eio_req*) = async_pool<T>;
    int (*aDone)(eio_req*) = async_done<T>;
    sendToThreadPool((void*)aPool, (void*)aDone, aAsOp);
    return Undefined();
  } else {
    try {
      process(data, that);
    } catch (const Xapian::Error& err) {
      throw Exception::Error(String::New(err.get_msg().c_str()));
    }
    return convert(data);
  }
}

template <class T>
class XapWrap : public ObjectWrap {
protected:
  XapWrap() : ObjectWrap(), mBusy(false) {}

  ~XapWrap() {}

  bool mBusy;

  friend struct AsyncOp<T>;
  friend Handle<Value> invoke<T>(bool async, const Arguments& args, void* data, FuncProcess process, FuncConvert convert);
  friend int async_pool<T>(eio_req* req);
};


class Enquire : public XapWrap<Enquire> {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

protected:
  Enquire(const Xapian::Database& iDb) : mEnq(iDb) {}

  ~Enquire() {
  }

  Xapian::Enquire mEnq;

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
  static void GetMset_process(void* data, void* that);
  static Handle<Value> GetMset_convert(void* data);
};

class Database : public XapWrap<Database> {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Database& getDb() { return *mDb; }

protected:
  Database() : mDb(NULL) {}

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

  struct Open_data {
    Open_data(Database* th, Handle<String> fn, int wop=0): that(th), filename(fn), writeopts(wop) {}
    Database* that;
    String::Utf8Value filename;
    int writeopts;
  };
  static void Open_process(void* data, void* that);
  static Handle<Value> Open_convert(void* data);

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> Reopen(const Arguments& args);


  static Handle<Value> AddDatabase(const Arguments& args);

};


class WritableDatabase : public Database {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::WritableDatabase& getWdb() { return *mWdb; }

protected:
  WritableDatabase() : Database() { }

  ~WritableDatabase() { }

  //used by AddDocument_data
  friend struct AsyncOp<WritableDatabase>;

  static Handle<Value> New(const Arguments& args);


  struct ReplaceDocument_data {
    ReplaceDocument_data(const Xapian::Document& doc): document(doc), idterm(Handle<String>()), action(ReplaceDocument_data::eAdd) {}
    ReplaceDocument_data(const Xapian::Document& doc, Handle<String> term): document(doc), idterm(term), action(ReplaceDocument_data::eRepleceTerm) {}
    ReplaceDocument_data(const Xapian::Document& doc, Xapian::docid id): document(doc), docid(id), idterm(Handle<String>()), action(ReplaceDocument_data::eReplaceDocId) {}
    enum { eAdd, eRepleceTerm, eReplaceDocId };
    const Xapian::Document& document;
    Xapian::docid docid;
    String::Utf8Value idterm;
    int action;
  };
  static void ReplaceDocument_process(void* data, void* that);
  static Handle<Value> ReplaceDocument_convert(void* data);

  static Handle<Value> ReplaceDocument(const Arguments& args);
  static Handle<Value> AddDocument(const Arguments& args);


  struct Commit_data {
    Commit_data(int op, bool fl=false) : type(op), flush(fl) {}
    enum { eCommit, eBeginTx, eCommitTx };
    int type;
    bool flush;
  };
  static void Commit_process(void* data, void* that);
  static Handle<Value> Commit_convert(void* data);

  static Handle<Value> Commit(const Arguments& args);
  static Handle<Value> BeginTransaction(const Arguments& args);
  static Handle<Value> CommitTransaction(const Arguments& args);
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
  static int GetData_pool(eio_req* req);
  static int GetData_done(eio_req* req);
  struct GetData_data : AsyncOp<Document> {
    GetData_data(Handle<Object> ob, Handle<Function> cb)
      : AsyncOp<Document>(ob, cb) {}
    std::string data;
  };
};


#endif
