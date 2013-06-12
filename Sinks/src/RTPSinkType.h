#ifndef __RTPSINK_H
#define __RTPSINK_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "RTPSink.h"
#include "ISink.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
// ------------------------------------------------------------------------------------------------------
// RTPSink: class RTPChannelSinkUDP
// ------------------------------------------------------------------------------------------------------
namespace Solutions { namespace Sink
{
	template <typename SOCKETTYPE, Sink::EnumSinkType SINKTYPE> 
	class RTPSinkType : public ISink
	{
		private:
			RTPSinkType();
			RTPSinkType(const RTPSinkType&);
			RTPSinkType operator= (const RTPSinkType&);

		public:
			RTPSinkType(const Source::IProperties& sourceProperties, Generics::NodeId& linkPoint) :
			    m_Streaming(Generics::NodeId ("0.0.0.0", RTPSink::StreamingPort()), MAXIMUM_PACKAGE_SIZE),
				m_RealSink(m_Streaming, sourceProperties, MAXIMUM_PACKAGE_SIZE)
			{
				if (SINKTYPE == Sink::RTP_UDP)
				{
					m_Streaming.RemoteNode (linkPoint);
					m_Streaming.Open (1);
				}
				if (SINKTYPE == Sink::RTP_TCP)
				{
					m_Streaming.Open (linkPoint, 0);
				}

				m_RealSink.RemoteNode(linkPoint);

				// Now give the link point the location to what it is linked !!!
				linkPoint = m_Streaming.LocalNode();
			}
			/* virtual */ ~RTPSinkType()
			{
				m_Streaming.Close (0);
			}

		public:
			// ------------------------------------------------------------------------------------
			// The next three methods are entry points for the SinkFactoryType template that 
			// implements the IChannelFactory interface. 
			// ------------------------------------------------------------------------------------
			static bool Supported(const Source::ISource* source)
			{
				return (RTPSink::Supported(source));
			}
			static ISink* Create(const Source::ISource* source, Generics::NodeId& linkPoint)
			{
				return (new RTPSinkType<SOCKETTYPE,SINKTYPE> (*(source->Properties()), linkPoint));
			}
			static inline Sink::EnumSinkType Type ()
			{
				return (SINKTYPE);
			}	

			// ------------------------------------------------------------------------------------
			// The next four methods are the implementations for the ISink interface. 
			// ------------------------------------------------------------------------------------
			virtual uint32 Sequence () const
			{
				return (m_RealSink.Sequence());
			}
			virtual uint32 SyncStamp () const
			{
				return (m_RealSink.SyncStamp());
			}
			virtual uint32 SinkSize () const
			{
				return (m_RealSink.FrameSize());
			}
			virtual void Sink (const uint64 presentationTime, Generics::DataElementContainer& frame)
			{
				m_RealSink.Sink(presentationTime, frame);
			}

		private:
			SOCKETTYPE		m_Streaming;
			RTPSink			m_RealSink;
	};

	typedef RTPSinkType<Generics::SocketDatagram, Sink::RTP_UDP>	RTPSinkUDP;
	typedef RTPSinkType<Generics::SocketStream,   Sink::RTP_TCP>	RTPSinkTCP;

} } // namespace Solutions::Sink

#endif // __RTPSINK_H
