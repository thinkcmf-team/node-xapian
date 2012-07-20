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

enum ArgumentType {
  eEnd,
  eInt32,
  eUint32,
  eBoolean,
  eString,
  eObject,
  eArray,
  eObjectDatabase,
  eObjectDocument,
  eNull,
  eFunction
};

bool checkArguments(int signature[], const Arguments& args, int optionals[]);
Handle<Value> throwSignatureErr(int signature[]);
Handle<Value> throwSignatureErr(int *signatures[], int sigN);

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
    struct Item {
      Xapian::docid id;
      Xapian::Document* document;
      Xapian::doccount rank, collapse_count;
      Xapian::weight weight;
      std::string collapse_key, description;
      Xapian::percent percent;
    };
    Item* set;
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
    enum { eNewDatabase, eNewWDatabase, eClose, eReopen, eKeepAlive, eAddDatabase };
    Open_data(int act, Database* th, Handle<String> fn, int wop=0): action(act), that(th), db(NULL), filename(fn), writeopts(wop) {}
    Open_data(int act): action(act), that(NULL), db(NULL), filename(Handle<String>()), writeopts(0) {}
    Open_data(int act, Local<Object> th, Database* pdb): action(act), that(ObjectWrap::Unwrap<Database>(th)), db(pdb), filename(Handle<String>()), writeopts(0) { db->Ref(); }
    ~Open_data() { if (db) db->Unref(); }
    int action;
    Database* that;
    Database*  db;
    String::Utf8Value filename;
    int writeopts;
  };
  static void Open_process(void* data, void* that);
  static Handle<Value> Open_convert(void* data);

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> Close(const Arguments& args);
  static Handle<Value> Reopen(const Arguments& args);
  static Handle<Value> KeepAlive(const Arguments& args);
  static Handle<Value> AddDatabase(const Arguments& args);

  struct Generic_data {
    enum { 
      eGetDescription, eHasPositions, eGetDoccount, eGetLastdocid, eGetAvlength, eGetTermfreq, eTermExists, eGetCollectionFreq,
      eGetValueFreq, eGetValueLowerBound, eGetValueUpperBound, eGetDoclengthLowerBound, eGetDoclengthUpperBound, eGetWdfUpperBound,
      eGetDoclength, eGetSpellingSuggestion, eGetMetadata, eGetUuid
    };
    Generic_data(int a) : action(a) {}
    Generic_data(int a, const std::string &s) : action(a), str1(s) {}
    Generic_data(int a, uint32_t v) : action(a), val1(v) {}
    Generic_data(int a, const std::string &s, uint32_t v) : action(a), str1(s), val1(v) {}
    int action;
    std::string str1, str2;
    uint32_t val1, val2;
    double vald1;
  };
  static void Generic_process(void* data, void* that);
  static Handle<Value> Generic_convert(void* data);

  static Handle<Value> GetDescription(const Arguments& args);
  static Handle<Value> HasPositions(const Arguments& args);
  static Handle<Value> GetDoccount(const Arguments& args);
  static Handle<Value> GetLastdocid(const Arguments& args);
  static Handle<Value> GetAvlength(const Arguments& args);
  static Handle<Value> GetTermfreq(const Arguments& args);
  static Handle<Value> TermExists(const Arguments& args);
  static Handle<Value> GetCollectionFreq(const Arguments& args);
  static Handle<Value> GetValueFreq(const Arguments& args);
  static Handle<Value> GetValueLowerBound(const Arguments& args);
  static Handle<Value> GetValueUpperBound(const Arguments& args);
  static Handle<Value> GetDoclengthLowerBound(const Arguments& args);
  static Handle<Value> GetDoclengthUpperBound(const Arguments& args);
  static Handle<Value> GetWdfUpperBound(const Arguments& args);
  static Handle<Value> GetDoclength(const Arguments& args);
  static Handle<Value> GetSpellingSuggestion(const Arguments& args);
  static Handle<Value> GetMetadata(const Arguments& args);
  static Handle<Value> GetUuid(const Arguments& args);


  struct GetDocument_data {
    GetDocument_data(Xapian::docid did) : docid(did) {}
    Xapian::docid docid;
    Xapian::Document* doc;
  };
  static void GetDocument_process(void* data, void* that);
  static Handle<Value> GetDocument_convert(void* data);

  static Handle<Value> GetDocument(const Arguments& args);


  struct Termiterator_data {
    Termiterator_data(int act, uint32_t fi, uint32_t mx): first(fi), maxitems(mx), tlist(NULL), action(act) {}
    Termiterator_data(int act, uint32_t did, uint32_t fi, uint32_t mx): first(fi), maxitems(mx), tlist(NULL), val(did), action(act) {}
    ~Termiterator_data() { if (tlist) delete [] tlist; }
    enum { 
      eTermlist
    };
    Xapian::termcount first, maxitems;
    struct Item {
      std::string tname, description;
      Xapian::termcount wdf;
      Xapian::doccount termfreq;
    };
    Item* tlist;
    Xapian::termcount size;
    uint32_t val;
    int action;
  };
  static void Termiterator_process(void* data, void* that);
  static Handle<Value> Termiterator_convert(void* data);

  static Handle<Value> Termlist(const Arguments& args);
};


