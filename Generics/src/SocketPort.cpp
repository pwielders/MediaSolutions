// SocketPort.cpp: implementation of the SocketPort class.
//
//////////////////////////////////////////////////////////////////////

#include "socketport.h"

#ifdef __POSIX__
#include <netdb.h>
#include <arpa/inet.h>
#include <stropts.h>
#include <poll.h>
#include <fcntl.h>
#define ERRORRESULT        errno
#define SD_BOTH            SHUT_RDWR
#define ERROR_WOULDBLOCK   EWOULDBLOCK
#define ERROR_INPROGRESS   EINPROGRESS
#define ERROR_ISCONN       EISCONN
#endif

#ifdef __LINUX__
#define POLLNORM           0x00
#endif

#ifdef __WIN32__
#include <Winsock2.h>
#include <ws2tcpip.h>
#define ERRORRESULT        ::WSAGetLastError ()
#define ERROR_WOULDBLOCK   WSAEWOULDBLOCK
#define ERROR_INPROGRESS   WSAEINPROGRESS
#define ERROR_ISCONN       WSAEISCONN
#pragma warning(disable: 4355) // 'this' used in initializer list
#endif

// A maximum of two threads can be active in the SocketPort class,
// with active we main pending on IO.
// Table           IO                     Cancel/Close
//------------------------------------------------------------------
// LISTEN          ACCEPT                 Optional
// STREAM          CONNECT                Optional
//                 RECEIVE/SEND           Optional
//                 CLOSE                  -------
// DATAGRAM        RECEIVE/SEND           Optional
//                 CLOSE                  -------
// State diagram
//
// External dependend blocking calls are:
//
// ACCEPT:     Returns when a connection comes in.
// RECEIVE:       Returns if data from the outside came in.
// SEND:      Return if data has been written to the outside.
// CONNECT:    Returns if the connection has been accepted on teh outside.
//
// These Calls can all be canceled. At the most, only two threads can be canceled 
// or closed at a time, this means that two synchronisation object wil be sufficient
// for operation.
//
// One object for:
// ACCEPT - RECEIVE - CONNECT  Mutual exclusive blocking calls which can not occure at
//                          the same time.
// SEND:                   Mutual exclusive blocking call which can occure parrallel
//                          to the RECEIVE, not to the others.
// 
// This in turn means that the following states should be sufficient.
//  
//           +-----------------+                                       +-----------------+
//   +------>| CLOSED          |<--(!RECEIVE,!SEND,!ACCEPT,!CONNECT)---| SHUTDOWN        |
//   |       +-----------------+                                       +-----------------+
//   |       (CONNECT)   ^                                                    ^    ^
//   |           V   (!CONNECT)                                               |    |
//   |       +-----------------+                                              |    |
// (OPEN)    | CONNECT*        |---(CANCEL,CLOSE)-----------------------------+    |
//   |       +-----------------+                                                   |
//   |             (OPEN)                                                          |
//   |               V                                                             |
//   |       +-----------------+                           +-----------------+     |
//   +------>| OPEN            |<-(!RECEIVE,!SEND,!ACCEPT)-| CANCEL          |<-+  |
//           +-----------------+                           +-----------------+  |  |
//              ^    ^    ^                                                     |  |
//              |    |    +-------(ACCEPT, !ACCEPT)---------+                   |  |
//     (RECEIVE,|    +--(SEND, !SEND)----+                  |                   |  | 
//    !RECEIVE) V                        V                  V                   |  |
//           +-----------------+ +-----------------+ +-----------------+        |  |
//           | RECEIVE*        | | SEND*           | | ACCEPT*         |        |  |
//           +-----------------+ +-----------------+ +-----------------+        |  |
//                   |  |                |  |               |  |                |  |
//                   |  +--(CANCEL)------|--+---------------|--+----------------+  |
//                   |                   |                  |                      |
//                   +-----(CLOSE)-------+------------------+----------------------+
//
// All states have one bit in the state mask, so 8 bits are nessecary.
//
// From the CONNECT, ACCEPT, RECIEVE/SEND bit cannot be simultaniously active.
//
// RECEIVE, SEND, DUPLEX  and ACCEPT state move (using the Cancel interface) to the CANCEL state.
// RECEIVE, SEND, DUPLEX, ACCEPT, OPEN state move (using the Close interface) to the SHUTDOWN state. 
// CONNECT state move (using the Cancel and Close  interface) to the SHUTDOWN state. 
// 
// The states marced with a * are Cancalable and Closable

namespace Solutions { namespace Generics
{

class WinSocketInitializer
{
	public:
		WinSocketInitializer()
		{
#ifdef __WIN32__
			WORD wVersionRequested;
			WSADATA wsaData;
			int err;

			/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
			wVersionRequested = MAKEWORD(2, 2);

			err = WSAStartup(wVersionRequested, &wsaData);
			if (err != 0) {
				/* Tell the user that we could not find a usable */
				/* Winsock DLL.                                  */
				printf("WSAStartup failed with error: %d\n", err);
				exit(1);
			}

			/* Confirm that the WinSock DLL supports 2.2.*/
			/* Note that if the DLL supports versions greater    */
			/* than 2.2 in addition to 2.2, it will still return */
			/* 2.2 in wVersion since that is the version we      */
			/* requested.                                        */

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
				/* Tell the user that we could not find a usable */
				/* WinSock DLL.                                  */
				printf("Could not find a usable version of Winsock.dll\n");
				WSACleanup();
				exit(1);
			}
			else
				printf("The Winsock 2.2 dll was found okay\n");
#endif
		}

