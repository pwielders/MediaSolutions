#ifndef __RTPSERVER_TRACE_H
#define __RTPSERVER_TRACE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	class RTSPRequest : public TraceType<RTSPRequest>
	{
		public:
			RTSPRequest(const String& text) :
				TraceType<RTSPRequest> (), m_Text(text)
			{
			}
			virtual ~RTSPRequest()
			{
			}
			virtual const char* GetTraceText () const
			{
				return (m_Text.c_str());
			}

		private:
			String		m_Text;
	};

	class RTSPResponse : public TraceType<RTSPResponse>
	{
		public:
			RTSPResponse(const String& text) :
				TraceType<RTSPResponse> (), m_Text(text)
			{
			}
			virtual ~RTSPResponse()
			{
			}
			virtual const char* GetTraceText () const
			{
				return (m_Text.c_str());
			}

		private:
			String		m_Text;
	};

} } // namespace Solutions::Trace

#endif // __RTPSERVER_TRACE_H