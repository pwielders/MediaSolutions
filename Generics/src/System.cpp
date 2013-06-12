#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#include "System.h"

namespace Solutions { namespace Generics
{

/* static */ String System::WorkingDirectory ()
{
	TCHAR pathName[_MAX_PATH]; // This is a buffer for the text

	_tgetcwd (pathName, sizeof(pathName) - 1);

	pathName[sizeof(pathName)-1] = '\0';

	return (pathName);
}

} } // namespace Solutions::Generics
