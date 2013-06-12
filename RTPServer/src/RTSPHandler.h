#ifndef __RTSPHANDLER_H
#define __RTSPHANDLER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions
{
	// ---- Referenced classes and types ----
	class RTSPServer;

	class RTSPHandler
	{
		private:
			RTSPHandler(const RTSPHandler&);
			RTSPHandler& operator= (const RTSPHandler&);

		public:
			RTSPHandler(Generics::SocketServerType<RTSPHandler>* parent, Generics::SocketStream* media);
			~RTSPHandler();

		public:
			void HandleProtocol ();
			void Process(const RTSP::Message& info);
			virtual bool Authenticate (const RTSP::Message& /* info */);

		private:
			void Options		(const RTSP::Message& info, RTSP::Message& response);
			void Describe		(const RTSP::Message& info, RTSP::Message& response);
			void Setup			(const RTSP::Message& info, RTSP::Message& response);
			void Play			(const RTSP::Message& info, RTSP::Message& response);
			void Pause			(const RTSP::Message& info, RTSP::Message& response);
			void SetParameter	(const RTSP::Message& info, RTSP::Message& response);
			void GetParameter	(const RTSP::Message& info, RTSP::Message& response);
			void Teardown		(const RTSP::Message& info, RTSP::Message& response);
			void Announce		(const RTSP::Message& info, RTSP::Message& response);
			void Redirect		(const RTSP::Message& info, RTSP::Message& response);
			void Record			(const RTSP::Message& info, RTSP::Message& response);

			bool SDPDescription(const Generics::TextFragment& identifier, String& result);
			uint16 AddChannel (Sessions::Session& sessionId, Generics::ProxyType<Source::ISources>& sources, const RTSP::MessageOptions& identifier, const uint8 trackId, const Generics::NodeId& connectPoint);

		private:
			RTSPServer&										m_Parent;
			Generics::SocketStream*							m_RTSPClient;
			String											m_ReceiveText;
			Generics::TextStreamerType<char, '\r', '\n'>	m_Channel;
			uint32											m_LastIndex;
			const Generics::NodeId&							m_RemoteNode;
	};

} // namespace Solutions

#endif // __RTSPHANDLER_H