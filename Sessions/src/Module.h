#ifndef __MODULE_SESSIONS_H
#define __MODULE_SESSIONS_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Sessions"
#endif

#include <Generics/include/Generics.h>
#include <Tracing/include/Tracing.h>
#include <Sources/include/Sources.h>
#include <Sinks/include/Sinks.h>
#include <Messages/include/Messages.h>

#undef EXTERNAL

#ifdef SESSIONS_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_SESSIONS_H
