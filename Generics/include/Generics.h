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

#ifndef __GENERICS_H
#define __GENERICS_H

#include "../src/Portability.h"
#include "../src/Module.h"
#include "../src/NodeId.h"
#include "../src/StateTrigger.h"
#include "../src/SocketPort.h"
#include "../src/SynchronizationDispatcherType.h"
#include "../src/UniformResourceLocator.h"
#include "../src/SocketServerType.h"
#include "../src/IMedia.h"
#include "../src/TextStreamerType.h"
#include "../src/Number.h"
#include "../src/Enumerate.h"
#include "../src/Time.h"
#include "../src/Timer.h"
#include "../src/Parser.h"
#include "../src/Proxy.h"
#include "../src/Singleton.h"
#include "../src/DataElement.h"
#include "../src/DataStorage.h"
#include "../src/DataElementIterator.h"
#include "../src/DataElementFile.h"
#include "../src/DataElementStreamer.h"
#include "../src/DataElementStreamerType.h"
#include "../src/DataStorage.h"
#include "../src/SlottedStorage.h"
#include "../src/SlottedStorageIterator.h"
#include "../src/Trace.h"
#include "../src/StlIterator.h"
#include "../src/DataElementContainer.h"
#include "../src/System.h"
#include "../src/MediaStreamer.h"

#include <list>
#include <map>
#include <vector>

#ifdef __WIN32__
#pragma comment(lib, "Generics.lib")
#endif

#endif