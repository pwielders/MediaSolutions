#ifndef __IMEDIA_H
#define __IMEDIA_H

#include "Portability.h"

namespace Solutions { namespace Generics
{

	const uint32  ERROR_GENERAL           = 1;
	const uint32  ERROR_UNAVAILABLE       = 2;
	const uint32  ERROR_ASYNC_FAILED      = 3;
	const uint32  ERROR_ASYNC_ABORTED     = 4;
	const uint32  ERROR_ILLEGAL_STATE     = 5;
	const uint32  ERROR_OPENING_FAILED    = 6;
	const uint32  ERROR_ACCEPT_FAILED     = 7;
	const uint32  ERROR_PENDING_SHUTDOWN  = 8;
	const uint32  ERROR_ALREADY_CONNECTED = 9;
	const uint32  ERROR_CONNECTION_CLOSED = 10;
	const uint32  ERROR_TIMEDOUT          = 11;

	struct EXTERNAL IMedia
	{
		virtual SYSTEM_SYNC_HANDLE	SynchronisationHandle	() const = 0;
		virtual uint8				PendingTriggers			() const = 0;
		virtual uint32				Abort					() = 0;
		virtual uint32				Send					(uint32&		nSize, 
															 const uint8	szBytes[],
															 uint32			nTime = INFINITE) = 0;
		virtual uint32				Receive					(uint32&		nSize, 
															 uint8			szBytes[],
															 uint32			nTime = INFINITE) = 0;
	};

} } // namespace Solutions::Generics

#endif // __IMEDIA_H
