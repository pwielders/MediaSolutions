#ifndef __MODULE_GENERICS_H
#define __MODULE_GENERICS_H

#ifndef MODULE_NAME
#define MODULE_NAME		"Generics"
#endif

#include "Portability.h"

#ifdef GENERICS_EXPORTS
#define EXTERNAL		EXTERNAL_EXPORT
#else
#define EXTERNAL		EXTERNAL_IMPORT
#endif

#endif // __MODULE_GENERICS_H
