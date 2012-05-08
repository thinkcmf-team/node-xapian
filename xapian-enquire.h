#ifndef _XAPIAN_ENQUIRE_H_
#define _XAPIAN_ENQUIRE_H_

#include <xapian.h>
#include "mime2text.h"

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>

#include "xapian-op.h"

using namespace v8;
using namespace node;




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

  static Handle<Value> New(const Arguments& args);

  static Handle<Value> SetQuery(const Arguments& args);

  struct Mset_data {
    Mset_data(uint32_t fi, uint32_t mx): first(fi), maxitems(mx), set(NULL) {}
    ~Mset_data() { if (set) delete [] set; }
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
  static Handle<Value> GetMsetSync(const Arguments& args);
  static Xapian::Error* GetMset_process(Mset_data *data, Enquire *pThis);
  static Handle<Value> GetMset_convert(Mset_data *data);


  //TODO: replace with a macro
  static int GetMset_pool(eio_req *req);
  static int GetMset_done(eio_req *req);
};

#endif //_XAPIAN_ENQUIRE_H_
