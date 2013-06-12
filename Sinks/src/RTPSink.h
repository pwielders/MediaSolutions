#ifndef __RTPMEDIASINK_H
#define __RTPMEDIASINK_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sink
{
	const uint32 MAXIMUM_PACKAGE_SIZE = 1500; // Link to the MTU size..

	// Forward struct declaration.
	struct PayLoadType;

	class EXTERNAL RTPSink
	{
		friend class StreamProcessor;
		friend class ControlProcessor;

		public:
			class EXTERNAL Info
			{
				private:
					Info (const Info&);
					Info& operator= (const Info&);

				public:
					Info(const Source::IProperties& sourceProperties);
					~Info();

				public:
					uint8 PayLoadType() const;
					uint32 EstimatedBandwidth() const;
					void SynchronisationSource(uint32 newSynchronisationSource) const;

					inline uint32 RTPTime() const
					{
						return (m_RTPBaseTime + m_CurrentRTPTime);
					}
					inline uint32 RTPTime(const uint64 presentationTime)
					{
						m_CurrentRTPTime =  static_cast<uint32>(presentationTime * m_Frequency); // m_Frequencey is in mS

						return (RTPTime());
					}
					inline uint32 SynchronisationSource() const
					{
						return (m_SynchronisationSource);
					}
					inline uint32 SendFrames() const
					{
						return (m_SendFrames);
					}
					inline uint32 SendBytes() const
					{
						return (m_SendBytes);
					}
					inline uint32 DroppedFrames() const
					{
						return (m_DroppedFrames);
					}
					inline uint32 DroppedBytes() const
					{
						return (m_DroppedBytes);
					}
					inline void SendFrame(const uint32 frameSize)
					{
						m_SendFrames++;
						m_SendBytes += frameSize;
					}
					inline void DroppedFrame(const uint32 frameSize)
					{
						m_DroppedFrames++;
						m_DroppedBytes += frameSize;
					}
					void PayloadPreparation (RTP::Message& header);

				private:
					struct PayLoadType*			m_PayLoad;
					uint32						m_Frequency;
					uint32						m_RTPBaseTime;
					uint32						m_CurrentRTPTime;
					mutable uint64				m_CollectionStartTime;
					mutable uint32				m_SendFrames;
					mutable uint32				m_DroppedFrames;
					mutable uint32				m_SendBytes;
					mutable uint32				m_DroppedBytes;
					mutable uint32				m_SynchronisationSource;
					mutable uint32				m_RTPPlayTime;
					const Source::IProperties&	m_SourceProperties;
			};

		private:
			class EXTERNAL RTPMedia
			{
				private:
					RTPMedia ();
					RTPMedia (const RTPMedia&);
					RTPMedia& operator= (const RTPMedia&);

				public:
					RTPMedia(Info& info, Generics::SocketPort& port);
					~RTPMedia();

				public:
					inline SYSTEM_SYNC_HANDLE SynchronisationHandle ()
					{
						return (m_Sender.Media()->SynchronisationHandle());
					}

					inline bool Send (const Generics::DataElementContainer& data)
					{
						return (m_Sender.Send(data));
					}

					void Process ();

				private:
					Info&						m_Info;
					Generics::MediaSender		m_Sender;
			};

			class EXTERNAL RTCPMedia
			{
				private:
					RTCPMedia ();
					RTCPMedia (const RTCPMedia&);
					RTCPMedia& operator= (const RTCPMedia&);

				public:
					RTCPMedia(Info& info, const uint16 portNumber);
					~RTCPMedia();

				public:
					inline uint64 NextTrigger () const
					{
						return (m_LastTrigger);
					}

					inline void RemoteNode (const Generics::NodeId& remoteNode)
					{
						m_Socket.RemoteNode (remoteNode);
					}

					inline const Generics::NodeId& RemoteNode () const
					{
						return (m_Socket.RemoteNode ());
					}

					inline SYSTEM_SYNC_HANDLE SynchronisationHandle ()
					{
						return (m_Socket.SynchronisationHandle());
					}

					// ----------------------------------------------
					// Entry points for threads
					// ----------------------------------------------
					void Process ();
					void Timed ();

				private:
					uint32 NextInterval (const bool didWeSend, const uint32 estimatedBandwidth, const bool initialSetting) const;

				private:
					Info&												m_Info;
					Generics::SocketDatagram							m_Socket;
					Generics::MediaSender								m_Sender;
					RTCP::Message										m_Message;
					uint64												m_LastTrigger;
					bool												m_RTCPSendReport;
					uint32												m_MemberCount;
			};


		private:
			RTPSink ();
			RTPSink (const RTPSink&);
			RTPSink operator= (const RTPSink&);

		public:
			RTPSink(Generics::SocketPort& stream, const Source::IProperties& sourceProperties, const uint32 bufferSize);
			virtual ~RTPSink();

			// ------------------------------------------------------------------------------------
			// The next method is an entry point for the SinkFactoryType template that implements
			// the IChannelFactory interface. 
			// ------------------------------------------------------------------------------------
			static bool Supported(const Source::ISource* sourceType);

		public:
			inline bool IsSender() const
			{
				return (true);
			}

			inline uint32 SynchronisationSource () const
			{
				return (m_Info.SynchronisationSource());
			}

			inline uint32 Sequence () const
			{
				return (m_SequenceNumber);
			}

			inline uint32 SyncStamp () const
			{
				return (m_Info.RTPTime());
			}

			inline void RemoteNode (const Generics::NodeId& remoteNode)
			{
				Generics::NodeId controlNode (remoteNode);
				controlNode.PortNumber(controlNode.PortNumber() + 1);
				m_ControlSink.RemoteNode (controlNode);
			}
			inline uint32 FrameSize () const
			{
				return (m_BufferSize);
			}
			void Sink (const uint64 presentationTime, Generics::DataElementContainer& frame);

			static uint16 StreamingPort ()
			{
				return ((m_LastPortUsed + 1) & 0xFFF7);
			}

		private:
			static uint16					m_LastPortUsed;
			uint32							m_BufferSize;
			Info							m_Info;
			RTPMedia						m_StreamSink;
			RTCPMedia						m_ControlSink;
			uint16							m_SequenceNumber;
			RTP::Message					m_RTPHeader;
			Generics::CriticalSection		m_AdminLock;
	};

} } // namespace Solutions::Sink

#endif // __RTPMEDIASINK_H
