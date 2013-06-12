#ifndef __TRACE_H
#define __TRACE_H

#include <stdio.h>
#include <assert.h>

#include "Module.h"

#ifndef _TRACE_LEVEL
#define _TRACE_LEVEL         0
#endif

#define TRACE_POINTER(x)     reinterpret_cast<unsigned long>(x)
#define TRACE_PRINTF(x,y,z)  printf(x y "\n", z); fflush (stdout);

#if _TRACE_LEVEL > 4
#define TRACE_L5(x,y)           TRACE_PRINTF("TRACE <L5>: ",x,y)
#else
#define TRACE_L5(x,y)   
#endif

#if _TRACE_LEVEL > 3
#define TRACE_L4(x,y)           TRACE_PRINTF("TRACE <L4>: ",x,y)
#else
#define TRACE_L4(x,y)   
#endif

#if _TRACE_LEVEL > 2
#define TRACE_L3(x,y)           TRACE_PRINTF("TRACE <L3>: ",x,y)
#else
#define TRACE_L3(x,y)   
#endif

#if _TRACE_LEVEL > 1
#define TRACE_L2(x,y)           TRACE_PRINTF("TRACE <L2>: ",x,y)
#else
#define TRACE_L2(x,y)   
#endif

#if _TRACE_LEVEL > 0
#define TRACE_L1(x,y)           TRACE_PRINTF("TRACE <L1>: ",x,y)
#define TRACE_FLUSH             fflush (stdout)
#ifndef ASSERT
#define ASSERT(x)               assert(x)
#endif
#else
#define TRACE_L1(x,y)   
#define TRACE_FLUSH 
#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x)               assert(x)
#else
#define ASSERT(x)
#endif
#endif
#endif

#define MSG_ASSERT(x,y)         if ((x) == FALSE) { TRACE_L1 (y, 0); ASSERT (FALSE); }
#define MSG_DROP(x, y)          printf(x, y)

#endif //__TRACE_H
