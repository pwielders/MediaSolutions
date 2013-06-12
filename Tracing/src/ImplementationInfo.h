#ifndef __IMPLEMENTATIONINFO_H
#define __IMPLEMENTATIONINFO_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
#define	IMPLEMENTATION_INFO		Trace::ImplementationInfo (		\
									__FILE__,					\
									__LINE__,					\
									typeid (*this).name ())

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	class ImplementationInfo
	{
		// -------------------------------------------------------------------
		// The default constructor is not an option for this object.
		// -------------------------------------------------------------------
		private:
			ImplementationInfo ();
			ImplementationInfo& operator= (const ImplementationInfo& RHS);

		public:
			ImplementationInfo			(
				const char						 a_FileName[],
				const unsigned int				 a_LineNumber,
				const char						 a_ClassName[]) :
					m_FileName					(strrchr(a_FileName,'\\')),
					m_ClassName					(a_ClassName),
					m_LineNumber				(a_LineNumber),
					m_TimeIssued				(Generics::Time::Now())
			{
				m_FileName = (m_FileName != NULL ? &m_FileName[1] : a_FileName);

				sprintf_s (m_DateTime, sizeof (m_DateTime), "%02d-%02d-%02d %02d:%02d:%02d:%03d",
							m_TimeIssued.Year()%100,
							m_TimeIssued.Month(),
							m_TimeIssued.Day(),
							m_TimeIssued.Hours(),
							m_TimeIssued.Minutes(),
							m_TimeIssued.Seconds(),
							m_TimeIssued.MilliSeconds());
			}
			ImplementationInfo			(
				const ImplementationInfo&	 a_Copy) :
					m_FileName					(a_Copy.m_FileName),
					m_ClassName					(a_Copy.m_ClassName),
					m_LineNumber				(a_Copy.m_LineNumber),
					m_TimeIssued				(a_Copy.m_TimeIssued)
			{			
				::strcpy_s (m_DateTime, sizeof(m_DateTime), a_Copy.m_DateTime);
			}
			~ImplementationInfo			()
			{
				TRACE_L5 ("Destructor ImplementationInfo <0x%X>", TRACE_POINTER (this));
				m_FileName = NULL;
				m_ClassName = NULL;
			}

		public:
			inline	const char*
				GetFileName	() const
			{
				return (m_FileName);
			}
			inline	const char*
				GetClassName	() const
			{
				return (m_ClassName);
			}
			inline	const unsigned int
				GetLineNumber	() const
			{
				return (m_LineNumber);
			}
			inline const Generics::Time&
				GetTime () const
			{
				return (m_TimeIssued);
			}
			inline const char*
				GetTimeString () const
			{
				return (m_DateTime);
			}
			//inline const GetTime

		private:
			const char*			m_FileName;
			const char*			m_ClassName;
			const unsigned int	m_LineNumber;
			Generics::Time		m_TimeIssued;
			char				m_DateTime[22];
	};

} } // namespace Solutions::Trace

#endif // __IMPLEMENTATIONINFO_H
