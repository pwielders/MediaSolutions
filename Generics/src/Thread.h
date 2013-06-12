// ===========================================================================
//
// Filename:    thread.h
//
// Description: Header file for the Posix thread functions. This class
//              encapsulates all posix thread functionality defined by the
//              system.
//              Note: once the thread is started, it can be stopped only
//                    once. Since there is not a good Posix interface to
//                    suspend a thread and to resume it again, the class
//                    shows this limited, one time start, one time stop
//                    ability. The stop is actually a pthread_cancel call.
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================
#ifndef __THREAD_H
#define __THREAD_H

#include "Module.h"
#include "Sync.h"
#include "StateTrigger.h"
#include "Queue.h"
#include "Time.h"

namespace Solutions { namespace Generics
{
	template <typename THREADLOCALSTORAGE>
	class ThreadLocalStorageType
	{
		private:
			ThreadLocalStorageType ()
			{
				TRACE_L5 ("Constructor ThreadControlBlockInfo <0x%X>", TRACE_POINTER (this));

				#ifdef __WIN32__
				m_Index = TlsAlloc ();

				ASSERT (m_Index != -1);
				#endif

				#ifdef __POSIX__
				if ( pthread_key_create (&m_Key, NULL) != 0)
				{
					ASSERT (false);
				}
				#endif
			}

		public:
			~ThreadLocalStorageType ()
			{
				TRACE_L5 ("Destructor ThreadControlBlockInfo <0x%X>", TRACE_POINTER (this));

				#ifdef __WIN32__
				if ( m_Index != -1 )
				{
					TlsFree (m_Index);
				}
				#endif

				#ifdef __POSIX__
				pthread_key_delete (m_Key);
				#endif
			}

			static ThreadLocalStorageType<THREADLOCALSTORAGE>& Instance()
			{
				static ThreadLocalStorageType<THREADLOCALSTORAGE> g_Singleton;

				return (g_Singleton);
			}

			THREADLOCALSTORAGE* Context ()
			{
				#ifdef __WIN32__
				void* l_Result = TlsGetValue (m_Index);
				#endif

				#ifdef __POSIX__
				void* l_Result = pthread_getspecific (m_Key);
				#endif

			   return reinterpret_cast <THREADLOCALSTORAGE*> (l_Result);
			}

			const THREADLOCALSTORAGE* Context () const
			{
				#ifdef __WIN32__
				void* l_Result = TlsGetValue (m_Index);
				#endif

				#ifdef __POSIX__
				void* l_Result = pthread_getspecific (m_Key);
				#endif

			   return reinterpret_cast <const THREADLOCALSTORAGE*> (l_Result);
			}

			void Context (THREADLOCALSTORAGE* l_Info)
			{
				#ifdef __WIN32__
				if (TlsSetValue (m_Index, &l_Info) == FALSE)
				{
					ASSERT (false);
				}
				#endif

				#ifdef __POSIX__
				if (pthread_setspecific (m_Key, &l_Info) != 0)
				{
					ASSERT (false);
				}
				#endif
			}

		private:
			#ifdef __WIN32__
			DWORD                              m_Index;
			#endif
			#ifdef __UNIX__
			pthread_key_t                      m_Key;
			#endif
	};

	class EXTERNAL Thread
	{
	  // -----------------------------------------------------------------------
	  // This object should not be copied or assigned. Prevent the copy 
	  // constructor and assignment constructor from being used. Compiler 
	  // generated assignment and copy methods will be blocked by the 
	  // following statments.
	  // Define them but do not implement them, compile error and/or link error.
	  // -----------------------------------------------------------------------
	  private:
		Thread	(const Thread& a_Copy);
		Thread& operator= (const Thread& a_RHS);

	  public:
		typedef enum
		{
		  SUSPENDED  = 0x0001,
		  RUNNING    = 0x0002,
		  DEACTIVATE = 0x0004,
		  BLOCKED    = 0x0008,
		  STOPPED    = 0x0010,
		  DELAYED    = 0x0020

		} thread_state;

	  public:
		Thread (const bool blRelease = true, const TCHAR* threadName = NULL);
		virtual ~Thread();

