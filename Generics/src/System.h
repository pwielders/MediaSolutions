#ifndef __SYSTEM_H
#define __SYSTEM_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "String.h"

namespace Solutions { namespace Generics
{
	class EXTERNAL System
	{
		public:
			static String WorkingDirectory ();
	};

} } // namespace Solutions::Generics

#endif // __SYSTEM_H

