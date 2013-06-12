#ifndef __TRACEUNIT_H
#define __TRACEUNIT_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "TraceMedia.h"

// ---- Helper types and constants ----

// ---- Helper functions ----

namespace Solutions { namespace Trace
{
	// ---- Referenced classes and types ----
	struct ITraceControl;
	struct ITrace;

	// ---- Class Definition ----
	class EXTERNAL TraceUnit : public Generics::SingletonType<TraceUnit>
	{
		friend class Generics::SingletonType<TraceUnit>;

		private:
			typedef	std::list<ITraceControl*>						TraceControlList;

		public:
			typedef Generics::IteratorType<TraceControlList, ITraceControl*>	Iterator;

		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statements.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceUnit (const TraceUnit&);
			TraceUnit& operator= (const TraceUnit&);

		protected:
			TraceUnit		 () :
				m_Categories ()
			{
				m_OutputChannel = NULL;
			}

		public:
			virtual ~TraceUnit()
			{
				if (m_OutputChannel != NULL)
				{
					delete m_OutputChannel;
				}
			}

		public:
			static inline bool HasOutput ()
			{
				return (m_OutputChannel != NULL);
			}
			void Announce (ITraceControl& Category);
			void Output (const Generics::NodeId& host);
			Iterator GetCategories();		
			uint32 SetCategories (const bool enable, const char* module, const char* category);

			static void
				Trace(const char			a_FileName[],
					  const unsigned int	a_LineNumber,
					  const char			a_ClassName[],
					  const ITrace*	const	a_Information);

		private:
			static char* CopyText (char* destination, const char* source, uint32& maxSize);

		private:
			TraceControlList					m_Categories;
			static Generics::CriticalSection	m_Admin;
			static TraceMedia*					m_OutputChannel;
	};

} } // namespace Solutions::Trace

#endif // __TRACEUNIT_H
