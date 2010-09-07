#include "tomato.h"

RB_DEFINE_CLASS_FUNCS(Tomato);

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_bind_object(int argc, VALUE *argv, VALUE self);

VALUE cTomato;
VALUE cTomatoError;

Tomato::Tomato()
{
  trace("create Tomato<%x>", this);
  this->rb_instance = Data_Wrap_Struct(cTomato, RB_MARK_FUNC(Tomato), RB_FREE_FUNC(Tomato), this);
  this->rb_references = rb_hash_new();
  rb_iv_set(this->rb_instance, "@context", rb_class_new_instance(0, 0, cTomatoContext));
}

Tomato::~Tomato()
{
  trace("destroy Tomato<%x>", this);
}

void Tomato::rb_gc_mark()
{
  ::rb_gc_mark(this->rb_instance);
  ::rb_gc_mark(this->rb_references);
}

//void UnbindValue(Persistent<Value> bound_object, void*)
//{
//  trace("UnbindValue");
////  if (!bound_object->IsObject()) throw std::string("  ! Bound object is not an object (huh?!)");
////  Handle<Value> external = (Handle<Object>::Cast(bound_object))->GetHiddenValue(String::New("ruby_value"));
////  if (!external->IsExternal()) throw std::string("  ! Bound object does not contain an External");
////  RubyValue *wrapper = (RubyValue *)External::Unwrap(external);
////  
////  trace("  - Releasing bound value %s", wrapper->inspect());
////  delete wrapper;
//}

void Tomato::bind(VALUE object, const char *object_name)
{
  #if DEBUG == 1
    VALUE inspection = rb_funcall(object, rb_intern("inspect"), 0);
    trace("Tomato<%x>#bind: %s as %s", this, StringValuePtr(inspection), object_name);
  #endif
  Context::Scope csx(this->context()->context());
  Handle<Object> global = this->global();
  RubyValue *value = new RubyValue();
  value->set(object);
  trace("  - adding Ruby reference to object");
  add_reference(value->rb_wrapper());
  
  trace("  - setting bound object on global context");
  global->Set(String::New(object_name), value->toJavascript());
  
}

VALUE Tomato::reference(VALUE obj)
{
  VALUE object_id = rb_funcall(obj, rb_intern("object_id"), 0);
  VALUE ary = rb_hash_aref(this->rb_references, INT2FIX(object_id));
  if (NIL_P(ary))
  { /* if entry doesn't exist, create it */
    ary = rb_ary_new2(2);
    rb_hash_aset(this->rb_references, object_id, ary);
    rb_ary_store(ary, 0, INT2FIX(0));
    rb_ary_store(ary, 1, obj);
    #if DEBUG == 1
      VALUE inspection = rb_funcall(ary, rb_intern("inspect"), 0);
      trace("Tomato<%x>#reference: creating entry %d => %s", this, NUM2LONG(object_id), StringValuePtr(inspection));
    #endif
  }
  return ary;
}

void Tomato::add_reference(VALUE obj)
{
  VALUE ref = this->reference(obj);
  int index = FIX2INT(rb_ary_entry(ref, 0)) + 1;
  rb_ary_store(ref, 0, INT2FIX(index));
  #if DEBUG == 1
    VALUE inspection = rb_funcall(ref, rb_intern("inspect"), 0);
    trace("Tomato<%x>#add_reference - %s", this, StringValuePtr(inspection));
  #endif
}

void Tomato::remove_reference(VALUE obj)
{
//  VALUE ref = this->reference(obj);
//  if (rb_ary_entry(obj, 0) == 0)
//  {
//    rb_raise(rb_e)
//  }
}


extern "C"
void Init_tomato(void)
{
  Init_debug();
  trace("Tomato system initialization has started.");

  cTomato = rb_define_class("Tomato", rb_cObject);
  cTomatoError = rb_define_class_under(cTomato, "Error", rb_eRuntimeError);

  rb_define_alloc_func(cTomato, (VALUE(*)(VALUE))&RB_ALLOC_FUNC(Tomato));
  
  rb_define_method(cTomato, "run", (VALUE(*)(...))&fTomato_run, -1);
  rb_define_method(cTomato, "bind_object", (VALUE(*)(...))&fTomato_bind_object, -1);
  
  Init_v8();
  Init_wrappers();
  
  trace("Tomato system initialization has been completed.");
}

TomatoContext *Tomato::context()
{
  #if DEBUG == 1
    VALUE instance_str = rb_funcall(rb_iv_get(this->rb_instance, "@context"), rb_intern("inspect"), 0);
    trace("Tomato<%x>::context on %s", this, StringValuePtr(instance_str));
  #endif
  RB_EXTRACT_STRUCT(rb_iv_get(this->rb_instance, "@context"), TomatoContext, context);
  return context;
}

/*
  call-seq:
    tomato.run("some javascript")

  Runs some JavaScript code and returns the result.
*/
static VALUE fTomato_run(int argc, VALUE *argv, VALUE self)
{
  return rb_funcall2(rb_iv_get(self, "@context"), rb_intern("run"), argc, argv);
}

/*
  call-seq:
    tomato.bind_object(5, "number")
    tomato.run("obj + 5") #=> 10
    
  Binds a Ruby object to this Tomato, making it accessible within this
  JavaScript context.
  
  Returns self.
*/
static VALUE fTomato_bind_object(int argc, VALUE *argv, VALUE self)
{
  if (argc != 2) rb_raise(rb_eArgError, "Expected 2 arguments");
  
  HandleScope scope;
  RB_EXTRACT_STRUCT(self, Tomato, tomato);
  tomato->bind(argv[0], StringValuePtr(argv[1]));
  
  return self;
}
