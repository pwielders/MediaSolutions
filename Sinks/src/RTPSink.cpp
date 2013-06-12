#include "RTPSink.h"
#include "RTPAdministrator.h"

namespace Solutions { namespace Sink
{

// -------------------------------------------------------------------------------------------------------------------------------
// RTP used static tables, only to be used within this source file.
// -------------------------------------------------------------------------------------------------------------------------------

static void NoPayloadPreparation (RTP::Message& /* header */, const Source::IProperties& /* info */)
{
}
	
static void MPEGAudio (RTP::Message& header, const Source::IProperties& /* info */)
{
	static uint32				  HeaderInfo = 0;
	static Generics::DataElement  Wrapper(sizeof(HeaderInfo), reinterpret_cast<uint8*> (&HeaderInfo));

	// Plain audio header extension, insert all zeros..
	header.Extension(Wrapper);
}

static void MPEGVideo (RTP::Message& header, const Source::IProperties& info)
{
	static uint32					HeaderInfo = 0;
	static Generics::DataElement	Wrapper(sizeof(HeaderInfo), reinterpret_cast<uint8*> (&HeaderInfo));
	const Source::IVideoProperties&	videoInfo = static_cast<const Source::IVideoProperties&>(info); 

	// Now we have the video properties, fill it according to the RFC specification

	// Plain audio header extension, insert all zeros..
	header.Extension(Wrapper);
}

// -------------------------------------------------------------------------------------------------------------------------------
// The port uses for RTP is Odd Even next to each other for Streaming (Even) and for Control +1 (ODD)
// -------------------------------------------------------------------------------------------------------------------------------
const uint16 START_RTP_PORTRANGE  = 6970;
/* static */ uint16	RTPSink::m_LastPortUsed = START_RTP_PORTRANGE;

// -------------------------------------------------------------------------------------------------------------------------------
// RTP used static tables, only to be used within this source file.
// -------------------------------------------------------------------------------------------------------------------------------
struct PayLoadType
{
	Source::EnumSourceType						SourceType;
	Source::EnumStreamType						StreamType;
	uint8										Identifier;
	const TCHAR*								Name;
	unsigned int								Frequency;
	unsigned int								Channels;

