#ifndef WRAPPERS_H
#define WRAPPERS_H

/* in tomato.cpp */
//extern void UnbindValue(Persistent<Value> bound_object, void*);

/* in wrappers.cpp */
extern void Init_wrappers(void);

/*
  Contains a Javascript value which can then be converted to a corresponding Ruby type.
  If the value itself wraps a Ruby object, the original object is returned. Otherwise,
  a new instance of the closest matching Ruby type will be returned.
  
  For example, if the value is the number 5, then toRuby() will return the Fixnum 5.
  If the value is wrapped around a Ruby VALUE, then the VALUE itself will be returned.
*/
class JavascriptValue
{
  private:
    Handle<Value> value;
    VALUE toRubyString();
    VALUE toRubyNumber();
    VALUE toRubyBoolean();
    VALUE toRubyDate();
    VALUE toRubyArray();

  public:
    JavascriptValue(Handle<Value> value) { this->value = value; }
    Handle<Value> toJSON(Handle<Value> json);
    VALUE toRuby();
};

/*
  Contains a Ruby VALUE which can then be converted to a corresponding Javascript type.
  If the value itself wraps a Javascript object, the original object is returned. Otherwise,
  a new instance of the closest matching Javascript type will be returned.
*/
class RubyValue
{
  private:
    VALUE rb_value;
    
    /* rb_self is an instance of the Ruby class Tomato::RubyValue. rb_self is the object
    that we add to Tomato#references. The reason we effectively wrap the wrapper is so that
    we can unbind the object when the Tomato is garbage collected by Ruby. Since the Tomato
    is GCed, it stands to reason that its corresponding JavaScript context will also die, and
    all persistent references within that context should be disposed. */
    VALUE rb_self;
    
    Persistent<Object> js_value;
    
  public:
    RubyValue();
    ~RubyValue();
    void set(VALUE object);
    void rb_gc_mark(void) { ::rb_gc_mark(this->rb_value); }
    VALUE value() { return this->rb_value; }
    char *inspect() { VALUE inspection = rb_funcall(this->rb_value, rb_intern("inspect"), 0); return StringValuePtr(inspection); }
    
    Handle<Value> toJavascript() { return this->js_value; }
    Handle<Value> toJavascriptPrimitive();
    VALUE rb_wrapper() { return this->rb_self; }
};

#endif//WRAPPERS_H
