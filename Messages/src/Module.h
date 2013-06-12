#ifndef __MODULE_MESSAGES_H
#define __MODULE_MESSAGES_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Messages"
#endif

#include <Generics/include/Generics.h>

#undef EXTERNAL

#ifdef MESSAGES_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_MESSAGES_H
