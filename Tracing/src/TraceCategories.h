#ifndef __TRACECATEGORIES_H
#define __TRACECATEGORIES_H

// ---- Include system wide include files ----
#include <stdlib.h>

// ---- Include local include files ----
#include "Module.h"
#include "TraceControl.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	class TraceConstructor : public TraceType<TraceConstructor>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceConstructor(const TraceConstructor& a_Copy);
			TraceConstructor& operator= (const TraceConstructor& a_RHS);

		public:
			TraceConstructor() :
				TraceType<TraceConstructor> ()
			{
			}
			virtual ~TraceConstructor()
			{
			}
			virtual const char* GetTraceText () const
			{
				return ("Constructor called");
			}
	};

	class TraceDestructor : public TraceType<TraceDestructor>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceDestructor(const TraceDestructor& a_Copy);
			TraceDestructor& operator= (const TraceDestructor& a_RHS);

		public:
			TraceDestructor() :
				TraceType<TraceDestructor> ()
			{
			}
			virtual ~TraceDestructor()
			{
			}
			virtual const char* GetTraceText () const
			{
				return ("Destructor called");
			}
	};

	class TraceCopyConstructor : public TraceType<TraceCopyConstructor>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceCopyConstructor(const TraceCopyConstructor& a_Copy);
			TraceCopyConstructor& operator= (const TraceCopyConstructor& a_RHS);

		public:
			TraceCopyConstructor() :
				TraceType<TraceCopyConstructor> ()
			{
			}
			virtual ~TraceCopyConstructor()
			{
			}
			virtual const char* GetTraceText () const
			{
				return ("Copy constructor called");
			}
	};

	class TraceAssignmentOperator : public TraceType<TraceAssignmentOperator>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceAssignmentOperator(const TraceAssignmentOperator& a_Copy);
			TraceAssignmentOperator& operator= (const TraceAssignmentOperator& a_RHS);

		public:
			TraceAssignmentOperator() :
				TraceType<TraceAssignmentOperator> ()
			{
			}
			virtual ~TraceAssignmentOperator()
			{
			}
			virtual const char* GetTraceText () const
			{
				return ("Assignment operator called");
			}
	};

	class TraceMethodEntry : public TraceType<TraceMethodEntry>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceMethodEntry();
			TraceMethodEntry(const TraceMethodEntry& a_Copy);
			TraceMethodEntry& operator= (const TraceMethodEntry& a_RHS);

		public:
			TraceMethodEntry(const TCHAR MethodName[]) : 
				TraceType<TraceMethodEntry> ()		
			{
	#ifdef _UNICODE
 				unsigned int Size	   = _tcslen (MethodName)+1;
				char*		 TempSpace = static_cast <char*> (alloca (Size));

				/*lint -e{534} */
				::wcstombs (TempSpace, MethodName, Size);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Entered Method: %s", 
								TempSpace);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Entered Method: %s", 
								MethodName);
	#endif
			}
			virtual ~TraceMethodEntry()
			{
			}
			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char m_Text[TRACINGBUFFERSIZE];
	};

	class TraceMethodExit : public TraceType<TraceMethodExit>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceMethodExit();
			TraceMethodExit(const TraceMethodExit& a_Copy);
			TraceMethodExit& operator= (const TraceMethodExit& a_RHS);

		public:
			explicit TraceMethodExit(const TCHAR MethodName[]) :
				TraceType<TraceMethodExit>	()
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
								"Left Method: %s", 
								MethodName);
	#endif
			}
			virtual ~TraceMethodExit()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceInformationText : public TraceType<TraceInformationText>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceInformationText();
			TraceInformationText(const TraceInformationText& a_Copy);
			TraceInformationText& operator= (const TraceInformationText& a_RHS);

		public:
			explicit TraceInformationText(const TCHAR text[]) :
				TraceType<TraceInformationText> ()		
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text, TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								text);
	#endif
			}
			virtual ~TraceInformationText()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceWarningText : public TraceType<TraceWarningText>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceWarningText();
			TraceWarningText(const TraceWarningText& a_Copy);
			TraceWarningText& operator= (const TraceWarningText& a_RHS);

		public:
			explicit TraceWarningText(const TCHAR text[]) :
				TraceType<TraceWarningText> ()
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text, TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								text);
	#endif
			}
			virtual ~TraceWarningText()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceErrorText : public TraceType<TraceErrorText>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceErrorText();
			TraceErrorText(const TraceErrorText& a_Copy);
			TraceErrorText& operator= (const TraceErrorText& a_RHS);

		public:
			explicit TraceErrorText(const TCHAR text[]) :
				TraceType<TraceErrorText> ()
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text, TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								text);
	#endif
			}
			virtual ~TraceErrorText()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceFatalText : public TraceType<TraceFatalText>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceFatalText();
			TraceFatalText(const TraceFatalText& a_Copy);
			TraceFatalText& operator= (const TraceFatalText& a_RHS);

		public:
			explicit TraceFatalText(const TCHAR text[]) :
				TraceType<TraceFatalText>	()
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text, TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Info: %s", 
								text);
	#endif

			}
			virtual ~TraceFatalText()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceInitialisation : public TraceType<TraceInitialisation>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceInitialisation();
			TraceInitialisation(const TraceInitialisation& a_Copy);
			TraceInitialisation& operator= (const TraceInitialisation& a_RHS);

		public:
			explicit TraceInitialisation(const TCHAR text[]) :
				TraceType<TraceInitialisation>	()
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text,TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Initialisation: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Initialisation: %s", 
								text);
	#endif
			}
			virtual ~TraceInitialisation()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char 	m_Text[TRACINGBUFFERSIZE];
	};

	class TraceAssert: public TraceType<TraceAssert>
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceAssert(const TraceAssert& a_Copy);
			TraceAssert& operator= (const TraceAssert& a_RHS);

		public:
			TraceAssert() :
				TraceType<TraceAssert>	()
			{
					strcpy_s(m_Text, sizeof(m_Text), "Assertion: <<No description supplied>>");
			}

			explicit TraceAssert(const TCHAR text[]) :
				TraceType<TraceAssert>	()
			{
	#ifdef _UNICODE
				unsigned int TextSize		= ::_tcslen (text) + 1;
				char*		 TempText		= static_cast <char*> (alloca (TextSize));

				/*lint -e{534} */
				::wcstombs (TempText, text, TextSize);

				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Assertion: %s", 
								TempText);
	#else
				::_snprintf_s (	m_Text, sizeof(m_Text), 
								TRACINGBUFFERSIZE, 
								"Assertion: %s", 
								text);
	#endif
			}
			virtual ~TraceAssert()
			{
			}

			virtual const char* GetTraceText () const
			{
				return (m_Text);
			}

		private:
			char		m_Text[TRACINGBUFFERSIZE];
	};

} } // namespace Solutions::Trace

#endif // __TRACECATEGORIES_H
