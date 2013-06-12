#ifndef __MODULE_STREAMERS_H
#define __MODULE_STREAMERS_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Streamers"
#endif

#include <Generics/include/Generics.h>
#include <Tracing/include/Tracing.h>
#include <Messages/include/Messages.h>

#undef EXTERNAL

#ifdef STREAMERS_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_STREAMERS_H