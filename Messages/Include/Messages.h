// ===========================================================================
//
// Filename:    Messages.h
//
// Description: Module include file for the Messages module
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2010/01/04
//
// ===========================================================================

#ifndef __MESSAGES_H
#define __MESSAGES_H

#include "../src/Module.h"
#include "../src/MPEGHeader.h"
#include "../src/MPEGSource.h"
#include "../src/RTSPMessage.h"
#include "../src/RTPMessage.h"
#include "../src/RTCPMessage.h"
#include "../src/SDPMessage.h"
#include "../src/RIFFHeader.h"
#include "../src/AVIMediaHeader.h"
#include "../src/MPEGTransport.h"
#include "../src/MPEGAudio.h"
#include "../src/MPEGVideo.h"

#ifdef __WIN32__
#pragma comment(lib, "Messages.lib")
#endif

#endif // __MESSAGES_H