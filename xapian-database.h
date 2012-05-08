#ifndef _XAPIAN_DATABASE_
#define _XAPIAN_DATABASE_

#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;


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


//TODO: move in other .h
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
