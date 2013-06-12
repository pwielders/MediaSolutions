#ifndef __TRACEMEDIA_H
#define __TRACEMEDIA_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	// ---- Helper types and constants ----
	struct ITrace;

	class EXTERNAL TraceMedia
	{
		private:
			class DispatcherThread : public Generics::Thread
			{
				private:
					DispatcherThread (const DispatcherThread&);
					DispatcherThread& operator= (const DispatcherThread&);

				public:
					DispatcherThread		(TraceMedia& parent, Generics::IMedia* media) :
						Generics::Thread	(false),
						m_Parent			(parent),
						m_Media				(media)
					{
					}
					~DispatcherThread ()
					{
					}

				public:
					virtual uint32 Worker ()
					{
						while (IsRunning())
						{
							HANDLE eventHandle = m_Media->SynchronisationHandle();
							// Wait till something happens !!!
							WaitForMultipleObjects(1, &eventHandle, FALSE, INFINITE);

							// Make sure it is not a restart request...
							m_Parent.HandleMessage();
						}

						return (INFINITE);
					}

				private:
					TraceMedia&			m_Parent;
					Generics::IMedia*	m_Media;
			};

		
		private:
			TraceMedia();
			TraceMedia(const TraceMedia&);
			TraceMedia& operator= (const TraceMedia&);

		public:
			TraceMedia (const Generics::NodeId& nodeId);
			~TraceMedia ();

			void SendTraceLine (
				const char			a_FileName[],
				const unsigned int	a_LineNumber,
				const char			a_ClassName[],
				const ITrace*		a_Information);

		private:
			uint8* CopyText (uint8* destination, const char* source, uint32& maxSize);
			void HandleMessage();

		private:
			Generics::SocketDatagram	m_Output;
			DispatcherThread			m_CommandHandler;
	};

} } // namespace Solutions::Trace

#endif // __TRACEMEDIA_H
