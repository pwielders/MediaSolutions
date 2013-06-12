#ifndef __MODULE_RTPSERVER_H
#define __MODULE_RTPSERVER_H

#ifndef MODULE_NAME
#define MODULE_NAME		"RTPServer"
#endif

#include <Generics/include/Generics.h>
#include <Tracing/include/Tracing.h>
#include <Messages/include/Messages.h>
#include <Sources/include/Sources.h>
#include <Sinks/include/Sinks.h>
#include <Sessions/include/Sessions.h>

#undef EXTERNAL

#ifdef RTPSERVER_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_RTPSERVER_H
