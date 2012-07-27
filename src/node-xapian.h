#ifndef _NODE_XAPIAN_H_
#define _NODe_XAPIAN_H_

#include <stdlib.h>

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

struct GenericData {
  struct Item {
    Item() : isStr(false) {};
    Item(const char *a) : isStr(true) { string = new std::string(a); };
    Item(double a) : dbl(a), isStr(false) {};
    Item(uint32_t a) : uint32(a), isStr(false) {};
    Item(int32_t a) : int32(a), isStr(false) {};
    Item(bool a) : boolean(a), isStr(false) {};
    void setString(const std::string &str) { if (isStr) *string = str; else { string = new std::string(str); isStr = true; } };
    void unsetString() { if (isStr) { delete string; isStr = false; } };
    ~Item() { if (isStr) delete string; }
    union {
      double dbl;
      uint32_t uint32;
      int32_t int32;
      bool boolean;
      std::string* string;
    };
    bool isStr;
  };
  int action;
  Item* val;
  Item retVal;
  GenericData();
  GenericData(int act, const Arguments& args, int signature[], int optionals[], Item defaults[]) : action(act) {
    int aLength;
    for (aLength = 0; signature[aLength] != eEnd; ++aLength);
    val = new Item[aLength];
    for (int aOptN = 0, aSigN = 0; signature[aSigN] != eEnd; ++aSigN) {
      int aArgInd = signature[aSigN] < 0 && optionals[aOptN] >= 0 ? optionals[aOptN] : aSigN;
      if (signature[aSigN] < 0) {
        ++aOptN;
        if (optionals[aOptN-1] < 0) {
          if (abs(signature[aSigN]) != eFunction)
            val[aSigN] = defaults[aOptN-1];
          continue;
        }
      }
      switch (abs(signature[aSigN])) {
      case eInt32:    val[aSigN].int32 = args[aArgInd]->Int32Value();          break;
      case eUint32:   val[aSigN].uint32 = args[aArgInd]->Uint32Value();        break;
      case eBoolean:  val[aSigN].boolean = args[aArgInd]->BooleanValue();      break;
      case eString:   val[aSigN].setString(*String::Utf8Value(args[aArgInd])); break;
      case eFunction: break;
      default: assert(0);
      }
    }
  }
  ~GenericData() { if (val) delete[] val; }
};

template<class T>
Handle<Value> generic_start(int act, const Arguments& args, int signature[], GenericData::Item* defaults = NULL) {
  HandleScope scope;
  int aLength = 0;
  for (int a = 0; signature[a] != eEnd; ++a)
    if (signature[a]<0) aLength++;
  assert(aLength > 0);
  int* optionals = new int[aLength];
  if (!checkArguments(signature, args, optionals))
    return throwSignatureErr(signature);
  GenericData* aData = new GenericData(act, args, signature, optionals, defaults); //deleted by Generic_convert on non error
  Handle<Value> aResult = Undefined();
  try {
    aResult = invoke<T>(optionals[aLength-1] >= 0, args, (void*)aData, T::Generic_process, T::Generic_convert);
  } catch (Handle<Value> ex) {
    delete[] optionals;
    delete aData;
    return ThrowException(ex);
  }
  delete[] optionals;
  return scope.Close(aResult);
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

  static void Generic_process(void* data, void* that);
  static Handle<Value> Generic_convert(void* data);

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
    Open_data(int act, Database* th, Database* pdb): action(act), that(th), db(pdb), filename(Handle<String>()), writeopts(0) { db->Ref(); }
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
    Termiterator_data(int act, const std::string &s, uint32_t fi, uint32_t mx): first(fi), maxitems(mx), tlist(NULL), str(s), action(act) {}
    ~Termiterator_data() { if (tlist) delete [] tlist; }
    enum { 
      eTermlist, eAllterms, eAlltermsPrefix, eSpellings, eSynonyms, eSynonymKeys, eMetadataKeys
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
    std::string str;
    int action;
  };
  static void Termiterator_process(void* data, void* that);
  static Handle<Value> Termiterator_convert(void* data);

  static Handle<Value> Termlist(const Arguments& args);
  static Handle<Value> Allterms(const Arguments& args);
  static Handle<Value> Spellings(const Arguments& args);
  static Handle<Value> Synonyms(const Arguments& args);
  static Handle<Value> SynonymKeys(const Arguments& args);
  static Handle<Value> MetadataKeys(const Arguments& args);
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

  static void Generic_process(void* data, void* that);
  static Handle<Value> Generic_convert(void* data);

protected:
  Document(Xapian::Document* iDoc) : mDoc(iDoc) {}

  ~Document() {
    delete mDoc;
  }

  Xapian::Document* mDoc;

  static Handle<Value> New(const Arguments& args);

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
