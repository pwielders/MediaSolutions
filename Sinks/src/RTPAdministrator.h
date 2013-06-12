#ifndef __RTCPADMINISTRATOR_H
#define __RTCPADMINISTRATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "RTPSink.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sink
{
	class EXTERNAL RTPAdministrator : public Generics::SingletonType<RTPAdministrator>
	{
		private:
			static const uint32 PERIODIC_CLEANUP_TIME = (5 * 60);	// Every 5 minutes !!! 

			class EXTERNAL CleanupWork
			{
				public:
					void Timed();
			};

			class EXTERNAL RecipientSession
			{
				private:
					RecipientSession& operator= (const RecipientSession&);

				public:
					RecipientSession();
					RecipientSession(const RecipientSession& copy);
					~RecipientSession();

				public:
					uint32 IncrementAgingCount ();
					void Update  (const RTCP::Message::ReportBlock& info);
					void Update  (const RTP::Message& info);
					const RTCP::Message::ReportBlock& Message() const;

				private:
					RTCP::Message::ReportBlock	m_Message;
					uint16						m_Aging;
			};

		private:
			RTPAdministrator(const RTPAdministrator&);
			RTPAdministrator& operator= (const RTPAdministrator&);

		public:
			RTPAdministrator();
			~RTPAdministrator();

			bool IsMember (const uint32 syncSource) const;
			uint32 MemberCount () const;
			uint32 SenderCount () const;
			double AveragePackageSize() const;

			void AddMedia (RTPSink& channel);
			void RemoveMedia (const RTPSink& channel); 

			void AnalysePackage (const RTCP::Message& message);
			void AnalysePackage (const RTP::Message& header);

		private:
			void AddSenderSource (const uint32 syncSource, const RTP::Message& info);
			void RemoveSource (const uint32 syncSource);
			void CleanupSources ();

		private:
			Generics::CriticalSection			m_Admin;
			std::map<uint32, RecipientSession>	m_ReportingSources;
			std::list<uint32>					m_RemoteSenders;
			std::list<RTPSink*>					m_LocalSenders;
			double								m_AverageRTCPSize;
			Generics::TimerType<CleanupWork>	m_CleanupTask;
	};

} } // namespace Solutions::Sink

#endif // __RTCPADMINISTRATOR_H