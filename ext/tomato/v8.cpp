#include "tomato.h"

static VALUE fV8_version(VALUE self);
static VALUE fV8_allocate(VALUE klass);
static void v8_mark(V8Context *v8);
static void v8_free(V8Context *v8);
static v8::Handle<v8::Value> debug(const Arguments &args);

VALUE cV8 = Qnil;

void v8_init(void)
{
  cV8 = rb_define_class_under(cTomato, "V8", rb_cObject);
  
  /* class method "version" */
  rb_define_module_function(cV8, "version", (ruby_method_vararg *)&fV8_version, 0);
  
  /* object allocation method, which will initialize our V8 context */
  rb_define_alloc_func(cV8, (ruby_method_1 *)&fV8_allocate);
}

static VALUE fV8_allocate(VALUE klass)
{
  V8Context *v8 = new V8Context;
  VALUE instance = Data_Wrap_Struct(klass, v8_mark, v8_free, v8);
  
  HandleScope handle_scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::New("debug"), FunctionTemplate::New(debug));
  v8->context = Context::New(NULL, global);

  return instance;  
}

static void v8_mark(V8Context *v8)
{
  //rb_gc_mark(tomato->rb_references);
}

static void v8_free(V8Context *v8)
{
  v8->context.Dispose();
  delete v8;
}

static VALUE fV8_version(VALUE self)
{
  return rb_str_new2(V8::GetVersion());
}

static v8::Handle<v8::Value> debug(const Arguments &args)
{
  Handle<Value> arg;
  Tomato *tomato = (Tomato *)(Handle<External>::Cast(args.Holder()->Get(String::New("_tomato")))->Value());
  for (int i = 0; i < args.Length(); i++)
  {
    arg = args[i];
    if (i > 0) printf(", ");
    String::Utf8Value str(inspect_js(tomato, args[i]));
    printf("<%s>", ToCString(str));
  }
  printf("\n");
  return Null();
}
