#include "tomato.h"

void Init_debug(void)
{
  if (LOG_FILE != 0)
    fclose(fopen(LOG_FILE, "w")); // just to null it out
}

void trace(VALUE string)
{
#ifdef DEBUG
  trace(StringValuePtr(string));
#endif
}

void trace(const char *string, ...)
{
#ifdef DEBUG
  va_list argp;

  va_start(argp, string);
  if (LOG_FILE == 0)
  {
    vfprintf(stderr, string, argp);
    fprintf(stderr, "\n");
  }
  else
  {
    FILE *out = fopen(LOG_FILE, "a+");
    vfprintf(out, string, argp);
    fprintf(out, "\n");
    fclose(out);
  }
  va_end(argp);
#endif
}
