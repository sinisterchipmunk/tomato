#include "tomato.h"

// Executes a string within the current v8 context.
VALUE execute(V8Tomato *tomato, Handle<String> source, Handle<Value> name)
{
  HandleScope handle_scope;
  TryCatch try_catch;
  Handle<Script> script = Script::Compile(source, name);
  if (script.IsEmpty())
  {
    raise_error(&try_catch);
    return Qnil;
  }
  else
  {
    Handle<Value> result = script->Run();
    if (result.IsEmpty())
    {
      raise_error(&try_catch);
      return Qnil;
    }
    else
    {
      return ruby_value_of(tomato, result);
    }
  }
}
