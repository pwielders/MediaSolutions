#ifndef __MODULE_SINKS_H
#define __MODULE_SINKS_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Sinks"
#endif

#include <Generics/include/Generics.h>
#include <Messages/include/Messages.h>
#include <Sources/include/Sources.h>

#undef EXTERNAL

#ifdef SINKS_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_SINKS_H
