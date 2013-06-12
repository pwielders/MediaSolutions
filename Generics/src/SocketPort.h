// SocketPort.h: interface for the SocketPort class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SOCKETPORT_H
#define __SOCKETPORT_H

#include "Module.h"
#include "NodeId.h"
#include "IMedia.h"
#include "StateTrigger.h"

#ifdef __WIN32__
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef  __UNIX__
#define SOCKET            unsigned int
#define SOCKET_ERROR      static_cast<unsigned int>(-1)
#define INVALID_SOCKET    static_cast<SOCKET> (-1)
#include "signals.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace Solutions { namespace Generics
{
	class EXTERNAL SocketPort : public IMedia
	{
	  friend class Trigger;

	  public:
		typedef enum 
		{
			CLOSE    = 0x01,
			OPEN     = 0x02,
			RECEIVE  = 0x04,
			SEND     = 0x08,    
			CONNECT  = 0x10,
			ACCEPT   = 0x20,
			CANCEL   = 0x40,  // Exceptional state result of Cancel call being handled
			SHUTDOWN = 0x80   // Exceptional state result of Close call being handled
		
		} enum_state;  

	  // -------------------------------------------------------------------------
	  // This object should not be copied, assigned or created with a default 
	  // constructor. Prevent them from being used, generatoed by the compiler.
	  // define them but do not implement them. Compile error and/or link error.
	  // -------------------------------------------------------------------------
	  private:
		SocketPort(const SocketPort& a_RHS);
		SocketPort& operator= (const SocketPort& a_RHS);

	  public:
		typedef enum
		{
		  DATAGRAM = SOCK_DGRAM,
		  STREAM   = SOCK_STREAM,
		  RAW      = SOCK_RAW,
		  LISTEN,

		} enum_type;


	  protected:
		SocketPort (const NodeId&      refLocalNode, 
					const enum_type    socketType, 
					const uint32 bufferSize = 0);

	  public:
		virtual ~SocketPort();

	  public:
		inline const NodeId&      LocalNode             () const 
		{
		  return (m_LocalNode); 
		}
		inline const uint8         State                 () const
		{
		  return (m_enumState);
		}
		inline void               Wait                  (uint8 a_State) const
		{
		  m_enumState.WaitState (a_State, INFINITE);  
		}

		virtual uint8 PendingTriggers () const
		{
			return (m_Trigger.PendingTriggers());
		}

		virtual SYSTEM_SYNC_HANDLE SynchronisationHandle () const
		{ 
			return (SyncHandle(m_Trigger.SystemSync())); 
		}

		virtual uint32      Abort                 ();

		const NodeId        RemoteNode            () const;
		uint32				ReadBytes             () const;
		uint32              BufferSize            (bool          a_read) const;
		uint32              Open                  (const uint16 portRange);
		uint32              Open                  (const NodeId& cRemote, 
												   uint32        a_Time);
		uint32              Close                 (uint32        a_Time);
		uint32              Accept                (SocketPort&   a_Socket,
												   NodeId&		 a_Remote,
												   uint32        a_Time);
		uint32              Send                  (const NodeId& cRemote,
												   uint32&       nSize, 
												   const uint8   szBytes[],
												   uint32        nTime = INFINITE);
		uint32              Receive               (NodeId&       cRemote,
												   uint32&       nSize, 
												   uint8         szBytes[],
												   uint32        nTime = INFINITE);
	  private:
		void                Register              ();
		void                Unregister            ();
		bool                SetStatus             (uint8         a_Action);
		void                ClearStatus           (uint8         a_Action);
		uint32              WaitForAction         (uint8         a_ActionType,
												   uint32        a_Time);
		uint32              Read                  (NodeId&       a_Remote, 
												   uint32&       a_Size, 
												   uint8         a_Bytes[]);
		uint32              Write                 (const NodeId& a_Remote, 
												   uint32&       a_Size, 
												   const uint8   a_Bytes[]);
		uint32			    Accept				  (SocketPort&   a_Socket,
												   NodeId&		 a_Node);
		uint32              Connect               (const struct  sockaddr_in& a_Remoter);
		SOCKET				ConstructSocket       (const uint32  mode, 
												   const uint32  bufferSize, 
												   NodeId&       localNode);
		SOCKET				FreeSocket            (const uint32  mode, 
												   const uint32  bufferSize, 
												   NodeId&       localNode,
												   const uint16  maxPorts = ~0);

	  protected:
		inline operator           SOCKET                () { return (m_cSocket); }

		static void				  DestroySocket (SOCKET& socket);
		static SOCKET			  CreateSocket  (uint32 mode, uint32 bufferSize);

	  private:
	#ifdef __WIN32__
		class EXTERNAL Trigger
	#else
		class EXTERNAL Trigger : public SignalHandler
	#endif
		{
		  private:
			Trigger (const Trigger& a_Copy);
			Trigger&   operator= (const Trigger& a_Copy);

		  public:
			Trigger (SocketPort& a_Socket);
			virtual ~Trigger ();

		  public:
			// The check method is single threaded. Make sure only one thread
			// accesses this method at a time.
			uint8  Check           (uint8 a_Action);

			bool					   Wait       (uint32 a_Time);
			void				       SetEvent   ();
			uint8              PendingTriggers () const;
			inline SYSTEM_SYNC_HANDLE  SystemSync () const { return (m_EventHandle); }

	#ifdef __WIN32__       
			void           TriggerEvents   (uint8 a_Action);
	#endif

		  private: 
	#ifdef __POSIX__
			virtual bool   SignalHandler   ();
	#endif

		  private:
			SocketPort&    m_Socket;
			uint8  m_EventBits;
			uint8  m_EventMask;

	#ifdef __POSIX__
			Event          m_EventHandle;
	#endif

	#ifdef __WIN32__
			HANDLE         m_EventHandle;
	#endif
		};

	  private:
		Trigger                   m_Trigger;
		NodeId                    m_LocalNode;
		#pragma warning(disable: 4251) 
		StateTrigger<uint8>        m_enumState;
		#pragma warning(default: 4251) 
		enum_type                 m_enumType;
		SOCKET                    m_cSocket;
		CriticalSection           m_syncAdmin;
		uint8					  m_PendingTriggers;
		int						  m_BufferSize;
	};

	class EXTERNAL SocketStream : public SocketPort
	{
		friend class SocketListner;

		// -------------------------------------------------------------------------
		// This object should not be copied, assigned or created with a default
		// constructor. Prevent them from being used, generatoed by the compiler.
		// define them but do not implement them. Compile error and/or link error.
		// -------------------------------------------------------------------------
		private:
			SocketStream(const SocketStream& a_RHS);
			SocketStream& operator= (const SocketStream& a_RHS);

		public:
			SocketStream(
			  const uint32 nBufferSize = 0) :
				SocketPort      (NodeId(), SocketPort::STREAM, nBufferSize),
				m_RemoteNode    ()
			{
			  TRACE_L5 ("Constructor SocketStream (NodeId&, CSokcetId&) <0x%X>", TRACE_POINTER (this));
			}

			SocketStream(
			  const NodeId&      refLocalNode,
			  const uint32		 nBufferSize = 0) :
				SocketPort      (refLocalNode, SocketPort::STREAM, nBufferSize),
				m_RemoteNode    ()
			{
			  TRACE_L5 ("Constructor SocketStream (NodeId&, CSokcetId&) <0x%X>", TRACE_POINTER (this));
			}

			SocketStream(
			  const NodeId&      refLocalNode,
			  const NodeId&      refRemoteNode,
			  const uint32 nBufferSize = 0) :
				SocketPort      (refLocalNode, SocketPort::STREAM, nBufferSize),
				m_RemoteNode    (refRemoteNode)
			{
			  TRACE_L5 ("Constructor SocketStream (NodeId&, CSokcetId&) <0x%X>", TRACE_POINTER (this));
			}

			virtual ~SocketStream()
			{
			  TRACE_L5 ("Destructor SocketStream <0x%X>", TRACE_POINTER (this));
			}

		public:
			inline uint8 PendingTriggers () const
			{
				return (SocketPort::PendingTriggers() & (RECEIVE|SEND|CLOSE));
			}

			inline const NodeId&  RemoteNode () const 
			{ 
				return (m_RemoteNode); 
			}

			inline uint32 Open (uint32 a_Time)
			{
				return (SocketPort::Open(m_RemoteNode, a_Time));
			}

			inline uint32 Open (const NodeId& cRemote, uint32 a_Time)
			{
				m_RemoteNode = cRemote;

				return (Open(a_Time));
			}

			virtual uint32 Send (uint32& nSize, const uint8 szBytes[], uint32 a_Time)
			{
			  return (SocketPort::Send (m_RemoteNode, nSize, szBytes, a_Time));
			}

			virtual uint32 Receive (uint32& nSize, uint8 szBytes[], uint32 a_Time)
			{
			  NodeId l_RemoteNode = m_RemoteNode;

			  return (SocketPort::Receive (l_RemoteNode, nSize, szBytes, a_Time));
			}

		private:
			NodeId      m_RemoteNode;
	};

	class EXTERNAL SocketDatagram : public SocketPort
	{
	  // -------------------------------------------------------------------------
	  // This object should not be copied, assigned or created with a default
	  // constructor. Prevent them from being used, generatoed by the compiler.
	  // define them but do not implement them. Compile error and/or link error.
	  // -------------------------------------------------------------------------
		private:
			SocketDatagram();
			SocketDatagram(const SocketDatagram& a_RHS);
			SocketDatagram& operator= (const SocketDatagram& a_RHS);

		public:
			SocketDatagram(
				const NodeId&		 refLocalNode,
				const unsigned int	 nBufferSize = 0) :
					SocketPort		(refLocalNode, SocketPort::DATAGRAM, nBufferSize),
					m_RemoteNode	(),
					m_ReceiverNode	()
			{
			  TRACE_L5 ("Constructor SocketDatagram <0x%X>", TRACE_POINTER (this));
			}

			SocketDatagram(
				const NodeId&		 refLocalNode,
				const NodeId&		 refRemoteNode,
				const unsigned int	 nBufferSize = 0) :
					SocketPort		(refLocalNode, SocketPort::DATAGRAM, nBufferSize),
					m_RemoteNode	(refRemoteNode),
					m_ReceiverNode	()
			{
			  TRACE_L5 ("Constructor SocketDatagram <0x%X>", TRACE_POINTER (this));
			}

			virtual ~SocketDatagram()
			{
			  TRACE_L5 ("Destructor SocketDatagram <0x%X>", TRACE_POINTER (this));
			}
			 
		public:
			inline uint8 PendingTriggers () const
			{
				return (SocketPort::PendingTriggers() & (RECEIVE|SEND));
			}

			inline const NodeId& ReceivedNode () const 
			{ 
				return (m_ReceiverNode); 
			}

			inline const NodeId& RemoteNode () const 
			{ 
				return (m_RemoteNode); 
			}

			inline void RemoteNode (const NodeId& remoteNode) 
			{ 
				m_RemoteNode = remoteNode; 
			}

			virtual uint32 Send (uint32& nSize, const uint8 szBytes[], uint32 a_Time)
			{
				return (SocketPort::Send (m_RemoteNode, nSize, szBytes, a_Time));
			}

			virtual uint32 Receive (uint32& nSize, uint8 szBytes[], uint32 a_Time)
			{
				return (SocketPort::Receive (m_ReceiverNode, nSize, szBytes, a_Time));
			}

		private:
			NodeId          m_ReceiverNode;
			NodeId          m_RemoteNode;
	};

	class EXTERNAL SocketListner : public SocketPort
	{
	  // -------------------------------------------------------------------------
	  // This object should not be copied, assigned or created with a default
	  // constructor. Prevent them from being used, generatoed by the compiler.
	  // define them but do not implement them. Compile error and/or link error.
	  // -------------------------------------------------------------------------
		private:
			SocketListner();
			SocketListner(const SocketListner& a_RHS);
			SocketListner& operator= (const SocketListner& a_RHS);

		public:
			SocketListner(
				const NodeId&	 refLocalNode) :
					SocketPort	(refLocalNode, SocketPort::LISTEN, 0)
			{
			  TRACE_L5 ("Constructor SocketListner <0x%X>", TRACE_POINTER (this));
			}

			virtual ~SocketListner()
			{
			  TRACE_L5 ("Destructor SocketListner <0x%X>", TRACE_POINTER (this));
			}

		public:
			inline uint8 PendingTriggers () const
			{
				return (SocketPort::PendingTriggers() & ACCEPT);
			}

			inline uint32 Accept (SocketStream& a_Socket, uint32 time)
			{
				return (SocketPort::Accept (a_Socket, a_Socket.m_RemoteNode, time));
			}

			virtual uint32 Send (uint32& nSize, const uint8 szBytes[], uint32 nTime);
			virtual uint32 Receive (uint32& nSize, uint8 szBytes[], uint32 nTime);
	};

} } // namespace Solutions::Generics

#endif // __SOCKETPORT_H
