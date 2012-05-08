
#include "xapian-enquire.h"
#include "xapian-database.h"
#include "xapian-op.h"

Persistent<FunctionTemplate> Enquire::constructor_template;



void Enquire::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Enquire"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "set_query", SetQuery);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset", GetMset);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "get_mset_sync", GetMsetSync);

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



Xapian::Error* Enquire::GetMset_process(Mset_data *aData, Enquire *pThis)
{

  try {
  Xapian::MSet aSet = pThis->mEnq.get_mset(aData->first, aData->maxitems);
  aData->set = new Mset_data::Mset_item[aSet.size()];
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

Handle<Value> Enquire::GetMset_convert(Mset_data *aData)
{
  HandleScope scope;

  Local<Array> aList(Array::New(aData->size));
  Local<Function> aCtor(Document::constructor_template->GetFunction());
  for (int a = 0; a < aData->size; ++a) {
    Local<Object> aO(Object::New());


    Local<Value> aDoc[] = { External::New(aData->set[a].document) };
    aO->Set(String::NewSymbol("document"      ), aCtor->NewInstance(1, aDoc));
//    Local<Object> docObj=aCtor->NewInstance(); -> it crashes even if i only call this and delete the previous 2 lines ( on sync)

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


//TODO: create templates to shrink even more
Handle<Value> Enquire::GetMset(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3 || !args[0]->IsUint32() || !args[1]->IsUint32() || !args[2]->IsFunction())
    return ThrowException(Exception::TypeError(String::New("arguments are (number, number, function)")));

  OpInfo *aInfo;
  try {
    Mset_data *aData;
    AsyncOp<Enquire> *aAsOp;
    aData = new Mset_data(args[0]->Uint32Value(), args[1]->Uint32Value());
    aAsOp = new AsyncOp<Enquire>(args.This(), Local<Function>::Cast(args[2]));
    aInfo=new OpInfo(aData,aAsOp);
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }
  sendToThreadPool((void*)GetMset_pool, (void*)GetMset_done, aInfo);

  return Undefined();
}


//TODO: create templates to shrink even more
Handle<Value> Enquire::GetMsetSync(const Arguments& args) {
  HandleScope scope;
  //TODO: verify if busy

  if (args.Length() < 2 || !args[0]->IsUint32() || !args[1]->IsUint32())
    return ThrowException(Exception::TypeError(String::New("arguments are (number, number)")));
  Mset_data *aData=NULL;
  Xapian::Error* error=NULL;
  try {
    Enquire *pThis=ObjectWrap::Unwrap<Enquire>(args.This());
    aData = new Mset_data(args[0]->Uint32Value(), args[1]->Uint32Value());
    error = GetMset_process(aData,pThis);
  } catch (Local<Value> ex) {
    return ThrowException(ex);
  }
  if (aData) delete aData;
  if (error!=NULL)
  {
    std::string errorStr=error->get_msg();
    delete error;
    return ThrowException(Exception::Error(String::New(errorStr.c_str())));
  }
  Handle<Value> result=GetMset_convert(aData);
  return scope.Close(result);
}




//These two can be generated with a simple macro for any async operation
int Enquire::GetMset_pool(eio_req *req) {
  OpInfo* aInfo = (OpInfo*) req->data;
  Mset_data *aData = (Mset_data*)aInfo->data;
  AsyncOp<Enquire> *aAsOp = (AsyncOp<Enquire>*)aInfo->op;

  aAsOp->error=GetMset_process(aData, aAsOp->object);
  aAsOp->poolDone();
  return 0;
}
int Enquire::GetMset_done(eio_req *req) {
  HandleScope scope;

  OpInfo* aInfo = (OpInfo*) req->data;
  Mset_data *aData = (Mset_data*)aInfo->data;
  AsyncOp<Enquire> *aAsOp = (AsyncOp<Enquire>*)aInfo->op;

  Handle<Value> argv[2];
  if (aAsOp->error) {
    argv[0] = Exception::Error(String::New(aAsOp->error->get_msg().c_str()));
  } else {
    argv[0] = Null();
    argv[1] = GetMset_convert(aData);
  }

  tryCallCatch(aAsOp->callback, aAsOp->object->handle_, aAsOp->error ? 1 : 2, argv);

  delete aData;
  delete aAsOp;
  delete aInfo;

  return 0;
}
