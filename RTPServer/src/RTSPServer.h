#ifndef __RTSPSERVER_H
#define __RTSPSERVER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "RTSPHandler.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions 
{
	class RTSPServer : public Generics::SocketServerType<RTSPHandler>
	{
		public:
			typedef enum Properties
			{
				AGENT_SETUP_ALLOWED = 0x01
			};

		private:
			RTSPServer(const RTSPServer&);
			RTSPServer& operator= (const RTSPServer&);

		public:
			RTSPServer (const Generics::NodeId& node, const TCHAR description[], const uint32 characteristics);
			virtual ~RTSPServer ();

		public:
			virtual bool Autheticate ();
			const TCHAR* Description () const;
			bool AgentSetup () const;
			Generics::ProxyType<Source::ISources> Source (const Generics::TextFragment& identifier);
			Sink::Factory& Factory ();

			// The registration methods are conform the Producer-Consumer pattern.
			// That what is produced by the caller, will be destructed by this server.
			void Register (Source::ISourceFactory* sources);
			void Register (Sink::ISinkFactory* sources);

		private:
			String							m_Description;
			uint32							m_Characteristics;
			Source::Factory					m_SourceTypes;
			Sink::Factory					m_SinkTypes;
	};

} // namespace Solutions

#endif // __RTSPHANDLER_H