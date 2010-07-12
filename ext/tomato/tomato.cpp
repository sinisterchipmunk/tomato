#include "tomato.h"

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_allocate(VALUE klass);
static VALUE fTomato_execute(VALUE self, const char *javascript, const char *filename);
static void tomato_mark(Tomato *tomato);
static void tomato_free(Tomato *tomato);

Tomato::Tomato(VALUE klass)
{
  this->rb_instance = Data_Wrap_Struct(klass, tomato_mark, tomato_free, this);
  this->rb_references = rb_hash_new();
  rb_iv_set(this->rb_instance, "@v8", rb_class_new_instance(0, 0, cV8));
}

Tomato::~Tomato()
{
  //
}

void Tomato::rb_gc_mark() { ::rb_gc_mark(this->rb_references); }

Persistent<Context> Tomato::context()
{
  V8Context *v8;
  Data_Get_Struct(rb_iv_get(this->rb_instance, "@v8"), V8Context, v8);
  return v8->context;
}

VALUE Tomato::execute(const char *javascript, const char *filename)
{
  Context::Scope context_scope(this->context());  
  HandleScope handle_scope;
  Handle<String> source_code = String::New(javascript);
  Handle<String> name = String::New(filename);
  return compile_and_run(source_code, name);
}

VALUE Tomato::compile_and_run(Handle<String> source, Handle<Value> name)
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
      return ruby_value_of(this, result);
    }
  }
}


VALUE Tomato::run(int argc, VALUE *argv)
{
  if (argc == 0)
  {
    rb_raise(rb_eArgError, "expected at least 1 argument: the JavaScript to be executed");
    return Qnil;
  }
  else if (argc > 2)
  {
    rb_raise(rb_eArgError, "expected at most 2 arguments: the JavaScript to be executed, and an optional file name");
    return Qnil;
  }
  
  char *javascript = StringValuePtr(argv[0]);
  char *filename;
  if (argc == 2)
    filename = StringValuePtr(argv[1]);
  else
    filename = (char *)"(dynamic)";

  /* We have to do things this way because rb_raise does a longjmp, which causes C++ destructors not to fire.
     V8 is somewhat reliant on those destructors, so that's a Bad Thing. */
  try
  {
    return this->execute(javascript, filename);
  }
  catch (std::string const &exception_message)
  {
    rb_raise(cTomatoError, "%s", exception_message.c_str());
  }
  catch (VALUE const &rbErr)
  {
    rb_exc_raise(rbErr);
  }
}



/* The following section interfaces Tomato with the Ruby API */

VALUE cTomato = Qnil;
VALUE cTomatoError = Qnil;
VALUE rb_cTime = Qnil;

extern "C"
void Init_tomato(void)
{
  /* Look up the DateTime class */
  rb_cTime = rb_const_get(rb_cObject, rb_intern("Time"));

  /* define the Tomato class */
  cTomato = rb_define_class("Tomato", rb_cObject);
  
  /* definte the Tomato::Error class */
  cTomatoError = rb_define_class_under(cTomato, "Error", rb_eRuntimeError);
  
  /* object allocation method, which will initialize our V8 context */
  rb_define_alloc_func(cTomato, (ruby_method_1 *)&fTomato_allocate);
  
  /* instance method "run" accepts a String argument */
  rb_define_method(cTomato, "run", (ruby_method_vararg *)&fTomato_run, -1);
    
  /* instance method "bind_method" */
  rb_define_method(cTomato, "_bind_method", (ruby_method_vararg *)&fTomato_bind_method, -1);
  
  /* instance method "_bind_class" */
  rb_define_method(cTomato, "_bind_class", (ruby_method_vararg *)&fTomato_bind_class, 2);
  
  /* init V8-specific junk */
  v8_init();
}


static VALUE fTomato_allocate(VALUE klass)
{
  Tomato *tomato = new Tomato(klass);
  return tomato->rb_instance;
}

static void tomato_mark(Tomato *tomato)
{
  tomato->rb_gc_mark();
}

static void tomato_free(Tomato *tomato)
{
  delete tomato;
}

/* Runs a String of JavaScript code. */
static VALUE fTomato_run(int argc, VALUE *argv, VALUE self)
{
  Tomato *tomato;
  Data_Get_Struct(self, Tomato, tomato);
  return tomato->run(argc, argv);
}
