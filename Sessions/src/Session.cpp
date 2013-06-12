#include "Session.h"
#include "Channel.h"
#include "SessionStore.h"
#include "TraceCategories.h"

namespace Solutions { namespace Sessions
{

class Processor : public Generics::SingletonType<Processor>
{
	friend class Generics::SingletonType<Processor>;

	private:
		class SessionTimeoutHandler
		{
			private:
				SessionTimeoutHandler();

			public:
				inline SessionTimeoutHandler (Session& content) : m_Content(&content)
				{
				}
				inline SessionTimeoutHandler(const SessionTimeoutHandler& copy) : m_Content (copy.m_Content)
				{
				}
				inline ~SessionTimeoutHandler ()
				{
				}
				inline SessionTimeoutHandler& operator= (const SessionTimeoutHandler& RHS)
				{
					m_Content = RHS.m_Content;

					return (*this);
				}
				inline void Timed()
				{
					// Run the actual job, decoupled......
					Processor::Instance().ScheduleWork(*m_Content);
				}
				inline bool operator== (const SessionTimeoutHandler& RHS) const
				{
					return (m_Content == RHS.m_Content);
				}
				inline bool operator!= (const SessionTimeoutHandler& RHS) const
				{
					return !(operator==(RHS));
				}

			private:
				Session*	m_Content;
		};

	protected:
		Processor () :
		   m_Processor   (_T("SessionProcessor")),
		   m_DelayedWork (_T("SessionTimer"))
		{
		}
		~Processor()
		{
			// Stop handling any new requests..
			m_Processor.Block ();
			m_Processor.Wait (Generics::Thread::BLOCKED, INFINITE);

			// If we kill the processor, make sure all sessions are killed,
			// now they can no longer be server
			SessionStore::Instance().Clear();
		}

	public:
		void ScheduleWork(Session& entry, const uint64 presentationTime = 0)
		{
			if (presentationTime == 0)
			{
				Generics::ThreadContextType<Session*> newJob (&entry);
				m_Processor.Submit(newJob, INFINITE);
			}
			else
			{
				m_DelayedWork.Schedule(presentationTime, SessionTimeoutHandler(entry));
			}
		}

		void RevokeWork(Session& entry)
		{
			m_DelayedWork.Revoke (SessionTimeoutHandler(entry));
			m_Processor.Revoke (Generics::ThreadContextType<Session*>(&entry));
		}

	private:
		Generics::ThreadPoolType<Generics::ThreadContextType<Session*>, 1, ~0>		m_Processor;
		Generics::TimerType<SessionTimeoutHandler>									m_DelayedWork;
};

class ChannelIterator : public Channel::Iterator
{

	private:
		ChannelIterator ();

	public:
		inline ChannelIterator (Channel* root) :
			Channel::Iterator(root)
		{
		}
		inline ChannelIterator (const Iterator& copy) :
			Channel::Iterator(copy)
		{
		}
		~ChannelIterator()
		{
		}

		inline ChannelIterator& operator= (const ChannelIterator& RHS)
		{
			Channel::Iterator::operator=(RHS);

			return (*this);
		}

