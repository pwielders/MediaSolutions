#ifndef __SESSION_H
#define __SESSION_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "Channel.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sessions
{
	class EXTERNAL Session
	{
		friend class SessionWork;

		private:
			typedef enum PlayState
			{
				INIT,
				PAUSED,
				PLAYING
			};

		private:
			Session();
			Session(const Session&);
			Session& operator= (const Session&);

		public:
			Session(Generics::ProxyType<Source::ISources>& sources);
			virtual ~Session ();

		public:
			inline bool IsValid () const
			{
				return (m_State != INIT);
			}

			inline uint64 Duration () const
			{
				return (m_Medium->Duration());
			}

			inline Channel::Iterator Channels ()
			{
				return (Channel::Iterator(m_Channels));	
			}

			inline const Generics::TextFragment& SessionId() const
			{
				return (m_SessionId);
			}

			uint64 Play (const Generics::OptionalType<Generics::TextFragment>& channelId, const uint64 presentationTime);
			uint64 Pause (const Generics::OptionalType<Generics::TextFragment>& channelId);

			// ----------------------------------------------
			// These are the methods to construct the session
			// Once all channels have been added, activate
			// the session. No new channels can be added to
			// an activated session.
			// ----------------------------------------------
			void Activate();
			void Deactivate();
			void AddChannel (Source::ISource* source, Sink::ISink* sink);

			// ----------------------------------------------
			// Entry points for threads
			// ----------------------------------------------
			void Process();

		private:
			void AddChannel (Channel* channel);

		private:
			Generics::TextFragment					m_SessionId;
			Generics::ProxyType<Source::ISources>	m_Medium;
			Channel*								m_Channels;
			uint64									m_BaseTime;
			PlayState								m_State;
			mutable Generics::CriticalSection		m_Admin;
			static uint32							m_Sequence;
	};

} } // namespace Solutions::Sessions

#endif // __SESSION_H