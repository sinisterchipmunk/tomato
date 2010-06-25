#include "tomato.h"

VALUE eTransError = Qnil;

Local<Object> js_error_from(VALUE ruby_error)
{
  VALUE message = rb_funcall(ruby_error, rb_intern("to_s"), 0);
  Local<Object> js_error = Local<Object>::Cast(Exception::Error(String::New(StringValuePtr(message))));
  js_error->Set(String::New("_is_ruby_error"), Boolean::New(true), DontEnum);
  return js_error;  
}

void err_init(void)
{
  eTransError = rb_define_class_under(cTomato, "TranslatableError", cTomatoError);
}


/* The following was adapted from samples/shell.cc in v8 project. */
void raise_error(TryCatch *try_catch)
{
  HandleScope handle_scope;
  String::Utf8Value exception(try_catch->Exception());
  Handle<Message> message = try_catch->Message();
  
  /* check for Ruby error; if it's an error, reraise $!. TODO: See $! can ever be a different error... */
  Handle<Value> is_ruby_error = Local<Object>::Cast(try_catch->Exception())->Get(String::New("_is_ruby_error"));
  if (is_ruby_error->IsBoolean() && is_ruby_error->IsTrue())
  {
    throw rb_gv_get("$!");
  }
  
  std::string errmsg;
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    errmsg += ToCString(exception);
    errmsg += "\n";
  } else {
    // Print (filename):(line number): (message).
    String::Utf8Value filename(message->GetScriptResourceName());
    int linenum = message->GetLineNumber();
    errmsg += ToCString(filename);
    errmsg += ":";
    errmsg += linenum;
    errmsg += ToCString(exception);
    errmsg += "\n";
    
    // Print line of source code.
    String::Utf8Value sourceline(message->GetSourceLine());
    errmsg += ToCString(sourceline);
    errmsg += "\n";

    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      errmsg += " ";
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      errmsg += "^";
    }
    errmsg += "\n";
    String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = ToCString(stack_trace);
      errmsg += stack_trace_string;
      errmsg += "\n";
    }
  }

  throw errmsg;
}
