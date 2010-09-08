#ifndef DEBUG_H
#define DEBUG_H

#if DEBUG == 1
  #define INC_TRACE_DEPTH (++TRACE_DEPTH)
  #define DEC_TRACE_DEPTH (--TRACE_DEPTH)
#else
  #define INC_TRACE_DEPTH 
  #define DEC_TRACE_DEPTH 
#endif

#define TRACE_INDENTATION 3

/* debug.cpp */
extern int TRACE_DEPTH;
extern void Init_debug(void);
extern void trace(VALUE string);
extern void trace(const char *string, ...);

#endif//DEBUG_H
