#include "tomato.h"

VALUE cTomato = Qnil;
VALUE cTomatoError = Qnil;
VALUE rb_cTime = Qnil;

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_allocate(VALUE klass);

static VALUE fTomato_execute(VALUE self, const char *javascript, const char *filename);

static void tomato_mark(V8Tomato *tomato);
static void tomato_free(V8Tomato *tomato);

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
  
  /* init error-specific junk */
  err_init();
  
  /* init V8-specific junk */
  v8_init();
}


static VALUE fTomato_allocate(VALUE klass)
{
  V8Tomato *tomato = new V8Tomato;
  VALUE instance = Data_Wrap_Struct(klass, tomato_mark, tomato_free, tomato);
  rb_iv_set(instance, "@v8", rb_class_new_instance(0, 0, cV8));
  
  tomato->rb_instance = instance;
  tomato->rb_references = rb_hash_new();

  return instance;  
}

static void tomato_mark(V8Tomato *tomato)
{
  rb_gc_mark(tomato->rb_references);
}

static void tomato_free(V8Tomato *tomato)
{
  delete tomato;
}

/* Runs a String of JavaScript code. */
static VALUE fTomato_run(int argc, VALUE *argv, VALUE self)
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
    VALUE return_value = fTomato_execute(self, javascript, filename);
    return return_value;
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

static VALUE fTomato_execute(VALUE self, const char *javascript, const char *filename)
{
  V8Tomato *tomato;
  Data_Get_Struct(self, V8Tomato, tomato);
  Context::Scope context_scope(tomato_v8_context(tomato));  
  HandleScope handle_scope;
  Handle<String> source_code = String::New(javascript);
  Handle<String> name = String::New(filename);
  return execute(tomato, source_code, name);
}

Persistent<Context> tomato_v8_context(V8Tomato *tomato)
{
  V8Context *v8;
  Data_Get_Struct(rb_iv_get(tomato->rb_instance, "@v8"), V8Context, v8);
  return v8->context;
}