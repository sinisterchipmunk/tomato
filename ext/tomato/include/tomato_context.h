#ifndef TOMATO_CONTEXT_H
#define TOMATO_CONTEXT_H

/* tomato_context.cpp */
class TomatoContext
{
  private:
    VALUE rb_instance;
    Persistent<Context> js_context;

    VALUE compile_and_run(Handle<String> source, Handle<Value> name);

  public:
    TomatoContext();
    ~TomatoContext();
    Persistent<Context> context() { return js_context; }
    Handle<Object> global();
    
    VALUE instance() { return rb_instance; }
    VALUE execute(const char *javascript, const char *filename);
};

extern void Init_v8(void);
extern VALUE cTomatoContext;

#endif//TOMATO_CONTEXT_H