class WritableDatabase : public Database {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::WritableDatabase& getWdb() { return *mWdb; }

protected:
  WritableDatabase() : Database() { }

  ~WritableDatabase() { }

  static Handle<Value> New(const Arguments& args);


  struct ReplaceDocument_data {
    enum { eAdd, eReplaceTerm, eReplaceDocId };
    ReplaceDocument_data(const Xapian::Document& doc) : document(doc), idterm(Handle<String>()), action(eAdd) {}
    ReplaceDocument_data(const Xapian::Document& doc, Handle<String> term) : document(doc), idterm(term), action(eReplaceTerm) {}
    ReplaceDocument_data(const Xapian::Document& doc, Xapian::docid id) : document(doc), docid(id), idterm(Handle<String>()), action(eReplaceDocId) {}
    const Xapian::Document& document;
    Xapian::docid docid;
    String::Utf8Value idterm;
    int action;
  };
  static void ReplaceDocument_process(void* data, void* that);
  static Handle<Value> ReplaceDocument_convert(void* data);

  static Handle<Value> ReplaceDocument(const Arguments& args);
  static Handle<Value> AddDocument(const Arguments& args);


  struct Generic_data {
    Generic_data(int a) : action(a) {}
    Generic_data(int a, uint32_t v) : action(a), val1(v) {}
    Generic_data(int a, const std::string &s) : action(a), str1(s) {}
    Generic_data(int a, const std::string &s, uint32_t v) : action(a), str1(s), val1(v) {}
    Generic_data(int a, const std::string &s1, const std::string &s2) : action(a), str1(s1), str2(s2) {}
    enum {
      eCommit, eBeginTx, eCommitTx, eCancelTx, eDeleteDocumentDid, eDeleteDocumentTerm, eAddSpelling, eRemoveSpelling,
      eAddSynonym, eRemoveSynonym, eClearSynonyms, eSetMetadata, eGetDescription
    };
    int action;
    std::string str1, str2;
    uint32_t val1;
  };
  static void Generic_process(void* data, void* that);
  static Handle<Value> Generic_convert(void* data);

  static Handle<Value> Commit(const Arguments& args);
  static Handle<Value> BeginTransaction(const Arguments& args);
  static Handle<Value> CommitTransaction(const Arguments& args);
  static Handle<Value> CancelTransaction(const Arguments& args);
  static Handle<Value> DeleteDocument(const Arguments& args);
  static Handle<Value> AddSpelling(const Arguments& args);
  static Handle<Value> RemoveSpelling(const Arguments& args);
  static Handle<Value> AddSynonym(const Arguments& args);
  static Handle<Value> RemoveSynonym(const Arguments& args);
  static Handle<Value> ClearSynonyms(const Arguments& args);
  static Handle<Value> SetMetadata(const Arguments& args);
  static Handle<Value> GetDescription(const Arguments& args);
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
  Query(const Xapian::Query& q) : ObjectWrap(), mQry(q) {}