	//proprietary handlers fro additional data
	void (*PayloadPreparation) (RTP::Message& header, const Source::IProperties& info);
};

static struct PayLoadType g_PayLoadTable[] = 
{
	{ Source::UNKNOWN,		Source::AUDIO,	   0, _T("PCMU"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   2, _T("G726-32"),  8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   3, _T("PCMU"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   4, _T("GSM"),      8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   5, _T("DVI4"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   6, _T("DVI4"),    16000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   7, _T("LPC"),      8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   8, _T("PCMA"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	   9, _T("G722"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	  10, _T("L16"),     44100,  2, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	  11, _T("L16"),     44100,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::AUDIO,	  12, _T("QCELP"),    8000,  1, NoPayloadPreparation },
	{ Source::MPEG_STREAM,	Source::AUDIO,	  14, _T("MPA"),     90000,  1, MPEGAudio }, // 'number of channels' is actually encoded in the media stream
	{ Source::UNKNOWN,		Source::VIDEO,	  15, _T("G728"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  16, _T("DVI4"),    11025,  2, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  17, _T("DVI4"),    22050,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  18, _T("G729"),     8000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  25, _T("CELB"),    90000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  26, _T("JPEG"),    90000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  28, _T("NV"),      90000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  31, _T("H261"),    90000,  1, NoPayloadPreparation },
	{ Source::MPEG_STREAM,	Source::VIDEO,	  32, _T("MPV"),     90000,  1, NoPayloadPreparation },
	{ Source::MPEG_PACKAGES,Source::VIDEO,	  33, _T("MP2T"),    90000,  1, NoPayloadPreparation },
	{ Source::UNKNOWN,		Source::VIDEO,	  34, _T("H263"),    90000,  1, NoPayloadPreparation }
};

static struct PayLoadType* GetProtocolInfo (const Source::EnumSourceType sourceType, const Source::EnumStreamType streamType)
{
	struct PayLoadType* result = NULL;
	unsigned int index = 0;

	while ((index < (sizeof(g_PayLoadTable)/ sizeof(struct PayLoadType))) && !((g_PayLoadTable[index].SourceType == sourceType) && (g_PayLoadTable[index].StreamType == streamType)))
	{
		index++;
	}

	if (index < (sizeof(g_PayLoadTable)/sizeof(struct PayLoadType)))
	{
		result = &(g_PayLoadTable[index]);
	}

	return (result);
}

// ------------------------------------------------------------------------------------------------------
// Description: Active instances are realised by using threadpools or worker dispatchser. In case the
//              need for a rental thread can be indicated by a synchronisation event, the class 
//              SynchronisationDispatcherType<ELEMENT> is used. In case it is just a matter of 
//              "requesting" a rental thread, the ThreadPoolType<ELEMENT> class is used. If we require
//              a timed dispatch, the TimerType<ELEMENT> is used.
//				As these are all template class, the following methods are required to be implemented 
//              by the templated argument class:
//
//				class SynchronisationDispatcherType<ELEMENT>:
//					SYSTEM_SYNC_HANDLE ELEMENT::SynchronisationHandle();
//					void ELEMENT::Process();
//
//				class ThreadPoolType<ELEMENT>:
//					void ELEMENT::Process();
//
//				class TimerType<ELEMENT>:
//					void ELEMENT::Timed();
// ------------------------------------------------------------------------------------------------------
// ACTIVE INSTANCES: class StreamProcessor
// ------------------------------------------------------------------------------------------------------
class StreamProcessor : public Generics::SingletonType<StreamProcessor>
{
	friend class  Generics::SingletonType<StreamProcessor>;

	protected:
		StreamProcessor() :
			 m_Processor(_T("RTPMonitor"))
		{
		}
		~StreamProcessor()
		{
		}

	public:
		inline void AddMedia(RTPSink::RTPMedia& entry)
		{
			Generics::SynchronisationContextType<RTPSink::RTPMedia*> newEntry (&entry);
			m_Processor.AddSynchronization(newEntry);
		}

		inline void RemoveMedia(RTPSink::RTPMedia& entry)
		{
			Generics::SynchronisationContextType<RTPSink::RTPMedia*> newEntry (&entry);
			m_Processor.RemoveSynchronization(newEntry);
		}

	private:
		Generics::SynchronizationDispatcherType<Generics::SynchronisationContextType<RTPSink::RTPMedia*>>	m_Processor;
};

// ------------------------------------------------------------------------------------------------------
// Description: Active instances are realised by using threadpools or worker dispatchser. In case the
//              need for a rental thread can be indicated by a synchronisation event, the class 
//              SynchronisationDispatcherType<ELEMENT> is used. In case it is just a matter of 
//              "requesting" a rental thread, the ThreadPoolType<ELEMENT> class is used. If we require
//              a timed dispatch, the TimerType<ELEMENT> is used.
//				As these are all template class, the following methods are required to be implemented 
//              by the templated argument class:
//
//				class SynchronisationDispatcherType<ELEMENT>:
//					SYSTEM_SYNC_HANDLE ELEMENT::SynchronisationHandle();
//					void ELEMENT::Process();
//
//				class ThreadPoolType<ELEMENT>:
//					void ELEMENT::Process();
//
//				class TimerType<ELEMENT>:
//					void ELEMENT::Timed();
// ------------------------------------------------------------------------------------------------------
// ACTIVE INSTANCES: class ControlProcessor
// ------------------------------------------------------------------------------------------------------
class ControlProcessor : public Generics::SingletonType<ControlProcessor>
{
	friend class Generics::SingletonType<ControlProcessor>;

	private:
		class Worker : public Generics::SynchronizationDispatcherType<Generics::SynchronisationContextType<RTPSink::RTCPMedia*>>
		{
			private:
				Worker(const Worker&);
				Worker& operator= (const Worker&);

			public:
				Worker (const TCHAR* threadName) : 
					Generics::SynchronizationDispatcherType<Generics::SynchronisationContextType<RTPSink::RTCPMedia*>>(threadName)
				{
				}
				~Worker()
				{
				}

			protected:
				virtual void Handle (RTPSink::RTCPMedia& element)
				{
					element.Process();
				}
		};	
	
	protected:
		ControlProcessor() :
			 m_Processor(_T("RTCPMonitor")),
			 m_TimedProcessor(_T("RTCPTimer"))
		{
		}
		~ControlProcessor()
		{
		}

	public:
		inline void AddMedia(RTPSink::RTCPMedia& entry)
		{
			Generics::SynchronisationContextType<RTPSink::RTCPMedia*> newEntry (&entry);

			m_Processor.AddSynchronization(newEntry);
		}

		inline void RemoveMedia(RTPSink::RTCPMedia& entry)
		{
			m_Processor.RemoveSynchronization(Generics::SynchronisationContextType<RTPSink::RTCPMedia*>(&entry));
			m_TimedProcessor.Revoke(Generics::TimerContextType<RTPSink::RTCPMedia*>(&entry));
		}

		void Schedule (RTPSink::RTCPMedia& channelSink, const uint64 nextDrop)
		{
			Generics::TimerContextType<RTPSink::RTCPMedia*> newJob (&channelSink);

			m_TimedProcessor.Schedule(nextDrop, newJob);
		}

	private:
		Worker																	m_Processor;
		Generics::TimerType<Generics::TimerContextType<RTPSink::RTCPMedia*>>	m_TimedProcessor;
};

// ------------------------------------------------------------------------------------------------------
// class RTPSink::Info
// ------------------------------------------------------------------------------------------------------
RTPSink::Info::Info(const Source::IProperties& sourceProperties) : 
	m_PayLoad(NULL),
	m_Frequency(8), // In Hz/mS
	m_SendFrames(0),
	m_DroppedFrames(0),
	m_SendBytes(0),
	m_DroppedBytes(0),
	m_RTPBaseTime(Generics::RangeType<uint32, true, true>(0, NUMBER_MAX_UNSIGNED(uint32)).Random()),
	m_SynchronisationSource(Generics::RangeType<uint32, true, true>(0, NUMBER_MAX_UNSIGNED(uint32)).Random()),
	m_RTPPlayTime(0),
	m_CollectionStartTime(Generics::Time::Now().Ticks()),
	m_SourceProperties(sourceProperties)
{
	// Get the RTP parameters from the type of stream identifier.
	struct PayLoadType* info = GetProtocolInfo(sourceProperties.SourceType(),sourceProperties.StreamType());

	if (info != NULL)
	{
		m_PayLoad = info;
		m_Frequency = (info->Frequency / 1000); // should be in Hz/ms
	}
}

RTPSink::Info::~Info()
{
}

void RTPSink::Info::PayloadPreparation (RTP::Message& header)
{
	m_PayLoad->PayloadPreparation(header, m_SourceProperties);
}

uint8 RTPSink::Info::PayLoadType() const
{
	return (m_PayLoad->Identifier);
}

void RTPSink::Info::SynchronisationSource(uint32 newSynchronisationSource) const
{
	m_SynchronisationSource = newSynchronisationSource;
	m_SendFrames = 0;
	m_DroppedFrames = 0;
	m_SendBytes = 0;
	m_DroppedBytes = 0;
	m_CollectionStartTime = Generics::Time::Now().Ticks();
}

uint32 RTPSink::Info::EstimatedBandwidth() const
{
	// If bandwidth can be calculates, do it, otherwise make a guess at 500 kbps.
	uint32 result = (500 * 1024);

	// Calcualte the number of seconds that have passed since we are collecting info
	uint64 collectionTime = (Generics::Time::Now().Ticks() - m_CollectionStartTime) / (TICKS_PER_MILLISECONDS * 1000);

	if (collectionTime != 0)
	{
		uint64 bitsSend = 8 * m_SendBytes;
		result = static_cast<uint32>(bitsSend / collectionTime);
	}

	return (result);
}

// ------------------------------------------------------------------------------------------------------
// class RTPSink::RTCPMedia
// ------------------------------------------------------------------------------------------------------
/*****

A.7 Computing the RTCP Transmission Interval

   The following functions implement the RTCP transmission and reception
   rules described in Section 6.2. These rules are coded in several
   functions:

       o  rtcp_interval() computes the deterministic calculated
          interval, measured in seconds.  The parameters are defined in
          Section 6.3.

       o  OnExpire() is called when the RTCP transmission timer expires.

       o  OnReceive() is called whenever an RTCP packet is received.

   Both OnExpire() and OnReceive() have event e as an argument. This is
   the next scheduled event for that participant, either an RTCP report
   or a BYE packet.  It is assumed that the following functions are
   available:

       o  Schedule(time t, event e) schedules an event e to occur at
          time t. When time t arrives, the function OnExpire is called
          with e as an argument.

       o  Reschedule(time t, event e) reschedules a previously scheduled
          event e for time t.

       o  SendRTCPReport(event e) sends an RTCP report.

       o  SendBYEPacket(event e) sends a BYE packet.

       o  TypeOfEvent(event e) returns EVENT_BYE if the event being
          processed is for a BYE packet to be sent, else it returns
          EVENT_REPORT.

       o  PacketType(p) returns PACKET_RTCP_REPORT if packet p is an
          RTCP report (not BYE), PACKET_BYE if its a BYE RTCP packet,
          and PACKET_RTP if its a regular RTP data packet.

       o  ReceivedPacketSize() and SentPacketSize() return the size of
          the referenced packet in octets.

       o  NewMember(p) returns a 1 if the participant who sent packet p
          is not currently in the member list, 0 otherwise. Note this
          function is not sufficient for a complete implementation
          because each CSRC identifier in an RTP packet and each SSRC in
          a BYE packet should be processed.

       o  NewSender(p) returns a 1 if the participant who sent packet p
          is not currently in the sender sublist of the member list, 0
          otherwise.

       o  AddMember() and RemoveMember() to add and remove participants
          from the member l
       o  AddSender() and RemoveSender() to add and remove participants
          from the sender sublist of the member list.
*****/

/*
* Minimum average time between RTCP packets from this site (in
* seconds).  This time prevents the reports from `clumping' when
* sessions are small and the law of large numbers isn't helping
* to smooth out the traffic.  It also keeps the report interval
* from becoming ridiculously small during transient outages like
* a network partition.
*/
double const RTCP_MIN_TIME = 5.;
/*
* Fraction of the RTCP bandwidth to be shared among active
* senders.  (This fraction was chosen so that in a typical
* session with one or two active senders, the computed report
* time would be roughly equal to the minimum report time so that
* we don't unnecessarily slow down receiver reports.) The
* receiver fraction must be 1 - the sender fraction.
*/
double const RTCP_SENDER_BW_FRACTION = 0.25;
double const RTCP_RCVR_BW_FRACTION = (1-RTCP_SENDER_BW_FRACTION);
/*
* To compensate for "unconditional reconsideration" converging to a
* value below the intended average.
*/
double const COMPENSATION = 2.71828 - 1.5;

#pragma warning (disable :4355)
RTPSink::RTCPMedia::RTCPMedia(Info& info, const uint16 portNumber) : 
	m_Info(info),
	m_Socket(Generics::NodeId(_T("0.0.0.0"), portNumber)),
	m_Sender(&m_Socket),
	m_Message(),
	m_LastTrigger(Generics::Time::Now().Ticks()),
	m_MemberCount(0)
{
	m_Socket.Open(1);

	ControlProcessor::Instance().AddMedia(*this);

	m_MemberCount = RTPAdministrator::Instance().MemberCount();
	uint32 deltaTime = NextInterval(true, m_Info.EstimatedBandwidth(), true);
	m_LastTrigger += (TICKS_PER_MILLISECONDS * static_cast<uint64>(deltaTime));
}
#pragma warning (default :4355)

RTPSink::RTCPMedia::~RTCPMedia()
{
	ControlProcessor::Instance().RemoveMedia(*this);
	m_Socket.Close(0);
}

uint32 RTPSink::RTCPMedia::NextInterval (const bool didWeSend, const uint32 estimatedBandwidth, const bool initialSetting) const
{
	// We only want the RTCP bandwith here 5% of the RTP bandwidth
	double rtcp_bw = ((static_cast<double>(estimatedBandwidth) + 1) / 20.);

	uint32 members = m_MemberCount; 
	uint32 senders = RTPAdministrator::Instance().SenderCount();

	/*
	* Minimum average time between RTCP packets from this site (in
	* seconds).  This time prevents the reports from `clumping' when
	* sessions are small and the law of large numbers isn't helping
	* to smooth out the traffic.  It also keeps the report interval
	* from becoming ridiculously small during transient outages like
	* a network partition.
	*/
	double const RTCP_MIN_TIME = 5.;
	/*
	* Fraction of the RTCP bandwidth to be shared among active
	* senders.  (This fraction was chosen so that in a typical
	* session with one or two active senders, the computed report
	* time would be roughly equal to the minimum report time so that
	* we don't unnecessarily slow down receiver reports.) The
	* receiver fraction must be 1 - the sender fraction.
	*/
	double const RTCP_SENDER_BW_FRACTION = 0.25;
	double const RTCP_RCVR_BW_FRACTION = (1-RTCP_SENDER_BW_FRACTION);
	/*
	* To compensate for "unconditional reconsideration" converging to a
	* value below the intended average.
	*/
	double const COMPENSATION = 2.71828 - 1.5;

	double t;                   /* interval */
	double rtcp_min_time = RTCP_MIN_TIME;
	int n;                      /* no. of members for computation */

	/*
	* Very first call at application start-up uses half the min
	* delay for quicker notification while still allowing some time
	* before reporting for randomization and to learn about other
	* sources so the report interval will converge to the correct
	* interval more quickly.
	*/
	if (initialSetting)
	{
	   rtcp_min_time /= 2;
	}

	/*
	* If there were active senders, give them at least a minimum
	* share of the RTCP bandwidth.  Otherwise all participants share
	* the RTCP bandwidth equally.
	*/
	n = members;
	if (senders > 0 && senders < members * RTCP_SENDER_BW_FRACTION) 
	{
	   if (didWeSend) {
		   rtcp_bw *= RTCP_SENDER_BW_FRACTION;
		   n = senders;
	   } else {
		   rtcp_bw *= RTCP_RCVR_BW_FRACTION;
		   n -= senders;
	   }
	}

	/*
	* The effective number of sites times the average packet size is
	* the total number of octets sent when each site sends a report.
	* Dividing this by the effective bandwidth gives the time
	* interval over which those packets must be sent in order to
	* meet the bandwidth target, with a minimum enforced.  In that
	* time interval we send one report so this time is also our
	* average time between reports.
	*/
	t = RTPAdministrator::Instance().AveragePackageSize() * n / rtcp_bw;
	if (t < rtcp_min_time) t = rtcp_min_time;

	/*
	* To avoid traffic bursts from unintended synchronization with
	* other sites, we then pick our actual next report interval as a
	* random number uniformly distributed between 0.5*t and 1.5*t.
	*/
	 // a random 30-bit integer
	Generics::RangeType<uint32, true, true> range (0, 0x3FFFFFFF); 

	t = t * ((static_cast<double>(range.Random())/(1024*1024*1024)) + 0.5);
	t = t / COMPENSATION;

	// t moust be translated to milliseconds to delay..
	return (static_cast<uint32>(t * 1000));
}

void RTPSink::RTCPMedia::Process ()
{
	uint8 bits = m_Socket.PendingTriggers();

	// If we receive something on this socket, trash it. No idea what to do with it ;-)
	if ((bits & Generics::SocketPort::RECEIVE) == Generics::SocketPort::RECEIVE)
	{
		uint8	buffer[1024];
		uint32	bufferSize = sizeof(buffer);

		while ((m_Sender.Media()->Receive(bufferSize, buffer, 0) == OK) && (bufferSize == 0))
		{
			// Seems like we need to do another run, reload 
			bufferSize = sizeof(buffer);
		}
	}

	// If the send buffer is empty, maybe we can send something else ?
	if ((bits & Generics::SocketPort::SEND) == Generics::SocketPort::SEND)
	{
		m_Sender.Send();
	}
}

void RTPSink::RTCPMedia::Timed ()
{
	m_MemberCount = RTPAdministrator::Instance().MemberCount();
	uint64 nextDelta = (TICKS_PER_MILLISECONDS * static_cast<uint64>(NextInterval(true, m_Info.EstimatedBandwidth(), false)));
	m_LastTrigger += nextDelta;

	// Is the next time in the future ?
	if (m_LastTrigger < Generics::Time::Now().Ticks())
	{
		// No it is already passed, get a new time in the future.
		m_LastTrigger = Generics::Time::Now().Ticks();
		m_LastTrigger += nextDelta;
	}

	// Schedule the next drop...
	ControlProcessor::Instance().Schedule (*this, m_LastTrigger);

	// Now send out a RTCP report !!!
	if (m_Sender.Send() == true)
	{
		//if (m_Sender)
		//{
			Generics::Time currentTime = Generics::Time::Now();
			m_Message.IsSenderReport(true);
			m_Message.SynchronisationSource(m_Info.SynchronisationSource());
			m_Message.NTPTimeStamp(currentTime.NTPTime());
			m_Message.RTPTimeStamp(m_Info.RTPTime());
			m_Message.PackageCount(m_Info.SendFrames());
			m_Message.OctetCount(m_Info.SendBytes());

			//TODO:  In the future we might add receiver reports !!!
		//}
		//else
		//{
		//	m_Message.IsReceiverReport(true);
		//	//TODO:  In the future we might add receiver reports !!!
		//}

		//TRACE(TraceSessionFlow, (_T("ControlMessage::Send")));

		// Send out that 1 package.
		m_Sender.Send(m_Message.Data());
	}
}
// ------------------------------------------------------------------------------------------------------
// class RTPSink::RTPMedia
// ------------------------------------------------------------------------------------------------------
#pragma warning (disable :4355)
RTPSink::RTPMedia::RTPMedia(Info& info, Generics::SocketPort& port) : 
	m_Info(info),
	m_Sender(&port)
{
}
#pragma warning (default :4355)

RTPSink::RTPMedia::~RTPMedia()
{
}

void RTPSink::RTPMedia::Process ()
{
	uint8 bits = m_Sender.Media()->PendingTriggers();

	// If we receive something on this socket, trash it. No idea what to do with it ;-)
	if ((bits & Generics::SocketPort::RECEIVE) == Generics::SocketPort::RECEIVE)
	{
		uint8	buffer[1024];
		uint32	bufferSize = sizeof(buffer);

		while ((m_Sender.Media()->Receive(bufferSize, buffer, 0) == OK) && (bufferSize == 0))
		{
			// Seems like we need to do another run, reload 
			bufferSize = sizeof(buffer);
		}
	}

	// If the send buffer is empty, maybe we can send something else ?
	if ((bits & Generics::SocketPort::SEND) == Generics::SocketPort::SEND)
	{
		// Well see if we need to send anything that is left !
		m_Sender.Send();
	}
}

// ------------------------------------------------------------------------------------------------------
// class RTPSink
// ------------------------------------------------------------------------------------------------------
#pragma warning (disable :4355)
RTPSink::RTPSink(Generics::SocketPort& stream, const Source::IProperties& sourceProperties, const uint32 bufferSize) :
	m_Info(sourceProperties),
	m_StreamSink(m_Info, stream),
	m_ControlSink(m_Info, stream.LocalNode().PortNumber() + 1),
	m_SequenceNumber(Generics::RangeType<uint16, true, true>(0, static_cast<uint16>(~0)).Random()), // Just get a random 16 bit number to start with.. 
	m_RTPHeader(RTP_VERSION),
	m_AdminLock()
{
	m_LastPortUsed += 2;
	//TODO: Make the buffer size dependent on the type of transfer (m_Info.PayLoadType())
	m_BufferSize = bufferSize - m_RTPHeader.HeaderSize();

	StreamProcessor::Instance().AddMedia(m_StreamSink);
	ControlProcessor::Instance().Schedule (m_ControlSink, m_ControlSink.NextTrigger());
	RTPAdministrator::Instance().AddMedia(*this);

}

#pragma warning (default :4355)

/* virtual */ RTPSink::~RTPSink()
{
	RTPAdministrator::Instance().RemoveMedia(*this);
	ControlProcessor::Instance().RemoveMedia(m_ControlSink);
	StreamProcessor::Instance().RemoveMedia(m_StreamSink);
}

// --------------------------------------------------------------------------------------
// The next method is called by the template instantiation of IChannelSinkFactory.
// --------------------------------------------------------------------------------------
/* static */ bool RTPSink::Supported(const Source::ISource* source)
{
	bool supported = false;

	Source::EnumSourceType typeOfStream = source->Properties()->SourceType();

	// See if this source is in the RTP supported sources table.
	PayLoadType* info = GetProtocolInfo(source->Properties()->SourceType(), source->Properties()->StreamType());

	if (info != NULL)
	{
		switch (typeOfStream)
		{
			case Source::MPEG_STREAM:
			{
				supported = true;
				break;
			}
			case Source::AVI_MEDIA:
			{
				supported = true;
				break;
			}
			default:
			{
				break;
			}
		}
	}

	return (supported);
}

void RTPSink::Sink (const uint64 presentationTime, Generics::DataElementContainer& frame)
{
	// Check if we are not busy..
	m_AdminLock.Lock();

	// Build a HeaderFrame and Calculate the time relative to the frequency and round it
	m_RTPHeader.TimeStamp(m_Info.RTPTime(presentationTime));
	m_RTPHeader.SessionSource (m_Info.SynchronisationSource());
	m_RTPHeader.Sequence (m_SequenceNumber++);
	m_RTPHeader.PayLoadType(m_Info.PayLoadType());

	// Add the extension, if required to RTPHeader
	m_Info.PayloadPreparation(m_RTPHeader);
		
	// Insert the header into the package..
	frame.Front(m_RTPHeader.Data());

	// See if we can send (is a slot available ?)
	if (m_StreamSink.Send(frame) == true)
	{
		m_Info.SendFrame (static_cast<uint32>(frame.Size()));
	}
	else
	{
		m_Info.DroppedFrame(static_cast<uint32>(frame.Size()));
	}

	m_AdminLock.Unlock();
}

} } // namespace Solutions::Sink
