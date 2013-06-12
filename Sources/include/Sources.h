// ===========================================================================
//
// Filename:    Generics.h
//
// Description: Module include file for the InfraStructure module
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2010/01/04
//
// ===========================================================================

#ifndef __SOURCES_H
#define __SOURCES_H

#include "../src/Module.h"
#include "../src/ISource.h"
#include "../src/MPG12Source.h"
#include "../src/MPG3Source.h"
#include "../src/SourceFile.h"
#include "../src/CachedSource.h"
#include "../src/SourceCollator.h"
#include "../src/Source.h"
#include "../src/Definitions.h"
#include "../src/SourceFactory.h"
#include "../src/SourceTypes.h"

#ifdef __WIN32__
#pragma comment(lib, "Sources.lib")
#endif

#endif // __SOURCES_H