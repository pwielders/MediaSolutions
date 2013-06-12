#ifndef __CHANNEL_H
#define __CHANNEL_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sessions
{
	// This class is used safely concurrent only because all calls are synchronised via the session objects.
	class EXTERNAL Channel 
	{
		public:
			static String Name (const uint8 index);

		public:
			class EXTERNAL Iterator
			{
				private:
					Iterator ();

				public:
					inline Iterator (Channel* root) :
						m_Index(NUMBER_MAX_UNSIGNED(uint32)),
						m_Current (NULL),
						m_Root(root)
					{
					}
					inline Iterator (const Iterator& copy) :
						m_Index(copy.m_Index),
						m_Current (copy.m_Current),
						m_Root(copy.m_Root)
					{
					}
					~Iterator()
					{
					}

					inline Iterator& operator= (const Iterator& RHS)
					{
						m_Index = RHS.m_Index;
						m_Current = RHS.m_Current;
						m_Root = RHS.m_Root;
						return (*this);
					}

				public:
					// Returns if we are at a valid Channel location.
					inline bool IsValid() const
					{
						return (m_Current != NULL);
					}

					// Returns if we are at a valid Channel location.
					inline void Reset()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint32);
						m_Current = NULL;
					}

					// Move on to the next sample/picture/frame
					bool Next ()
					{
						if (m_Index == NUMBER_MAX_UNSIGNED(uint32))
						{
							m_Current = m_Root;
							m_Index = 0;
						}
						else if (m_Current != NULL)
						{
							m_Index++;
							m_Current = m_Current->GetNext();
						}
						return (m_Current != NULL);
					}

					inline bool IsActive() const
					{
						return (m_Current->IsActive());
					}

					inline uint32 Sequence () const
					{
						return (m_Current->Sequence());
					}

					inline uint32 SyncStamp () const
					{
						return (m_Current->SyncStamp());
					}

					inline uint64 PresentationTime () const
					{
						return (m_Current->PresentationTime());
					}

					inline String Identifier () const
					{
						return (m_Current->Name());
					}

				protected:
					Channel* Element()
					{
						return (m_Current);
					}

				private:
					uint32					m_Index;
					Channel*				m_Root;
					Channel*				m_Current;
			};

		public:
			Channel(Channel*& root, Source::ISource* source, Sink::ISink* sink);
			virtual ~Channel();

		public:
			inline String Name() const
			{
				return (m_Identifier.Text());
			}

			//----------------------------------------------------
			// Methods to support selected streaming on channels
			// within a session (if required).
			//----------------------------------------------------
			inline bool IsActive() const
			{
				return (m_Active);
			}
			inline uint32 Sequence () const
			{
				return (m_Sink->Sequence());
			}
			inline uint32 SyncStamp () const
			{
				return (m_Sink->SyncStamp());
			}
			inline uint64 PresentationTime () const
			{
				return (m_Source->PresentationTime());
			}

			// Progress the stream up to the presentation time (relative in ms)
			uint64 SinkFrames (const uint64 presentationTime);

		private:
			friend class ChannelIterator;

			inline uint64 Play (const uint64 tickLocation)
			{
				// If the time where we need to start playing is defined, set it..
				if (tickLocation != NUMBER_MAX_UNSIGNED(uint64))
				{
					m_Source->PresentationTime(tickLocation);
				}

				m_Active = true;

				return (m_Source->PresentationTime());
			}
			inline uint64 Pause ()
			{
				m_Active = false;

				return (m_Source->PresentationTime());
			}
			inline uint64 PresentationTime ()
			{
				return (m_Source->PresentationTime());
			}
			inline Channel* GetNext()
			{
				return (m_NextChannel);
			}

		private:
			bool							m_Active;
			Source::ISource*				m_Source;
			Sink::ISink*					m_Sink;
			Channel*						m_NextChannel;
			Generics::TextFragment			m_Identifier;
	};

} } // namespace Solutions::Sessions

#endif // __CHANNEL_H