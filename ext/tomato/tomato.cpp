#include "tomato.h"

VALUE cTomato = Qnil;
VALUE cTomatoError = Qnil;
VALUE rb_cTime = Qnil;

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_version(VALUE self);
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
  
  /* instance method "version" */
  rb_define_method(cTomato, "version", (ruby_method_vararg *)&fTomato_version, 0);
  
  /* instance method "bind_method" */
  rb_define_method(cTomato, "_bind_method", (ruby_method_vararg *)&fTomato_bind_method, -1);

  /* init error-specific junk */
  err_init();
}


static VALUE fTomato_allocate(VALUE klass)
{
  V8Tomato *tomato = new V8Tomato;
  VALUE instance = Data_Wrap_Struct(klass, tomato_mark, tomato_free, tomato);
  
  HandleScope handle_scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  tomato->context = Context::New(NULL, global);
  tomato->rb_instance = instance;
  
  return instance;  
}

static void tomato_mark(V8Tomato *tomato) { }
static void tomato_free(V8Tomato *tomato)
{
  tomato->context.Dispose();
  delete tomato;
}

static VALUE fTomato_version(VALUE self)
{
  return rb_str_new2(V8::GetVersion());
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
  Context::Scope context_scope(tomato->context);  
  HandleScope handle_scope;
  Handle<String> source_code = String::New(javascript);
  Handle<String> name = String::New(filename);
  return execute(tomato, source_code, name);
}
