#ifndef __ISINK_H
#define __ISINK_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sink
{
	typedef enum EnumSinkType
	{
		UNKNOWN,
		RTP_UDP,
		RTP_TCP,
	};

	struct EXTERNAL ISink
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~ISink() { }

		virtual uint32 Sequence () const = 0;
		virtual uint32 SyncStamp () const = 0;
		virtual uint32 SinkSize () const = 0;

		// Determines the type of this sink.
		virtual void Sink (const uint64 presentationTime, Generics::DataElementContainer& frame) = 0;
	};

	struct EXTERNAL ISinkFactory
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~ISinkFactory() { }

		// Check if the source that we are going to offer is supported by this sink...
		virtual bool Supported(const Source::ISource* source) const = 0;

		// Determines the type of this source.
		virtual Sink::EnumSinkType Type () const = 0;

		// Create a sink, if possible...
		virtual ISink* Create(const Sink::EnumSinkType type, const Source::ISource* source, Generics::NodeId& toPoint) = 0;
	};

} } // namespace Solutions::Sink

#endif // __ISINK_H