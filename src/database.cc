#include "node-xapian.h"

Persistent<FunctionTemplate> Database::constructor_template;

void Database::Init(Handle<Object> target) {
  constructor_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "reopen", Reopen);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "add_database", AddDatabase);

  target->Set(String::NewSymbol("Database"), constructor_template->GetFunction());
}

Handle<Value> Database::New(const Arguments& args) {
  HandleScope scope;

  bool aAsync = args.Length() == 2 && args[1]->IsFunction();
  if (args.Length() != +aAsync+1 || !args[0]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are (string, [function])")));

  Database* that = new Database();
  that->Wrap(args.This());

  Open_data* aData = new Open_data(that, args[0]->ToString()); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aAsync, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
}

void Database::Open_process(void* pData, void* pThat) {
  Open_data* data = (Open_data*) pData;
  Database* that = (Database *) pThat;
  if (that->mDb)
    that->mDb->reopen();
  else
    that->mDb = data->writeopts ? new Xapian::WritableDatabase(*data->filename, data->writeopts) : new Xapian::Database(*data->filename);
}

Handle<Value> Database::Open_convert(void* pData) {
  Open_data* data = (Open_data*) pData;
  Database* that = data->that;
  delete data;
  return that->handle_;
}

Handle<Value> Database::Reopen(const Arguments& args) {
  HandleScope scope;

  bool aAsync = args.Length() == 1 && args[0]->IsFunction();
  if (args.Length() != +aAsync || !args[0]->IsString())
    return ThrowException(Exception::TypeError(String::New("arguments are ([function])")));

  Database* that = ObjectWrap::Unwrap<Database>(args.This());

  Open_data* aData = new Open_data(that, Handle<String>()); //deleted by Open_convert on non error

  Handle<Value> aResult;
  try {
    aResult = invoke<Database>(aAsync, args, (void*)aData, Open_process, Open_convert);
  } catch (Handle<Value> ex) {
    delete aData;
    return ThrowException(ex);
  }
  return scope.Close(aResult);
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

