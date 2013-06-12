#include "RTPAdministrator.h"

namespace Solutions { namespace Sink
{

void RTPAdministrator::CleanupWork::Timed()
{
	RTPAdministrator::Instance().CleanupSources();
}

RTPAdministrator::RecipientSession::RecipientSession() :
	m_Message(),
	m_Aging(0)
{
}

RTPAdministrator::RecipientSession::RecipientSession(const RecipientSession& copy) :
	m_Message(copy.m_Message),
	m_Aging(copy.m_Aging)
{
}

RTPAdministrator::RecipientSession::~RecipientSession()
{
}

uint32 RTPAdministrator::RecipientSession::IncrementAgingCount ()
{
	uint32 result  = m_Aging;

	++m_Aging;

	return (result);
}

void RTPAdministrator::RecipientSession::Update(const RTP::Message& /* info */)
{
	m_Aging = 0;
}

void RTPAdministrator::RecipientSession::Update(const RTCP::Message::ReportBlock& info)
{
	m_Aging = 0;
	m_Message = info.Data();
}

const RTCP::Message::ReportBlock& RTPAdministrator::RecipientSession::Message() const
{
	return (m_Message);
}

// -----------------------------------------------------------------------------------------------
// class RTPAdministrator::RTPAdministrator
// -----------------------------------------------------------------------------------------------
RTPAdministrator::RTPAdministrator() :
	m_Admin(),
	m_ReportingSources(),
	m_RemoteSenders(),
	m_LocalSenders(),
	m_AverageRTCPSize(1),
	m_CleanupTask(_T("RTCPCleanupProcess"))
{
}

RTPAdministrator::~RTPAdministrator()
{
}

double RTPAdministrator::AveragePackageSize() const
{
	return (m_AverageRTCPSize);
}

void RTPAdministrator::AddMedia (RTPSink& channel)
{
	if (channel.IsSender())
	{
		std::list<RTPSink*>::iterator index = std::find(m_LocalSenders.begin(), m_LocalSenders.end(), &channel);

		ASSERT (index == m_LocalSenders.end());

		m_LocalSenders.push_back(&channel);
	}
	else
	{
		// TOD: Implement client side, if applicable ;-)
		ASSERT (false);
	}
}

void RTPAdministrator::RemoveMedia (const RTPSink& channel)
{
	if (channel.IsSender())
	{
		std::list<RTPSink*>::iterator index = std::find(m_LocalSenders.begin(), m_LocalSenders.end(), &channel);

		if (index != m_LocalSenders.end())
		{
			m_LocalSenders.erase (index);
		}
	}
	else
	{
		// TOD: Implement client side, if applicable ;-)
		ASSERT (false);
	}
}

bool RTPAdministrator::IsMember (const uint32 syncSource) const
{
	bool result = (m_ReportingSources.find(syncSource) != m_ReportingSources.end());

	if (result == false)
	{
		std::list<RTPSink*>::const_iterator index = m_LocalSenders.begin();
		// Check the local senders list
		while ( (index != m_LocalSenders.end()) && ((*index)->SynchronisationSource() != syncSource) )
		{
			index++;
		}
		result = (index != m_LocalSenders.end());
	}

	return (result);
}

void RTPAdministrator::AnalysePackage (const RTCP::Message& report)
{
	m_Admin.Lock ();

	m_AverageRTCPSize = (1./16.) * report.Size() + (15./16.) * (m_AverageRTCPSize);

	if (report.IsSenderReport() || report.IsReceiverReport())
	{
		if (report.IsSenderReport())
		{
			AddSenderSource(report.SynchronisationSource(), RTP::Message(RTP_VERSION));
		}

		RTCP::Message::ReportBlock::Iterator index = report.Reports ();

		// Check the reports, they might be all members..
		while (index.Next() == true)
		{
			// Add this report..
			m_ReportingSources[index.Value().SynchronisationSource()].Update(index.Value());
		}
	}
	else if (report.IsBye())
	{
		RemoveSource (report.SynchronisationSource());
	}

	// Check if the timer is scheduled to run..
	if (m_CleanupTask.NextTrigger() == ~0)
	{
		m_CleanupTask.Schedule(Generics::Time::Now().Ticks() + (PERIODIC_CLEANUP_TIME * 1000 * TICKS_PER_MILLISECONDS), CleanupWork());
	}

	m_Admin.Unlock ();
}

// Only for remote senders, local sender report them selves via the AddServer i/f
void RTPAdministrator::AnalysePackage (const RTP::Message& message)
{
	m_Admin.Lock ();

	AddSenderSource(message.SessionSource(), message);

	m_Admin.Unlock ();
}

uint32 RTPAdministrator::MemberCount () const
{
	return (m_ReportingSources.size() + m_LocalSenders.size());
}

uint32 RTPAdministrator::SenderCount () const
{
	return (m_RemoteSenders.size() + m_LocalSenders.size());
}

void RTPAdministrator::RemoveSource (const uint32 syncSource)
{
	std::map<uint32,RecipientSession>::iterator index = m_ReportingSources.find(syncSource);

	if (index != m_ReportingSources.end())
	{
		// Someone wants to leave, update DB accordingly..
		m_ReportingSources.erase(index);

		std::list<uint32>::iterator senderIndex = std::find(m_RemoteSenders.begin(), m_RemoteSenders.end(), index->second.Message().SynchronisationSource());

		if (senderIndex != m_RemoteSenders.end())
		{
			m_RemoteSenders.erase(senderIndex);
		}
	}
}

void RTPAdministrator::AddSenderSource (const uint32 syncSource, const RTP::Message& message)
{
	// Maybe we found a new sender ?
	if (m_ReportingSources.find(syncSource) != m_ReportingSources.end())
	{
		// Here we might check the local servers in future releases to
		// upgrade/renew the syncSource of the local server as it might
		// conflict !!!!

		// We have a new sender/member.
		m_ReportingSources[syncSource].Update(message);
		m_RemoteSenders.push_back(syncSource);
	}					
}

void RTPAdministrator::CleanupSources ()
{
	m_Admin.Lock();

	std::map<uint32, RecipientSession>::iterator index = m_ReportingSources.begin();

	while (index != m_ReportingSources.end())
	{
		// Check if it duid not age. If so, kill it....
		if (index->second.IncrementAgingCount() > 3) // 15 minute time out
		{
			index = m_ReportingSources.erase(index);

			std::list<uint32>::iterator senderIndex = std::find(m_RemoteSenders.begin(), m_RemoteSenders.end(), index->second.Message().SynchronisationSource());

			if (senderIndex != m_RemoteSenders.end())
			{
				m_RemoteSenders.erase(senderIndex);
			}
		}
		else
		{
			++index;
		}
	}

	if (m_ReportingSources.size() > 0)
	{
		m_CleanupTask.Schedule(Generics::Time::Now().Ticks() + (PERIODIC_CLEANUP_TIME * 1000 * TICKS_PER_MILLISECONDS), CleanupWork());
	}

	m_Admin.Unlock();
}

} } // namespace Solutions::Sink
