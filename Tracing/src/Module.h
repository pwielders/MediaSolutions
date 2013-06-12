#ifndef __MODULE_TRACING_H
#define __MODULE_TRACING_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Tracing"
#endif

#include <Generics/include/Generics.h>

#undef EXTERNAL

#ifdef TRACING_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_TRACING_H