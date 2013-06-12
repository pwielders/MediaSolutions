#ifndef __TRACECONTROL_H
#define __TRACECONTROL_H

// ---- Include system wide include files ----
#include <typeinfo>

// ---- Include local include files ----
#include "Module.h"
#include "TraceUnit.h"
#include "ImplementationInfo.h"

// ---- Referenced classes and types ----
#define TRACINGBUFFERSIZE		 1024
#define OUTPUTBUFFERSIZE		(TRACINGBUFFERSIZE + 512)	// 512 bytes fro preamble


// ---- Helper types and constants ----
#define TRACE(TRACECATEGORY, PARAMETERS)						\
if ( (Trace::TraceType<TRACECATEGORY>::IsEnabled() != false) &&	\
	 (Trace::TraceUnit::HasOutput() == true) )					\
{																\
		TRACECATEGORY message PARAMETERS;						\
																\
		__if_exists(this)										\
		{														\
			Trace::TraceUnit::Trace (							\
					__FILE__,									\
					__LINE__,									\
					typeid (*this).name (),						\
					&message);									\
		}														\
		__if_not_exists(this)									\
		{														\
			Trace::TraceUnit::Trace (							\
					__FILE__,									\
					__LINE__,									\
					"<<Global>>",								\
					&message);									\
		}														\
}

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Trace
{
	struct ITraceControl
	{
		virtual const char* GetCategoryName () const = 0;
		virtual const char* GetModuleName () const = 0;
		virtual bool Enabled () const = 0;
		virtual void Enabled (const bool enabled) = 0;
	};

	struct ITrace : public ITraceControl
	{
		virtual const char* GetTraceText () const = 0;
	};

	template <typename CATEGORY>
	class TraceType : public ITrace  
	{
		private:
			class TraceControl : public ITraceControl
			{
				private:
					// -------------------------------------------------------------------
					// This object should not be copied or assigned. Prevent the copy 
					// constructor and assignment constructor from being used. Compiler 
					// generated assignment and copy methods will be blocked by the 
					// following statments.
					// Define them but do not implement them, compile error/link error.
					// -------------------------------------------------------------------
					TraceControl ();
					TraceControl (const TraceControl&);
					TraceControl& operator= (const TraceControl&);

				public:
					explicit TraceControl	(const char a_CategoryName[], const char a_ModuleName[]) :
						m_CategoryName		(GetFixedName (a_CategoryName)),
						m_ModuleName		(a_ModuleName),
						m_Enabled			(false)
					{
						// Register Our trace control unit, so it can be influenced from the outside
						// if nessecary..
						TraceUnit::Instance ().Announce (*this);
					}
					virtual ~TraceControl()
					{
					}

				public:
					inline bool IsEnabled () const
					{
						return (m_Enabled);
					}
					virtual const char* GetCategoryName () const
					{
						return (m_CategoryName);
					}
					virtual const char* GetModuleName () const
					{
						return (m_ModuleName);
					}
					virtual bool Enabled () const
					{
						return (IsEnabled());
					}
					virtual void Enabled (const bool enabled)
					{
						m_Enabled = enabled;
					}

				private:
					const char*
						GetFixedName (const char a_CategoryName[]) const
					{
						// Make sure the class keyword in front of it is removed.
						const char*		NewName   = a_CategoryName;
						const char*		SpaceName = strchr (a_CategoryName, ' ');

						// If we found a 'Space'Skip over it. the classname starts
						if (SpaceName != NULL)
						{
							NewName = ++SpaceName;
						}

						return (NewName);
					}

				protected:
					const char*					m_CategoryName;
					const char*					m_ModuleName;
					bool						m_Enabled;
			};

		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			TraceType (const TraceType<CATEGORY>&);
			TraceType<CATEGORY>& operator= (const TraceType<CATEGORY>&);

		public:
			TraceType()
			{
			}
			virtual ~TraceType ()
			{
			}

		public:
			// No dereference etc.. 1 straight line to enabled or not... Quick method..
			inline static bool IsEnabled ()
			{
				return (s_TraceControl.IsEnabled ());
			}

			virtual const char* GetCategoryName () const
			{
				return (s_TraceControl.GetCategoryName());
			}

			virtual const char* GetModuleName () const
			{
				return (s_TraceControl.GetModuleName());
			}

			virtual bool Enabled () const
			{
				return (s_TraceControl.Enabled());
			}

			virtual void Enabled (const bool enabled)
			{
				s_TraceControl.Enabled (enabled);
			}
			
		private:
			static TraceControl		s_TraceControl;
	};

	template <typename CATEGORY>
	typename TraceType<CATEGORY>::TraceControl typename TraceType<CATEGORY>::s_TraceControl (typeid(CATEGORY).name (), MODULE_NAME);

} } // namespace Solutions::Trace

#endif // __TRACECONTROL_H