  ~Query() {}

  static Handle<Value> New(const Arguments& args);

  //static Handle<Value> Fn(const Arguments& args);
};

class Document : public XapWrap<Document> {
public:
  static void Init(Handle<Object> target);

  static Persistent<FunctionTemplate> constructor_template;

  Xapian::Document* getDoc() {
    return mDoc;
  }

protected:
  Document(Xapian::Document* iDoc) : mDoc(iDoc) {}

  ~Document() {
    delete mDoc;
  }

  Xapian::Document* mDoc;

  static Handle<Value> New(const Arguments& args);


  struct Generic_data {
    enum { 
      eGetValue, eAddValue, eRemoveValue, eClearValues, eGetData, eSetData, eAddPosting, eAddTerm, eAddBooleanTerm,
      eRemovePosting, eRemoveTerm, eClearTerms, eTermlistCount, eValuesCount, eGetDocid, eSerialise, eGetDescription
    };
    Generic_data(int a) : action(a) {}
    Generic_data(int a, uint32_t v1) : action(a), val1(v1) {}
    Generic_data(int a, const std::string &s) : action(a), str(s) {}
    Generic_data(int a, const std::string &s, uint32_t v1) : action(a), str(s), val1(v1) {}
    Generic_data(int a, const std::string &s, uint32_t v1, uint32_t v2) : action(a), str(s), val1(v1), val2(v2) {}
    int action;
    std::string str;
    uint32_t val1, val2;
  };
  static void Generic_process(void* data, void* that);
  static Handle<Value> Generic_convert(void* data);

  static Handle<Value> GetValue(const Arguments& args);
  static Handle<Value> AddValue(const Arguments& args);
  static Handle<Value> RemoveValue(const Arguments& args);
  static Handle<Value> ClearValues(const Arguments& args);
  static Handle<Value> GetData(const Arguments& args);
  static Handle<Value> SetData(const Arguments& args);
  static Handle<Value> AddPosting(const Arguments& args);
  static Handle<Value> AddTerm(const Arguments& args);
  static Handle<Value> AddBooleanTerm(const Arguments& args);
  static Handle<Value> RemovePosting(const Arguments& args);
  static Handle<Value> RemoveTerm(const Arguments& args);
  static Handle<Value> ClearTerms(const Arguments& args);
  static Handle<Value> TermlistCount(const Arguments& args);
  static Handle<Value> ValuesCount(const Arguments& args);
  static Handle<Value> GetDocid(const Arguments& args);
  static Handle<Value> Serialise(const Arguments& args);
  static Handle<Value> GetDescription(const Arguments& args);


  struct Unserialise_data {
    Unserialise_data(const std::string &s) : str(s) {}
    std::string str;
    Xapian::Document* doc;
  };
  static void Unserialise_process(void* data, void* that);
  static Handle<Value> Unserialise_convert(void* data);

  static Handle<Value> Unserialise(const Arguments& args);


  struct Termlist_data {
    Termlist_data(uint32_t fi, uint32_t mx): first(fi), maxitems(mx), tlist(NULL) {}
    ~Termlist_data() { if (tlist) delete [] tlist; }
    Xapian::termcount first, maxitems;
    struct Item {
      std::string tname, description;
      Xapian::termcount wdf;
    };
    Item* tlist;
    Xapian::termcount size;
  };
  static void Termlist_process(void* data, void* that);
  static Handle<Value> Termlist_convert(void* data);

  static Handle<Value> Termlist(const Arguments& args);


  struct Values_data {
    Values_data(uint32_t fi, uint32_t mx): first(fi), maxitems(mx), vlist(NULL) {}
    ~Values_data() { if (vlist) delete [] vlist; }
    Xapian::termcount first, maxitems;
    struct Item {
      std::string value, description;
      Xapian::valueno valueno;
    };
    Item* vlist;
    Xapian::termcount size;
  };
  static void Values_process(void* data, void* that);
  static Handle<Value> Values_convert(void* data);

  static Handle<Value> Values(const Arguments& args);

};


#endif
