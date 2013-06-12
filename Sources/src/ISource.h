#ifndef __ISOURCE_H
#define __ISOURCE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "SourceTypes.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	// Indicates the container type that holds the specific data.
	typedef enum EnumSourceType
	{
		UNKNOWN,
		MPEG_STREAM,
		MPEG_PACKAGES,
		AVI_MEDIA,
	};

	// Indicates the kind of media that is contained in this source.
	typedef enum EnumStreamType
	{
		AUDIO,
		VIDEO,
		AUDIOVIDEO,
		TEXT
	};

	struct EXTERNAL IProperties
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~IProperties() { }

		// Determines the type of this source. e.g. MPEG1/MPEG2/TS/
		virtual EnumSourceType SourceType () const = 0;

		virtual EnumStreamType StreamType () const = 0;

		// The number of samples for each channel per second
		virtual uint32 BitRate () const = 0;
	};

	struct EXTERNAL IVideoProperties : public IProperties
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~IVideoProperties() { }

		// Determines the number of frames available in this GOP.
		virtual uint16 GOPSize () const = 0;

		// Determines the index of the frame in GOP
		virtual uint16 FrameInGOP () const = 0; 
	};

	struct EXTERNAL ISource
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~ISource() { }

		// This is the time that the current appointed data should be presented,
		// relative to the time that the first data was aquired.
		virtual uint64 PresentationTime () const = 0;

		// In case of persistent streams, there is a possibility to move back to
		// a presentation time, relative to the time the first data was aquired.
		virtual void PresentationTime (const uint64 slot) = 0;

		// This progresses the pointer one forward and stores all information it
		// moves over into the DataElementContainer. 
		// In case the value MAXUINT32 is returned, it means that we reached the 
		// end of the stream, nothing to send.
		// If 0 is returned, it means that there is currently no package available, 
		// move on to the next slot, where we might have data.
		virtual uint32 NextPackage (Generics::DataElementContainer& framePackage) = 0;

		// This method gives access to the interface that returns specific properties 
		// of the source. If it is an MPEG video source it could return thinks like 
		// the size of the GOP etc..
		virtual const IProperties* Properties () const = 0;
	};

	struct EXTERNAL ISources
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~ISources() {}

		// This identifier is uniquely identfies the root of the media being
		// streamed. (e.g. filename, source/nodeid where the data is coming from etc..
		virtual const Generics::OptionalType<Generics::URL> Content () const = 0;

		// This identifier gives the name of the main parser of the source
		// Used during the SDP identification process
		virtual const TCHAR* Parser () const = 0;

		// If possible, this indicates the duration that tht this stream takes.
		// Normally this is only possible in case of persistent/recorded streams.
		virtual uint64 Duration () const = 0;

		// Set the internal iterator back just before the first source interface
		virtual void Reset() = 0;

		// Move on to the next Source, if located on a valid source return true.
		virtual bool Next () = 0;

		// Get a streamer for each available channel.
		virtual ISource* Source () = 0;

		// Return the factory that was responsible for the creation of this instance.
		virtual struct ISourceFactory* Factory() = 0;
	};

	struct ISourceFactory
	{
		// They might want to destruct the interface. Make sure the destructor of the 
		// implementing object class is called.
		virtual ~ISourceFactory() { }

		virtual bool Supported(const Generics::URL& identification) const = 0;
		virtual Generics::ProxyType<ISources> Create (const Generics::URL& identification) = 0;
	};

} } // namespace Solutions::Source

#endif // __ISOURCE_H