
#include "op.h"
#include "enquire.h"
#include "database.h"


Persistent<FunctionTemplate> Enquire::constructor_template;



void Enquire::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Enquire"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_query", SetQuery);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset", GetMset);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset_sync", GetMset);

  target->Set(String::NewSymbol("Enquire"), constructor_template->GetFunction());
}

Handle<Value> Enquire::New(const Arguments& args) {
  HandleScope scope;
  Database* aDb;
  if (args.Length() < 1 || !(aDb = GetInstance<Database>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Database)")));
  Enquire* that = new Enquire(aDb->getDb());
  that->Wrap(args.This());
  return args.This();
}

Handle<Value> Enquire::SetQuery(const Arguments& args) {
  HandleScope scope;
  Query* aQ;
  if (args.Length() < 1 || !(aQ = GetInstance<Query>(args[0])))
    return ThrowException(Exception::TypeError(String::New("arguments are (Query)")));
  Enquire* that = ObjectWrap::Unwrap<Enquire>(args.This());
  if (that->mBusy)
    return ThrowException(Exception::Error(kBusyMsg));
  try {
    that->mEnq.set_query(aQ->mQry);
  } catch (const Xapian::Error& err) {
    return ThrowException(Exception::Error(String::New(err.get_msg().c_str())));
  }
  return Undefined();
}


Handle<Value> Enquire::GetMset(const Arguments& args) {
  HandleScope scope;
  bool aIsSync;
  if (args.Length() == 3 && args[0]->IsUint32() && args[1]->IsUint32() && args[2]->IsFunction())
    aIsSync=false;
  else if (args.Length() == 2 && args[0]->IsUint32() && args[1]->IsUint32())
    aIsSync=true;
  else
    return ThrowException(Exception::TypeError(String::New("arguments are (number, number, function) or (number, number)")));
  GetMset_data *aData=NULL;

  aData = new GetMset_data(args[0]->Uint32Value(), args[1]->Uint32Value());

  Handle<Value> aResult;
  try {
    aResult = do_all(aIsSync, args, (void*&)aData, Enquire::GetMset_process, Enquire::GetMset_convert);
  } catch (Local<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}


Xapian::Error* Enquire::GetMset_process(void *pData, void *pThat) {
  GetMset_data *data = (GetMset_data *) pData;
  Enquire *that = (Enquire *) pThat;
  try {
    Xapian::MSet aSet = that->mEnq.get_mset(data->first, data->maxitems);
    data->set = new GetMset_data::Mset_item[aSet.size()];
    data->size = 0;
    for (Xapian::MSetIterator a = aSet.begin(); a != aSet.end(); ++a, ++data->size) {
      data->set[data->size].id = *a;
      data->set[data->size].document = new Xapian::Document(a.get_document());
      data->set[data->size].rank = a.get_rank();
      data->set[data->size].collapse_count = a.get_collapse_count();
      data->set[data->size].weight = a.get_weight();
      data->set[data->size].collapse_key = a.get_collapse_key();
      data->set[data->size].description = a.get_description();
      data->set[data->size].percent = a.get_percent();
    }
  } catch (const Xapian::Error& err) {
    return new Xapian::Error(err);
  }
  return NULL;
}

Handle<Value> Enquire::GetMset_convert(void *&pData) {
  GetMset_data *data = (GetMset_data *) pData;
  Local<Array> aList(Array::New(data->size));
  Local<Function> aCtor(Document::constructor_template->GetFunction());
  for (int a = 0; a < data->size; ++a) {
    Local<Object> aO(Object::New());
    Local<Value> aDoc[] = { External::New(data->set[a].document) };
    aO->Set(String::NewSymbol("document"      ), aCtor->NewInstance(1, aDoc));
    aO->Set(String::NewSymbol("id"            ), Uint32::New(data->set[a].id                  ));
    aO->Set(String::NewSymbol("rank"          ), Uint32::New(data->set[a].rank                ));
    aO->Set(String::NewSymbol("collapse_count"), Uint32::New(data->set[a].collapse_count      ));
    aO->Set(String::NewSymbol("weight"        ), Number::New(data->set[a].weight              ));
    aO->Set(String::NewSymbol("collapse_key"  ), String::New(data->set[a].collapse_key.c_str()));
    aO->Set(String::NewSymbol("description"   ), String::New(data->set[a].description.c_str() ));
    aO->Set(String::NewSymbol("percent"       ),  Int32::New(data->set[a].percent             ));
    aList->Set(a, aO);
  }
  if (data) delete data;
  return aList;
}