		~WinSocketInitializer ()
		{
#ifdef __WIN32__
			/* then call WSACleanup when down using the Winsock dll */
			WSACleanup();
#endif
		}

		const bool IsInitialized () const
		{
			return (true);
		}
};

static WinSocketInitializer	g_SocketInitializer;

//////////////////////////////////////////////////////////////////////
// SocketPort
//////////////////////////////////////////////////////////////////////

SocketPort::Trigger::Trigger (SocketPort& a_Socket) :
#ifndef __WIN32__
  SignalHandler   (),
#endif

#ifdef __POSIX__
  m_EventHandle    ( true, false),
#endif

#ifdef __WIN32__
  m_EventHandle    ( ::WSACreateEvent() ),
#endif

  m_Socket         ( a_Socket ),
  m_EventBits      ( 0 ),
  m_EventMask      ( 0 )
{
  TRACE_L5 ("Constructor SocketPort::Trigger <0x%X>", TRACE_POINTER (this));

#ifdef __WIN32__
  if (m_EventHandle != WSA_INVALID_EVENT)
  {
    TRACE_L4 ("Error on trigger WSAEVENTSELECT. Error %d", ERRORRESULT);
  }
#endif
}

/* virtual */
SocketPort::Trigger::~Trigger ()
{
  TRACE_L5 ("Destructor SocketPort::Trigger <0x%X>", TRACE_POINTER (this));

#ifdef __WIN32__
  // Clean up the EventHandler.
  if (m_EventHandle != WSA_INVALID_EVENT)
  {
    ::WSACloseEvent (m_EventHandle);
  }
#endif
}

#ifdef __POSIX__
/* virtual */ bool
  SocketPort::Trigger::SignalHandler   ()
{
  uint8  l_Result = 0;
  struct pollfd  l_Poll;

  l_Poll.fd     = m_Socket.m_cSocket;
  l_Poll.events = POLLHUP|POLLIN|POLLOUT|POLLNORM;

  if (poll (&l_Poll, 1, 0) > 0)
  {
    l_Result = ( ((l_Poll.revents & POLLHUP)           != 0) ? CLOSE           : 0 ) |
               ( ((l_Poll.revents & (POLLNORM|POLLIN)) != 0) ? RECEIVE|ACCEPT  : 0 ) |
               ( ((l_Poll.revents & POLLOUT)           != 0) ? SEND|CONNECT    : 0 );

    // Register our interesting events as wel...
    m_EventBits |= (l_Result & m_EventMask);

    // Check if someone could still be interesetd in an oustanding action.
    if (m_EventBits != 0)
    {
      // Yip someone is still interested, trigger it.
      SetEvent ();
    }
  }

  // The SIGPOLL might have been intended for more sockets.
  return (m_EventBits != 0);
}
#endif

#ifdef __WIN32__
void
  SocketPort::Trigger::TriggerEvents (uint8 a_Action)
{
  uint32 m_Mask = ( (((a_Action & CONNECT) != 0) ? FD_CONNECT : 0) |
                          (((a_Action & RECEIVE) != 0) ? FD_READ    : 0) |
                          (((a_Action & SEND)    != 0) ? FD_WRITE   : 0) |
                          (((a_Action & CLOSE)   != 0) ? FD_CLOSE   : 0) |
                          (((a_Action & ACCEPT)  != 0) ? FD_ACCEPT  : 0) );

  // Set the calculated mask.
  if (::WSAEventSelect (m_Socket.m_cSocket, m_EventHandle, m_Mask) == SOCKET_ERROR )
  {
    TRACE_L5 ("Error on port socket WSAEVENTSELECT. Error %d", ERRORRESULT);
  }
}
#endif

uint8
	SocketPort::Trigger::PendingTriggers () const
{
	uint8 l_Result;

    // Update the trigger bits.
#ifdef __WIN32__
    WSANETWORKEVENTS  structAction;

    // reset the flag and load the current settings of the network.
    ::WSAEnumNetworkEvents (m_Socket, m_EventHandle, &structAction);

    l_Result = ((structAction.lNetworkEvents & FD_ACCEPT)  ? ACCEPT  : 0) |
               ((structAction.lNetworkEvents & FD_READ)    ? RECEIVE : 0) |
               ((structAction.lNetworkEvents & FD_WRITE)   ? SEND    : 0) |
               ((structAction.lNetworkEvents & FD_CONNECT) ? CONNECT : 0) |
               ((structAction.lNetworkEvents & FD_CLOSE)   ? CLOSE   : 0) ;
#endif

#ifdef __POSIX__
    struct pollfd  l_Poll;

    l_Poll.fd = m_Socket.m_cSocket;
    l_Poll.events = POLLHUP|POLLIN|POLLOUT|POLLNORM;

    if (poll (&l_Poll, 1, 0) > 0)
    {
      l_Result = ( ((l_Poll.revents & POLLHUP)           != 0) ? CLOSE          : 0 ) |
                 ( ((l_Poll.revents & (POLLIN|POLLNORM)) != 0) ? RECEIVE|ACCEPT : 0 ) |
                 ( ((l_Poll.revents & POLLOUT)           != 0) ? SEND|CONNECT   : 0 );
    }
#endif

	return (l_Result);
}

uint8
  SocketPort::Trigger::Check (uint8 a_Action)
{
#ifdef __POSIX__
  m_EventHandle.ResetEvent ();
#endif

#ifdef __WIN32__
  ::ResetEvent (m_EventHandle);
#endif

  uint8 l_Result = PendingTriggers();

  // Register our interesting events as wel...
  m_EventMask |= a_Action;
  m_EventBits |= (l_Result & m_EventMask);
  l_Result     = m_EventBits & a_Action;

  // Check if our bits are there.
  if (l_Result != 0)
  {
    // Yip an event that interests us is there, clear all mask end event bits.
    m_EventMask  ^= a_Action;
    m_EventBits  ^= l_Result;
  }

  // Check if someone could still be interesetd in an oustanding action.
  if (m_EventBits != 0)
  {
    // Yip someone is still interested, trigger it.
    SetEvent ();
  }

  return (l_Result);
}

bool
  SocketPort::Trigger::Wait (uint32 a_Seconds)
{
#ifdef __WIN32__
	return (WaitForSingleObject (m_EventHandle, a_Seconds) == (WAIT_OBJECT_0+1) ? true : false);
#endif

#ifdef __POSIX__
  return (m_EventHandle.Lock (a_Seconds) == 0 ? true : false);
#endif
}

void
  SocketPort::Trigger::SetEvent ()
{
#ifdef __POSIX__
  m_EventHandle.SetEvent ();
#endif
#ifdef __WIN32__
  ::WSASetEvent (m_EventHandle);
#endif
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Client Socket setup
SocketPort::SocketPort(
  const NodeId&          refSocketPort,
  const enum_type        nType, 
  const uint32     nBufferSize) :
    m_Trigger           ( *this ),
    m_syncAdmin         ( ),
    m_LocalNode         ( refSocketPort ),
    m_cSocket           ( INVALID_SOCKET ),
    m_enumState         ( CLOSE ),
    m_enumType          ( nType ),
	m_PendingTriggers   ( 0 ),
	m_BufferSize        ( nBufferSize )
{
	TRACE_L5 ("Constructor SocketPort (NodeId&) <0x%X>", TRACE_POINTER (this));
}

uint32 SocketPort::Accept (SocketPort& serverSocket, NodeId& node)
{
  struct sockaddr_in  l_Remote;
  socklen_t           l_Size  = sizeof (struct sockaddr_in);

  ASSERT (m_cSocket == INVALID_SOCKET);

  if ( (m_cSocket = ::accept (serverSocket.m_cSocket, (struct sockaddr*) &l_Remote, &l_Size)) != SOCKET_ERROR)
  {
	  struct sockaddr_in  l_LocalAddress;
	  bool                l_Initialized = false;
	  socklen_t           l_LocalSize  = sizeof (l_LocalAddress);
	 
      #ifdef __POSIX__ 
	  uint32        l_Value = 1;

	  if (fcntl(m_cSocket, F_SETFL, FASYNC) < 0)
	  {
		TRACE_L4 ("Error on port socket F_SETFL call. Error %d", ERRORRESULT);
	  }
	  else if (fcntl(m_cSocket, F_SETOWN, getpid ()) == -1)
	  {
		TRACE_L1 ("Setting Process ID failed. <%d>", ERRORRESULT);
	  }
	  else
	  #endif

	  #ifdef __DEC__
	  if ( ::ioctl (m_cSocket, FIONBIO, &l_Value) == -1)
	  {
		TRACE_L5 ("Error on port socket NON_BLOCKING call. Error %d", ERRORRESULT);
	  }
	  #endif

	  #ifdef __WIN32__
	  unsigned long        l_Value = 1;

	  if (::ioctlsocket (m_cSocket, FIONBIO, &l_Value) == -1)
	  {
		TRACE_L5 ("Error on port socket NON_BLOCKING call. Error %d", ERRORRESULT);
	  }
	  #endif

	  else if (::getsockname (m_cSocket, (struct sockaddr*) &l_LocalAddress, &l_LocalSize) == SOCKET_ERROR)
	  {
		TRACE_L5 ("Error on port socket GETSOCKETNAME. Error %d", ERRORRESULT);
	  }
	  else
	  {
		l_Initialized = true;
	  }

	  if (l_Initialized == true)
	  {
		SetStatus (OPEN);

		m_LocalNode = l_LocalAddress;
		node = l_Remote;
	  }
	  else if (m_cSocket != INVALID_SOCKET)
	  {
		SocketPort::DestroySocket(m_cSocket);

		m_cSocket = INVALID_SOCKET;
	  }
  }
  return (m_cSocket == INVALID_SOCKET ? ERROR_ILLEGAL_STATE : OK);
}

SocketPort::~SocketPort()
{
  TRACE_L5 ("Destructor SocketPort <0x%X>", TRACE_POINTER (this));

  if (m_cSocket != INVALID_SOCKET)
  {
    // If there were no outstanding transactions, we can close.
    Close (true);

    m_enumState.WaitState (CLOSE, INFINITE);   
  }
}

/* static */ void SocketPort::DestroySocket (SOCKET& socket)
{
#ifdef __WIN32__
	::closesocket (socket);
#endif

#ifdef __POSIX__
	::close (socket);
#endif
}

/* static */ SOCKET SocketPort::CreateSocket (const uint32 mode, const uint32 bufferSize)
{
	SOCKET               l_Result        = INVALID_SOCKET;

#ifdef __POSIX__
	unsigned long        l_Value         = 1;
#endif

	if ( (g_SocketInitializer.IsInitialized() == false) || ((l_Result = ::socket (AF_INET, mode, 0)) == INVALID_SOCKET) )
	{
		TRACE_L4 ("Error on creating socket SOCKET. Error %d", ERRORRESULT);
	}

#ifdef __DEC__
	// Probably not nessecary but tested with this.
	else if ( ::ioctl (l_Result, FIONBIO, &l_Value) == -1)
	{
		TRACE_L4 ("Error on port socket NON_BLOCKING call. Error %d", ERRORRESULT);
	} 
#endif

#ifdef __POSIX__
	else if (fcntl(l_Result, F_SETOWN, getpid ()) == -1)
	{
		TRACE_L1 ("Setting Process ID failed. <%d>", ERRORRESULT);
	}
	else if (fcntl(l_Result, F_SETFL, FASYNC) < 0) 
	{
		TRACE_L4 ("Error on port socket F_SETFL call. Error %d", ERRORRESULT);
	}
#endif

	else if (bufferSize == 0)
	{
		return (l_Result);
	}
	else if (::setsockopt (l_Result, SOL_SOCKET, SO_RCVBUF, (const char *) &bufferSize, sizeof(int)) == SOCKET_ERROR)
	{
		TRACE_L5 ("Error could not set Receive buffer size.", NULL);
	}
	else if (::setsockopt (l_Result, SOL_SOCKET, SO_SNDBUF, (const char *) &bufferSize, sizeof(int)) == SOCKET_ERROR)
	{
		TRACE_L5 ("Error could not set Send buffer size.", NULL);
	}
	else
	{
		// Seems like the socket is created...
		return (l_Result);
	}

	if (l_Result != INVALID_SOCKET)
	{
		SocketPort::DestroySocket(l_Result);
	}

	return (INVALID_SOCKET);
}

SOCKET SocketPort::ConstructSocket (const uint32 mode, const uint32 bufferSize, NodeId& localNode)
{
	SOCKET               l_Result        = SocketPort::CreateSocket(mode, bufferSize);
	struct sockaddr_in   l_LocalAddress  = localNode;
	socklen_t            l_LocalSize     = sizeof (l_LocalAddress);

	if (l_Result == INVALID_SOCKET)
	{
		TRACE_L4 ("Error on port socket BIND. Error %d", ERRORRESULT);
	}
	else if (::bind (l_Result, (struct sockaddr*) &l_LocalAddress, l_LocalSize) == SOCKET_ERROR)
	{
		TRACE_L4 ("Error on port socket BIND. Error %d", ERRORRESULT);
	}
	else if (::getsockname (l_Result, (struct sockaddr*) &l_LocalAddress, &l_LocalSize) == SOCKET_ERROR)
	{
		TRACE_L5 ("Error on port socket GETSOCKETNAME. Error %d", ERRORRESULT);
	}
	else
	{
		localNode = l_LocalAddress;
		return (l_Result);
	}

	SocketPort::DestroySocket(l_Result);

	return (INVALID_SOCKET);
}

SOCKET SocketPort::FreeSocket (const uint32 mode, const uint32  bufferSize, NodeId& localNode, const uint16 maxPorts)
{
	SOCKET               l_Result        = SocketPort::CreateSocket(mode, bufferSize);

	if (l_Result != INVALID_SOCKET)
	{
		bool				succeeded     = false;
		struct sockaddr_in  localAddress  = localNode;
		socklen_t           localSize     = sizeof (localAddress);
		uint16				ports         = maxPorts;

		while ((ports != 0) && (succeeded == false))
		{
			succeeded = (::bind (l_Result, (struct sockaddr*) &localAddress, localSize) != SOCKET_ERROR) &&
			            (::getsockname (l_Result, (struct sockaddr*) &localAddress, &localSize) != SOCKET_ERROR);

			if (succeeded == false)
			{
				// Lets try the next one if it is available..
				localAddress.sin_port++;
				ports--;
			}
		}

		if (succeeded == true)
		{
			localNode = localAddress;
		}
		else
		{
			SocketPort::DestroySocket(l_Result);
			l_Result = INVALID_SOCKET;
		}
	}


	return (l_Result);
}

void 
SocketPort::Register ()
{
  BYTE l_State = m_enumState;

#ifdef __WIN32__
  if ( (l_State & OPEN) != 0)
  {
    if (m_enumType == LISTEN)
    {
      m_Trigger.TriggerEvents (ACCEPT|CLOSE);
    }
    else
    {
      m_Trigger.TriggerEvents (RECEIVE|SEND|CLOSE);
    }
  }
  else
  {
    m_Trigger.TriggerEvents (CONNECT|CLOSE);
  }
#endif

#ifdef __POSIX__
  if ( ((l_State & CONNECT) != 0) || (m_enumType != STREAM) );
  {
    Signals::Instance ().Subscribe (SIGPOLL, m_Trigger);

    if (fcntl(m_cSocket, F_SETFL, FASYNC) < 0)
    {
      TRACE_L4 ("Error on port socket F_SETFL call. Error %d", ERRORRESULT);
    }
  }
#endif
}

void 
SocketPort::Unregister ()
{
#ifdef __WIN32__
  m_Trigger.TriggerEvents (0);
#endif

#ifdef __POSIX__
  // Before we delete the descriptor, get ride of the Trigger
  // subscribtion.
  Signals::Instance ().Unsubscribe (SIGPOLL, m_Trigger);

  if (fcntl(m_cSocket, F_SETFL, 0) < 0)
  {
    TRACE_L4 ("Error on port socket F_SETFL call. Error %d", ERRORRESULT);
  }
#endif

  ::shutdown (m_cSocket, SD_BOTH);
}

uint32 SocketPort::ReadBytes () const
{
	uint32 result = 0;

	if ((m_cSocket == INVALID_SOCKET) || 
#ifdef __WIN32__
		( ::ioctlsocket (m_cSocket, FIONREAD, &result) == SOCKET_ERROR))
#else
		( ::ioctl (m_cSocket, FIONREAD, &result) == -1))
#endif
	{
		result = WSAGetLastError ();
		result = 0;
	} 

	return (result);
}

uint32
  SocketPort::BufferSize (
    bool                a_read) const
{
	uint32 l_Result = 0;
	socklen_t    l_Size   =  sizeof(l_Result);

	if (m_cSocket == INVALID_SOCKET)
	{
		l_Result = m_BufferSize;
	}
	else
	{
		if (a_read == true)
		{
			if (::getsockopt (m_cSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*> (&l_Result), &l_Size) == SOCKET_ERROR)
			{
				TRACE_L5 ("Error could not set Receive buffer size.", NULL);
				l_Result = 0;
			}
		}
		else
		{
			if (::getsockopt (m_cSocket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*> (&l_Result), &l_Size) == SOCKET_ERROR)
			{
				TRACE_L5 ("Error could not set Send buffer size.", NULL);
				l_Result = m_BufferSize;
			}
		}
	}

	return (l_Result);
}

bool
SocketPort::SetStatus (uint8 a_Action)
{
  // VERIFY ALL PRECONDITIONS USING ASSERTS.
  BYTE l_State = m_enumState;

  // Not all states can be set. Check the ones we are allowed to set.
  ASSERT ( (a_Action == ACCEPT) || (a_Action == RECEIVE) || (a_Action == SEND)    || 
           (a_Action == OPEN)   || (a_Action == CLOSE)   || (a_Action == CONNECT) ||
           (a_Action == CANCEL) );

  // If we are in the LISTEN mode, only ACCEPT and OPEN is allowed.
  ASSERT (m_enumType == LISTEN   ? (a_Action & (RECEIVE|SEND|CONNECT)) == 0 : true);

  // If we are in the DATAGRAM mode, only RECEIVE, SEND and OPEN is allowed.
  ASSERT (m_enumType == DATAGRAM ? (a_Action & (ACCEPT|CONNECT)) == 0 : true);

  // If we are in the STREAM mode, only CONNECT, RECEIVE, SEND and OPEN is allowed.
  ASSERT (m_enumType == DATAGRAM ? (a_Action & (ACCEPT)) == 0 : true);

  // If an action bit is active, the open bit cannot be active, and if open or active bit are 
  // active, the close/connect bit cannot be active..
  ASSERT (((m_enumState & (CLOSE|CONNECT)) != 0) ^ (((m_enumState & OPEN) != 0) ^ ((m_enumState & (RECEIVE|SEND|ACCEPT)) != 0)));

  // There should be a socket available
  ASSERT (m_cSocket != INVALID_SOCKET);

  // NO PRECODITION VIOLATION. CONTINUE....

  // This big state change check.
  bool l_Result = (

       // A state can only be set once.
       ((l_State & a_Action) == 0)                                                                   &&

       // If we are in an exceptional state, we cannot move to action states.
       ((l_State & (SHUTDOWN|CANCEL)) != 0 ? (a_Action & (ACCEPT|RECEIVE|SEND|CONNECT)) == 0 : true) &&

       // If we are in the shutdown state, the Cancel state is not allowed. 
       ((l_State & (SHUTDOWN)) != 0 ? (a_Action & CANCEL) == 0 : true)                               &&

       // Check for the CLOSED state, if CLOSED, only OPEN and CONNECT is Allowed
       ((l_State & (CLOSE|CONNECT)) != 0 ? (a_Action & (SEND|RECEIVE|ACCEPT)) == 0 : 
                                           (a_Action & (OPEN|CONNECT)) == 0        ) );

  // See if it is allowed to set the bit.
  if ( l_Result != false )
  {
    // Ther is no exceptional situation going on, we filtered it out.
    switch (a_Action)
    {
      case OPEN:     // It seems we opened up, let's move to the open state.
                     m_enumState.SetState ((l_State & ~(CONNECT|ACCEPT|SEND|RECEIVE|CLOSE)) | OPEN);
                     Register             ();
					 //-------------------------------
					 //TODO: --- Trigger Opened up ---
					 //-------------------------------
                     break;
      case CONNECT:  // It seems we are about to try to connect. We leave the CLOSE state
                     m_enumState.SetState ((l_State & ~(CLOSE))|CONNECT);
                     Register             ();
                     break;
      case RECEIVE:
					 //-------------------------------
					 //TODO: --- Trigger a receive ---
					 //-------------------------------
      case SEND:
      case ACCEPT:   // We are about to do some action on the port. 
                     m_enumState.SetState ((l_State & ~(OPEN))|a_Action);
                     break;
      case CANCEL:   // Is ther action going on...
                     if ((l_State & (ACCEPT|SEND|RECEIVE)) != 0 )
                     {
                       // Yes, this means we have to delay the cancel.
                       m_enumState.SetState (l_State | CANCEL);
                     }
                     else if ((l_State & CONNECT) != 0)
                     {
                       // Canceling a CONNECT means a close.
                       m_enumState.SetState (l_State | SHUTDOWN);
                     } 
                     break;
      case CLOSE:    // This might lead to excceptional situations. Check it.
                     if ((l_State & (CONNECT|ACCEPT|SEND|RECEIVE)) != 0 )
                     {
                       // Seems like some action is still going on. Set The 
                       m_enumState.SetState (l_State | SHUTDOWN);
                     }
                     else if ((l_State & OPEN) != 0)
                     {
                       // Actually unregister and stop listening.
                       Unregister ();

                       m_enumState.SetState (CLOSE);
                     }
                     break;
      default:       ASSERT (false);
    }
  }

  // Give back if we could change the state.
  return (l_Result);
}

void
SocketPort::ClearStatus (uint8 a_Action)
{
  // VERIFY ALL PRECONDITIONS USING ASSERTS.

  // The state close can not be set or cleared, it is calculated
  ASSERT ( (a_Action == ACCEPT) || (a_Action == RECEIVE) || (a_Action == SEND) || (a_Action == CONNECT) );

  // If we are in the LISTEN mode, only ACCEPT and OPEN is allowed.
  ASSERT (m_enumType == LISTEN   ? (a_Action & (RECEIVE|SEND|CONNECT)) == 0 : true);

  // If we are in the DATAGRAM mode, only RECEIVE, SEND and OPEN is allowed.
  ASSERT (m_enumType == DATAGRAM ? (a_Action & (ACCEPT|CONNECT)) == 0 : true);

  // If we are in the STREAM mode, only CONNECT, RECEIVE, SEND and OPEN is allowed.
  ASSERT (m_enumType == DATAGRAM ? (a_Action & (ACCEPT)) == 0 : true);

  // If an action bit is active, the open bit cannot be active, and if open or active bit are 
  // active, the close/connect bit cannot be active..
  ASSERT (((m_enumState & (CLOSE|CONNECT)) != 0) ^ (((m_enumState & OPEN) != 0) ^ ((m_enumState & (RECEIVE|SEND|ACCEPT)) != 0)));

  // NO PRECODITION VIOLATION. CONTINUE....
  BYTE   l_State;

  switch (a_Action)
  {
    case CONNECT:  // Seems like the connection failed, fall back to CLOSED
                   Unregister ();

                   m_enumState.SetState (CLOSE);
                   break;
    case ACCEPT:   
    case RECEIVE:
    case SEND:     // Turn off the proper bit.
                   l_State = m_enumState & ~(a_Action);
                   
                   // Check exception bits for exceptional processing.
                   if ((l_State & (ACCEPT|SEND|RECEIVE)) == 0)
                   {
                     l_State = OPEN|(m_enumState&SHUTDOWN);

                     // Seems like there is no more activity, check exceptions....
                     if ((l_State & SHUTDOWN) != 0)
                     {
                       // Actually unregister and stop listening.
                       Unregister ();

                       l_State = CLOSE;
                     }
                     else
                     {
                       // Only set the stae once, or we will create a deadlock.
                       l_State = OPEN;
                     }
                   }

                   m_enumState.SetState (l_State);
                   break;
    default:       ASSERT (false);
  }
}

uint32
SocketPort::Abort ()
{
  uint32 nStatus = ERROR_ILLEGAL_STATE;

  // Make sure the state does not change in the mean time.
  m_syncAdmin.Lock ();

  if ( SetStatus (CANCEL) == true )
  {
    // Make sure that all blocking actions get released.
    if ( (m_enumState & (OPEN|CONNECT|ACCEPT|SEND|RECEIVE)) != 0 )
    {
      m_Trigger.SetEvent ();
    }

    // Done working on the object. Release it.
    m_syncAdmin.Unlock ();

    // I guess we succeeded.
    nStatus = OK;
  }
  else
  {
    // Done working on the object. Release it.
    m_syncAdmin.Unlock ();
  }

  return (nStatus);
}

// If the blForced boolean is set, the close cancels all outstanding 
// requests and waits till the connection is really terminated.
// If the flag is false, a Shutdown request is issued but a cancel isi
// not issued and it does not wait until the stage CLOSED is reached.
uint32
SocketPort::Close (uint32 a_Time)
{
  uint32 nStatus = ERROR_ILLEGAL_STATE;

  // Make sure the state does not change in the mean time.
  m_syncAdmin.Lock ();

  // If the state is already close, nothing to be done.
  if ( SetStatus (CLOSE) == true )
  {
    if ( ((m_enumState & (CONNECT|SEND|RECEIVE)) != 0) || (a_Time == ~0) )
    {
      SocketPort::DestroySocket(m_cSocket);

	  // Seems like there is still something going on. Cancel it.
      m_Trigger.SetEvent ();
	}

    // I guess we succeeded.
    nStatus = OK;
  }

  // Done with the states
  m_syncAdmin.Unlock ();

  return (nStatus);
}

uint32 SocketPort::Open (const uint16 portRange)
{
	uint32 nStatus = ERROR_ILLEGAL_STATE;

	// Make sure the state does not change in the mean time.
	m_syncAdmin.Lock ();

	if ((m_enumState & OPEN) == 0)
	{
		ASSERT (m_cSocket == INVALID_SOCKET);
		ASSERT (portRange != 0);

		if (portRange == 1)
		{
			m_cSocket = ConstructSocket ((m_enumType == LISTEN ? SOCK_STREAM : m_enumType), m_BufferSize, m_LocalNode);
		}
		else
		{
			m_cSocket = FreeSocket ((m_enumType == LISTEN ? SOCK_STREAM : m_enumType), m_BufferSize, m_LocalNode, portRange);
		}

		switch (m_enumType)
		{
		case DATAGRAM:
			{
				if (SetStatus (OPEN) == true)
				{
					nStatus = OK;
				}
				break;
			}
		case STREAM:
			{
				// PLease use the open with a remote node Id.
				ASSERT (false);
				break;
			}
		case LISTEN:
			{
				if (SetStatus (OPEN) == true)
				{
					if (::listen (m_cSocket, 5) == SOCKET_ERROR)
					{
						TRACE_L5 ("Error on port socket LISTEN. Error %d", ERRORRESULT);
						SetStatus (CLOSE);
					}
					else
					{
						nStatus  = OK;
					}
				}
				break;
			}
		}
	}

	// Done, state changed.
	m_syncAdmin.Unlock ();

	return (nStatus);
}

uint32 SocketPort::Open (const NodeId& a_Remote, uint32 a_Time)
{
	uint32 nStatus = ERROR_ILLEGAL_STATE;

	// Make sure the state does not change in the mean time.
	m_syncAdmin.Lock ();

	if ((m_enumState & OPEN) == 0)
	{
		if (SetStatus (CONNECT) == true) 
		{
			if (m_cSocket == INVALID_SOCKET)
			{
				m_cSocket = SocketPort::CreateSocket (SOCK_STREAM, m_BufferSize);
			}

			// The request is queued, now let's wait for the response.
			m_syncAdmin.Unlock ();

			nStatus = Connect (a_Remote);

			while ((a_Time != 0) && (nStatus == ERROR_ILLEGAL_STATE))
			{
				// Wait for some IO action to complete or we are interrupted.
				nStatus = WaitForAction (CONNECT, a_Time);

				if (nStatus == OK)
				{
					// If we try to connect again, we should get "ALREADY CONNECTED"
					// Then it it O.K. otherwise we have an error.
					if (Connect (a_Remote) == ERROR_ALREADY_CONNECTED)
					{
						nStatus = OK;
					}
				}
				// Turn off the action we were doing.
				if (nStatus == OK)
				{
					SetStatus (OPEN);
				}
				else
				{
					ClearStatus (CONNECT);
				}
			}
		}
	}

	// Done, state changed.
	m_syncAdmin.Unlock ();

	return (nStatus);
}

uint32 SocketPort::Connect (const struct sockaddr_in& l_Remote)
{
  uint32   nStatus = ERROR_ILLEGAL_STATE;

  if (::connect (m_cSocket, (const struct sockaddr*) &l_Remote, sizeof (struct sockaddr_in)) == SOCKET_ERROR)
  {
    uint32 l_Result = ERRORRESULT;

    if ( l_Result == ERROR_ISCONN)
    {
      nStatus = ERROR_ALREADY_CONNECTED;
    }
    else if ( (l_Result != ERROR_WOULDBLOCK) && (l_Result != ERROR_INPROGRESS) )
    {
      TRACE_L5 ("SocketPort::Connect Error %d\n", ERRORRESULT);

      nStatus = ERROR_ASYNC_FAILED;
    }
  }
  else
  {
    nStatus = OK;
  }

  return (nStatus);
}

uint32 SocketPort::Write (const NodeId& a_Remote, uint32& a_Size, const BYTE a_Bytes[])
{
  uint32               l_Status = ERROR_ILLEGAL_STATE;
  uint32               l_Size   = a_Size;
  const struct sockaddr_in&  l_Remote = a_Remote;

  // This needs to be a blocking call that has the ability to be interupted.
  if ( (l_Size = ::sendto (m_cSocket,
                reinterpret_cast <const char*> (&a_Bytes[0]),
                l_Size, 0,
                (struct sockaddr*) &l_Remote,
                sizeof (struct sockaddr_in))) == SOCKET_ERROR)
  {
    uint32 l_Result = ERRORRESULT;

    if ( (l_Result != ERROR_WOULDBLOCK) && (l_Result != ERROR_INPROGRESS) )
    {
      TRACE_L5 ("SocketPort::Send Error %d\n", ERRORRESULT);

      l_Status = ERROR_ASYNC_FAILED;
    }
  }
  else
  {
    a_Size  = a_Size - l_Size;
    l_Status = OK;

	  // Seems like everything succeeeded right away, trigger any waitng process for this..
      m_Trigger.SetEvent ();
  }

  return (l_Status);
}

uint32 SocketPort::Read (NodeId& a_Remote, uint32& a_Size, BYTE a_Bytes[])
{
  uint32               l_Size;
  struct sockaddr_in         l_Remote;
  uint32               l_Status  = ERROR_ILLEGAL_STATE;
  socklen_t                  l_Address = sizeof (l_Remote);

  // Read the actual data from the port.
  if ( (l_Size = ::recvfrom (m_cSocket,
                  reinterpret_cast <char*> (&a_Bytes[0]),
                  a_Size, 0, (struct sockaddr*) &l_Remote, 
                  &l_Address)) == SOCKET_ERROR)
  {
    uint32 l_Result = ERRORRESULT;

    if ( (l_Result != ERROR_WOULDBLOCK) && (l_Result != ERROR_INPROGRESS) )
    {
      TRACE_L5 ("SocketPort::Receive Error %d\n", ERRORRESULT);

      l_Status = ERROR_ASYNC_FAILED;
    }
  }
  else
  {
    a_Remote = l_Remote;
    a_Size   = a_Size - l_Size;
    l_Status = OK;

  // Seems like everything succeeeded right away, trigger any waitng process for this..
  m_Trigger.SetEvent ();

  }

  return (l_Status);
}

uint32 SocketPort::Send (const NodeId& a_Remote, uint32& a_Size, const BYTE a_Bytes[], uint32 a_Time)
{
  uint32 nStatus = ERROR_ILLEGAL_STATE;

  // Make sure the state does not change in the mean time.
  m_syncAdmin.Lock ();

  if (SetStatus (SEND) == true) 
  {
    // The request is queued, now let's wait for the response.
    m_syncAdmin.Unlock ();

    // We need to know where the next data should be dropped.
    int l_Size   = a_Size;

    nStatus = Write (a_Remote, a_Size, a_Bytes);

	if (a_Time != 0)
	{
		while ( nStatus == ERROR_ILLEGAL_STATE )
		{
		  // Wait for some IO action to complete or we are interrupted.
		  nStatus = WaitForAction (SEND, a_Time);

		  if (nStatus == OK)
		  {
			nStatus = Write (a_Remote, a_Size, &a_Bytes[l_Size - a_Size]);
		  }
		}
	}

    // Make it atomic
    m_syncAdmin.Lock ();

    // Turn off the SEND bit, it is available again.
    ClearStatus (SEND);
  }

  // Unlock the state change actions.
  m_syncAdmin.Unlock ();

  return (nStatus);
}

uint32
SocketPort::Receive (NodeId& a_Remote, uint32& a_Size, BYTE a_Bytes[], uint32 a_Time)
{
  uint32 nStatus = ERROR_ILLEGAL_STATE;

  // Make sure the state does not change in the mean time.
  m_syncAdmin.Lock ();

  if (SetStatus (RECEIVE) == true)
  {
    // The request is queued, now let's wait for the response.
    m_syncAdmin.Unlock ();

    // We need to know where the next data should be dropped.
    int l_Size   = a_Size;

    nStatus = Read (a_Remote, a_Size, a_Bytes);

    while ( (a_Time != 0) && (nStatus == ERROR_ILLEGAL_STATE) )
    {
      // Wait for some IO action to complete or we are interrupted.
      nStatus = WaitForAction (RECEIVE, a_Time);

      if (nStatus == OK)
      {
        nStatus = Read (a_Remote, a_Size, &a_Bytes[l_Size - a_Size]);
      }
    }

    // Make it atomic
    m_syncAdmin.Lock ();

    // Turn off the RECEIVE bit, it is available again.
    ClearStatus (RECEIVE);
  }

  // Unlock the state change actions.
  m_syncAdmin.Unlock ();

  return (nStatus);
}

uint32
  SocketPort::Accept (
    SocketPort&  a_Socket,
	NodeId& a_Remote,
    uint32 a_Time)
{
  uint32        l_Status = ERROR_ILLEGAL_STATE;

  ASSERT (m_enumType == LISTEN);

  // Make sure the state does not change in the mean time.
  m_syncAdmin.Lock ();

  if ( SetStatus (ACCEPT) == true )
  {
    // The request is queued, now let's wait for the response.
    m_syncAdmin.Unlock ();

    l_Status = a_Socket.Accept (*this, a_Remote);

    while ((a_Time != 0) && (l_Status == ERROR_ILLEGAL_STATE))
    {
      // Wait for some IO action to complete or we are interrupted.
      l_Status = WaitForAction (ACCEPT, a_Time);

      if (l_Status == OK)
      {
		  l_Status = a_Socket.Accept (*this, a_Remote);
      }
    }

    // Make it atomic
    m_syncAdmin.Lock ();

    // Turn off the ACCEPT mode bit, it is available again.
    ClearStatus (ACCEPT);
  }

  // Unlock the state change actions.
  m_syncAdmin.Unlock ();

  return (l_Status);
}

uint32
SocketPort::WaitForAction (uint8 a_ActionType, uint32 a_Time)
{
  uint32 l_Status = ERROR_ILLEGAL_STATE;

  // Get the lock before we can work on states and mask/event bits 
  m_syncAdmin.Lock ();

  do
  {
    if ((m_enumState & (SHUTDOWN|CLOSE)) != 0)
    {
      l_Status = ERROR_CONNECTION_CLOSED;
    }
    else if ( (m_enumState & CANCEL) != 0)
    {
      l_Status = ERROR_ASYNC_FAILED;
    }
    else
    {
      // We are interested in the CLOSE and in the passed in value.
      uint8 l_Action = m_Trigger.Check (a_ActionType|CLOSE);

      // Close is quite effective, goto exceptional mode, close the connection.
      if ((l_Action & CLOSE) != 0)
      {
        // It was a close from the other side.
        Close (true);
      }
      else if (l_Action != 0)
      {
        // See if we can escape it.
        l_Status = OK;
      }
      else
      {
        // About to wait for external triggers.

        // Relinguis the administration lock, we have to wait for our bit to 
        // be turned on.
        m_syncAdmin.Unlock ();

        if (m_Trigger.Wait (a_Time) == false)
        {
          l_Status = ERROR_TIMEDOUT;
        }

        // IO action (or exceptions) have occured, get the lock.
        m_syncAdmin.Lock ();
      }
    }

  } while (l_Status == ERROR_ILLEGAL_STATE);

  // Done clearing the bit.
  m_syncAdmin.Unlock ();

  return (l_Status); 
}

const NodeId SocketPort::RemoteNode () const
{
  NodeId result;

  if ((m_cSocket != INVALID_SOCKET) && ((m_enumState & OPEN) != 0))
  {
	  struct sockaddr_in  l_Remote;
	  socklen_t           l_Size = sizeof (l_Remote);

	  if (::getpeername (m_cSocket, (struct sockaddr*) &l_Remote, &l_Size) == SOCKET_ERROR)
	  {
		TRACE_L5 ("Error on port socket GETPEERNAME. Error %d", ERRORRESULT);
	  }
	  else
	  {
		result = NodeId(l_Remote);
	  }
  }

  return (result);
}

#pragma warning (disable : 4100)

/* virtual */uint32 SocketListner::Send (uint32& nSize, const uint8 szBytes[], uint32 nTime)
{
	// No way these sockets can send or receive !!!
	ASSERT (false);

	return (0);
}

/* virtual */ uint32 SocketListner::Receive (uint32& nSize, uint8 szBytes[], uint32 nTime)
{
	// No way these sockets can send or receive !!!
	ASSERT (false);

	return (0);
}

#pragma warning (default : 4100)

} } // namespace Solutions::Generics
