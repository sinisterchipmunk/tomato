#include "tomato.h"

RB_DEFINE_CLASS_FUNCS(Tomato);

static VALUE fTomato_run(int argc, VALUE *argv, VALUE self);
static VALUE fTomato_bind_object(int argc, VALUE *argv, VALUE self);

VALUE cTomato;
VALUE cTomatoError;

Tomato::Tomato()
{
  trace("create Tomato<%x>", this);
  INC_TRACE_DEPTH;
    this->rb_instance = Data_Wrap_Struct(cTomato, RB_MARK_FUNC(Tomato), RB_FREE_FUNC(Tomato), this);
    rb_iv_set(this->rb_instance, "@context", rb_class_new_instance(0, 0, cTomatoContext));
  DEC_TRACE_DEPTH;
}

Tomato::~Tomato()
{
  trace("destroy Tomato<%x>", this);
}

void Tomato::rb_gc_mark()
{
  ::rb_gc_mark(this->rb_instance);
  references.rb_gc_mark();
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
  INC_TRACE_DEPTH;
    Context::Scope csx(this->context()->context());
    Handle<Object> global = this->global();
    RubyValue *value = new RubyValue();
    value->set(object);
    
    trace("- adding Ruby reference to object");
    INC_TRACE_DEPTH;
      references.add(value->rb_wrapper());
    DEC_TRACE_DEPTH;
    
    trace("- setting bound object on global context");
    global->Set(String::New(object_name), value->toJavascript());
  DEC_TRACE_DEPTH;
}

extern "C"
void Init_tomato(void)
{
  Init_debug();
  trace("Tomato system initialization has started.");
  INC_TRACE_DEPTH;

    cTomato = rb_define_class("Tomato", rb_cObject);
    cTomatoError = rb_define_class_under(cTomato, "Error", rb_eRuntimeError);
  
    rb_define_alloc_func(cTomato, (VALUE(*)(VALUE))&RB_ALLOC_FUNC(Tomato));
    
    rb_define_method(cTomato, "run", (VALUE(*)(...))&fTomato_run, -1);
    rb_define_method(cTomato, "bind_object", (VALUE(*)(...))&fTomato_bind_object, -1);
    
    Init_v8();
    Init_wrappers();
    
    trace("- completed.");
  DEC_TRACE_DEPTH;
}

TomatoContext *Tomato::context()
{
  #if DEBUG == 1
    VALUE instance_str = rb_funcall(rb_iv_get(this->rb_instance, "@context"), rb_intern("inspect"), 0);
    trace("Tomato<%x>::context on %s", this, StringValuePtr(instance_str));
  #endif
  INC_TRACE_DEPTH;
    RB_EXTRACT_STRUCT(rb_iv_get(this->rb_instance, "@context"), TomatoContext, context);
  DEC_TRACE_DEPTH;
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