		void                   Suspend              ();
		void                   Block                ();
		void                   Run                  ();
		void                   Delay				();
		void                   Delay				(const unsigned int delay);
		void                   Delay				(const Time& absolute);
		thread_state           State                () const;
		bool                   Wait                 (const unsigned int enumState, unsigned int nTime = INFINITE) const;
		virtual uint32		   Worker               () = 0;
		inline bool            IsRunning            () const { return (m_enumState == RUNNING); }

		template<typename STORAGETYPE>
		static STORAGETYPE* GetContext ()
		{
			return (ThreadLocalStorageType<STORAGETYPE>.Instance().GetContext());
		}

	  protected:
		void                   Terminate			();
		bool                   State                (thread_state enumState);
		void				   ThreadName			(const TCHAR* threadName);

	  private:
	#ifdef __WIN32__
		static void            StartThread          (Thread* pObject);
	#endif 

	#ifdef __POSIX__
		static void*           StartThread          (Thread* pObject);
	#endif 

	  private:
		#pragma warning(disable: 4251) 
		StateTrigger<thread_state>           m_enumState;
		#pragma warning(default: 4251) 

		mutable CriticalSection              m_syncAdmin;

	#ifdef __POSIX__
		Event                                m_sigExit;
		pthread_t                            m_hThreadInstance;
	#endif

	#ifdef __WIN32__
		Event                                m_sigExit;
		thread_state                         m_enumSuspendedState;
		HANDLE                               m_hThreadInstance;
		DWORD								 m_ThreadId;
	#endif
	};

	//
	// Description: Helper class to use pointers or proxies (if lifetime management needs to be automated)
	//              as a carrier to be executed by the threadpooltype.
	//
	template<typename CONTEXT>
	class ThreadContextType
	{
		public:
			inline ThreadContextType() : m_Content ()
			{
			}
			inline ThreadContextType (const CONTEXT& content) : m_Content(content)
			{
			}
			inline ThreadContextType(const ThreadContextType<CONTEXT>& copy) : m_Content (copy.m_Content)
			{
			}
			inline ~ThreadContextType ()
			{
			}
			inline ThreadContextType<CONTEXT>& operator= (const ThreadContextType<CONTEXT>& RHS)
			{
				m_Content = RHS.m_Content;

				return (*this);
			}
			inline void Process()
			{
				m_Content->Process();
			}
			CONTEXT& Context()
			{
				return (m_Content);
			}
			inline bool operator== (const ThreadContextType<CONTEXT>& RHS) const
			{
				return (m_Content == RHS.m_Content);
			}
			inline bool operator!= (const ThreadContextType<CONTEXT>& RHS) const
			{
				return !(operator==(RHS));
			}
					
		private:
			CONTEXT	m_Content;
	};

	template<typename CONTEXT, const uint16 THREADCOUNT, const uint32 QUEUESIZE = 0x7FFFFFFF>
	class ThreadPoolType
	{
		private:
			template<typename CONTEXT>
			class ThreadUnitType : public Thread
			{
				// -----------------------------------------------------------------------
				// This object should not be copied or assigned. Prevent the copy 
				// constructor and assignment constructor from being used. Compiler 
				// generated assignment and copy methods will be blocked by the 
				// following statments.
				// Define them but do not implement them, compile error and/or link error.
				// -----------------------------------------------------------------------
				private:
					ThreadUnitType (const ThreadUnitType& a_Copy);
					ThreadUnitType& operator= (const ThreadUnitType& a_RHS);

				public:
					ThreadUnitType () : 
					  Thread (false), 
					  m_Queue (NULL),
					  m_Executing()
					{
					}
					~ThreadUnitType()
					{
					}
					inline void Initialize (QueueType<CONTEXT>& queue)
					{
						m_Queue = &queue;
					}

					bool Executing (const CONTEXT& thisElement) const
					{
						bool result = (m_Executing == thisElement);

						while (m_Executing == thisElement)
						{
							// Seems like we are executing it.
							// No fancy stuff, just give up the slice and try again later.
							::Sleep(0);
						}

						return (result);
					}
					inline void ThreadName (const TCHAR* threadName)
					{
						Thread::ThreadName (threadName);
					}

