
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
  bool isSync;
  if (args.Length() == 3 && args[0]->IsUint32() && args[1]->IsUint32() && args[2]->IsFunction())
    isSync=false;
  else if (args.Length() == 2 && args[0]->IsUint32() && args[1]->IsUint32())
    isSync=true;
  else
    return ThrowException(Exception::TypeError(String::New("arguments are (number, number, function) or (number, number)")));
  GetMset_data *aData=NULL;
  try {
    aData = new GetMset_data(args[0]->Uint32Value(), args[1]->Uint32Value());
  } catch (Local<Value> ex) {
    if (aData) delete aData;
    return ThrowException(ex);
  }
  return (isSync)?scope.Close(GetMset_do_sync(args,aData)):scope.Close(GetMset_do_async(args,aData));
}


Xapian::Error* Enquire::GetMset_process(GetMset_data *aData, Enquire *pThis)
{
  try {
  Xapian::MSet aSet = pThis->mEnq.get_mset(aData->first, aData->maxitems);
  aData->set = new GetMset_data::Mset_item[aSet.size()];
  aData->size = 0;
  for (Xapian::MSetIterator a = aSet.begin(); a != aSet.end(); ++a, ++aData->size) {
    aData->set[aData->size].id = *a;
    aData->set[aData->size].document = new Xapian::Document(a.get_document());
    aData->set[aData->size].rank = a.get_rank();
    aData->set[aData->size].collapse_count = a.get_collapse_count();
    aData->set[aData->size].weight = a.get_weight();
    aData->set[aData->size].collapse_key = a.get_collapse_key();
    aData->set[aData->size].description = a.get_description();
    aData->set[aData->size].percent = a.get_percent();
  }
  } catch (const Xapian::Error& err) {
    return new Xapian::Error(err);
  }
  return NULL;
}

Handle<Value> Enquire::GetMset_convert(GetMset_data *aData)
{
  HandleScope scope;
  Local<Array> aList(Array::New(aData->size));
  Local<Function> aCtor(Document::constructor_template->GetFunction());
  for (int a = 0; a < aData->size; ++a) {
    Local<Object> aO(Object::New());
    Local<Value> aDoc[] = { External::New(aData->set[a].document) };
    aO->Set(String::NewSymbol("document"      ), aCtor->NewInstance(1, aDoc));
    aO->Set(String::NewSymbol("id"            ), Uint32::New(aData->set[a].id                  ));
    aO->Set(String::NewSymbol("rank"          ), Uint32::New(aData->set[a].rank                ));
    aO->Set(String::NewSymbol("collapse_count"), Uint32::New(aData->set[a].collapse_count      ));
    aO->Set(String::NewSymbol("weight"        ), Number::New(aData->set[a].weight              ));
    aO->Set(String::NewSymbol("collapse_key"  ), String::New(aData->set[a].collapse_key.c_str()));
    aO->Set(String::NewSymbol("description"   ), String::New(aData->set[a].description.c_str() ));
    aO->Set(String::NewSymbol("percent"       ),  Int32::New(aData->set[a].percent             ));
    aList->Set(a, aO);
  }
  return scope.Close(aList);
}