	public:
		inline uint64 Pause ()
		{
			return (Element()->Pause());
		}
		inline uint64 Play (const uint64 presentationTime)
		{
			return (Element()->Play(presentationTime));
		}
};

/* static */ uint32	Session::m_Sequence = 0;

Session::Session(Generics::ProxyType<Source::ISources>& sources) :
	m_SessionId(),
	m_Medium (sources),
	m_Channels(NULL),
	m_State(INIT),
	m_Admin()
{
	// Let the identifier be generated as the this pointer...
	m_SessionId = Generics::TextFragment(Generics::NumberUnsigned64(reinterpret_cast<uint64>(this)).Text());

	// If we create a session, register if for retrieval later
	Sessions::SessionStore::Instance().Register(this);
}

Session::~Session()
{
	// Make sure there is no ongoing action...
	Deactivate();

	m_Admin.Lock();

	if (m_Channels != NULL)
	{
		delete m_Channels;

		m_Channels = NULL;
	}

	Sessions::SessionStore::Instance().Unregister(this);

	// Relase the adminlock, the fact that channels is NULL prevents any new scheduling.
	m_Admin.Unlock();

}

void Session::Activate()
{
	m_Admin.Lock();

	if ( (m_Channels != NULL) && (m_State == INIT) )
	{
		SessionStore::Instance().Register(this);

		m_State = PAUSED;
	}

	m_Admin.Unlock();
}

void Session::Deactivate()
{
	m_Admin.Lock();

	if ( (m_Channels != NULL) && (m_State != INIT) )
	{
		SessionStore::Instance().Unregister(this);

		m_State = INIT;
	}

	m_Admin.Unlock();

	// Make sure this session is no longer pending on the Processor to do some work..
	Processor::Instance().RevokeWork(*this);
}

void Session::AddChannel (Source::ISource* source, Sink::ISink* sink)
{
	// If we are in an active state, no way we will allow the
	// addition of channels.
	ASSERT (m_State == INIT);

	// Create a new channel and add it.
	new Channel (m_Channels, source, sink);
}

uint64 Session::Play (const Generics::OptionalType<Generics::TextFragment>& /* channelId */, const uint64 presentationTime)
{
	uint64  currentPresentationTime = NUMBER_MAX_UNSIGNED(uint64);

	// Make sure we are not interuppted by an action change or by
	// a destruction while we are streaming.
	m_Admin.Lock();

	if (m_State != INIT)
	{
		m_State = PLAYING;

		ChannelIterator index (m_Channels);

		while (index.Next())
		{
			uint64 channelTime = index.Play(presentationTime);

			if (channelTime < currentPresentationTime)
			{
				currentPresentationTime = channelTime;
			}
		}

		// Create an absolute base time...
		m_BaseTime = (Generics::Time::Now().Ticks() / TICKS_PER_MILLISECONDS) - currentPresentationTime;

		// We are done with the administration, release the lock we have.
		m_Admin.Unlock();

		// Seems we need to update something here, reschedule, reset...
		Processor::Instance().RevokeWork(*this);

		// Trigger a playout..
		Processor::Instance().ScheduleWork(*this, 0);
	}
	else
	{
		// We are done with the administration, release the lock we have.
		m_Admin.Unlock();
	}

	return (currentPresentationTime);
}

uint64 Session::Pause (const Generics::OptionalType<Generics::TextFragment>& /* channelId */)
{
	uint64  currentPresentationTime = NUMBER_MAX_UNSIGNED(uint64);

	m_Admin.Lock ();

	if ((m_State != PAUSED) && (m_State != INIT))
	{
		m_State = PAUSED;

		// For now turn them all off.
		ChannelIterator index (m_Channels);

		while (index.Next())
		{
			uint64 channelTime = index.Pause();

			if (channelTime < currentPresentationTime)
			{
				currentPresentationTime = channelTime;
			}
		}

		// We are done with the administration, release the lock we have.
		m_Admin.Unlock();

		// Remove this session from the scheduler...
		Processor::Instance().RevokeWork(*this);
	}
	else
	{
		ChannelIterator index (m_Channels);

		while (index.Next())
		{
			uint64 channelTime = index.PresentationTime();

			if (channelTime < currentPresentationTime)
			{
				currentPresentationTime = channelTime;
			}
		}

		// We are done with the administration, release the lock we have.
		m_Admin.Unlock();
	}

	return (currentPresentationTime);
}

void Session::Process ()
{
	m_Admin.Lock();

	if (m_Channels != NULL)
	{
		// Sink all items with a presentation time < than the given presentation time
		uint64 delayTime  = m_Channels->SinkFrames((Generics::Time::Now().Ticks() / TICKS_PER_MILLISECONDS)-m_BaseTime);

		// Should we reschedule this session ?
		if (delayTime == NUMBER_MAX_UNSIGNED(uint64))
		{
			m_State = PAUSED;

			TRACE(Trace::TraceSessionFlow, (_T("Process::StreamCompleted"), 0));
		}
		else if (m_State == PLAYING)
		{
			TRACE(Trace::TraceSessionFlow, (_T("Process::ScheduledWork"), static_cast<sint32>(delayTime - Generics::Time::Now().Ticks())));

			// Calculate the new delay....
			Processor::Instance().ScheduleWork (*this, delayTime);
		}
	}

	m_Admin.Unlock();
}

} } // namespace Solutions::Sessions