				private:
					virtual uint32 Worker ()
					{
						ASSERT (m_Queue != NULL);

						if (m_Queue->Extract(m_Executing, INFINITE) == true)
						{
							// Seems like we have work...
							m_Executing.Process();

							// Clear it out, we processed it.
							m_Executing = ThreadPoolType<CONTEXT,THREADCOUNT,QUEUESIZE>::s_EmptyContext;
						}

						// Do not wait keep on processing !!!
						return (0);
					}

				private:
					CONTEXT					m_Executing;
					QueueType<CONTEXT>*		m_Queue;
			};

		// -----------------------------------------------------------------------
		// This object should not be copied or assigned. Prevent the copy 
		// constructor and assignment constructor from being used. Compiler 
		// generated assignment and copy methods will be blocked by the 
		// following statments.
		// Define them but do not implement them, compile error and/or link error.
		// -----------------------------------------------------------------------
		private:
			ThreadPoolType (const ThreadPoolType& a_Copy);
			ThreadPoolType& operator= (const ThreadPoolType& a_RHS);

		public:
			ThreadPoolType	(const TCHAR* poolName) :
				m_Queue	(QUEUESIZE)
			{
				TCHAR unitName[1024];
				uint32 numberStart = 1;

				// insert the baseName
				_stprintf_s(unitName, 1000, "%s[", poolName);
				numberStart = _tcslen(unitName);

				for (uint32 teller = 0; teller < THREADCOUNT; teller++)
				{
					m_Units[teller].Initialize (m_Queue);
					m_Units[teller].Run	       ();

					if (poolName != NULL)
					{
						// Add the counter to the name..
						::_itoa_s(teller, &unitName[numberStart], sizeof(unitName) - numberStart - 1, 10);
						uint32 offset = _tcslen(&unitName[numberStart]);
						unitName[numberStart+offset]   = ']';
						unitName[numberStart+offset+1] = '\0';
						
						m_Units[teller].ThreadName (unitName);
					}
				}
			}
			~ThreadPoolType()
			{
				// Stop all threads...
				Block ();
				
				m_Queue.Flush();

				// Wait till all threads have reached completion
				Wait(Thread::BLOCKED, INFINITE);
			}

		public:
			inline void Submit (CONTEXT& data, const DWORD waitTime)
			{
				if (QUEUESIZE == ~0)
				{
					m_Queue.Post(data);
				}
				else
				{
					m_Queue.Insert (data, waitTime); 
				}
			}
			void Revoke (const CONTEXT& data)
			{
				if (m_Queue.Remove(data) == false)
				{
					uint16 count = THREADCOUNT;

					// Check if it is currently being executed and wait till it is done.								
					while ( (count > 0) && (m_Units[count-1].Executing(data) == false) )
					{
						--count;
					}
				}
			}

			bool Wait (const unsigned int enumState, unsigned int nTime = INFINITE) const
			{
				uint16 teller = THREADCOUNT;

				// Block all threads!!
				while ((teller > 0) && (m_Units[teller-1].Wait (enumState, nTime) == true) )
				{
					teller--;
				}			
		
				return (teller == 0);
			}

			void Block()
			{
				// Block all threads!!
				for (uint16 teller = THREADCOUNT; teller > 0; --teller )
				{
					m_Units[teller-1].Block();
				}

				m_Queue.Disable ();
			}

			void Run()
			{
				// Make all threads active again !!
				for (uint16 teller = THREADCOUNT; teller > 0; --teller )
				{
					m_Units[teller-1].Run();
				}					
			}

		private:
			QueueType<CONTEXT>			m_Queue;
			ThreadUnitType<CONTEXT>		m_Units[THREADCOUNT];
			static CONTEXT				s_EmptyContext;
	};

	template <typename CONTEXT, const uint16 THREADCOUNT, const uint32 QUEUESIZEF>
	CONTEXT typename ThreadPoolType<CONTEXT,THREADCOUNT,QUEUESIZEF>::s_EmptyContext;


} } // namespace Solutions::Generics

#endif // __THREAD_H
