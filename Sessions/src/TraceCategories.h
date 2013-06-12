#ifndef __SESSIONS_TRACE_H
#define __SESSIONS_TRACE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	class TraceSessionFlow : public TraceType<TraceSessionFlow>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceSessionFlow();
			TraceSessionFlow(const TraceSessionFlow& a_Copy);
			TraceSessionFlow& operator= (const TraceSessionFlow& a_RHS);

		public:
			explicit TraceSessionFlow(const TCHAR actionType[]) :
				TraceType<TraceSessionFlow>	()
			{
	#ifdef _UNICODE
 				unsigned int Size	   = _tcslen (MethodName)+1;
				char*		 TempSpace = static_cast <char*> (alloca (Size));

				/*lint -e{534} */
				::wcstombs (TempSpace, MethodName, Size);

				::_snprintf_s (	m_Text, 
								TRACINGBUFFERSIZE, 
								"Left Method: %s", 
								TempSpace);
	#else
				::_snprintf_s (	m_Text, 
								TRACINGBUFFERSIZE, 
								"Flow: %s", 
								actionType);
	#endif
			}
			explicit TraceSessionFlow(const TCHAR actionType[], const sint32 delay) :
				TraceType<TraceSessionFlow>	()
			{
	#ifdef _UNICODE
 				unsigned int Size	   = _tcslen (MethodName)+1;
				char*		 TempSpace = static_cast <char*> (alloca (Size));

				/*lint -e{534} */
				::wcstombs (TempSpace, MethodName, Size);

				::_snprintf_s (	m_Text, 
								TRACINGBUFFERSIZE, 
								"Flow: %s <%d>", 
								TempSpace,
								delay);
	#else
				::_snprintf_s (	m_Text, 
								TRACINGBUFFERSIZE, 
								"Flow: %s <%d>", 
								actionType,
								delay);
	#endif
			}
			virtual ~TraceSessionFlow()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

} } // namespace Solutions::Trace

#endif // __SESSIONS_TRACE_H