#include "tomato.h"

int TRACE_DEPTH = 0;

void Init_debug(void)
{
  if (LOG_FILE != 0)
    fclose(fopen(LOG_FILE, "w")); // just to null it out
}

void trace(VALUE string)
{
#ifdef DEBUG
  VALUE inspection = rb_funcall(string, rb_intern("inspect"), 0);
  trace(StringValuePtr(inspection));
#endif
}

void trace(const char *string, ...)
{
#ifdef DEBUG
  va_list argp;
  int i;

  va_start(argp, string);
  if (LOG_FILE == 0)
  {
    for (i = 0; i < TRACE_DEPTH*TRACE_INDENTATION; i++) fprintf(stderr, " ");
    vfprintf(stderr, string, argp);
    fprintf(stderr, "\n");
  }
  else
  {
    FILE *out = fopen(LOG_FILE, "a+");
    for (i = 0; i < TRACE_DEPTH*TRACE_INDENTATION; i++) fprintf(out, " ");
    vfprintf(out, string, argp);
    fprintf(out, "\n");
    fclose(out);
  }
  va_end(argp);
#endif
}
