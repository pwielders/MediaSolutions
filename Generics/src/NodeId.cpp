// SocketPortSettings.cpp: implementation of the NodeId class.
//
//////////////////////////////////////////////////////////////////////

#include "Portability.h"
#include "String.h"

#ifdef __UNIX__
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#define ERRORRESULT        errno
#endif

#ifdef __WIN32__
#define ERRORRESULT        ::WSAGetLastError ()
#endif

#include "NodeId.h"

namespace Solutions { namespace Generics
{

// By default this will be filled with an invalid address.
static NodeId      g_LocalHost;
static NodeId      g_LoopBack(_T("127.0.0.1"));

//----------------------------------------------------------------------------
// CONSTRUCTOR & DESTRUCTOR
//----------------------------------------------------------------------------
 NodeId::NodeId ()
{
  // Fill it with FF's
  memset (&m_structInfo, 0xFF, sizeof (m_structInfo));
}

NodeId::NodeId (const struct sockaddr_in& rInfo)
{
  // Clear all private member variables before using them.
  memcpy (&m_structInfo, &rInfo, sizeof(m_structInfo));
}

NodeId::NodeId (
  const TCHAR         strHostName[], 
  const unsigned int  nPortNumber)
{
  // Convert the name to an IP structure address
  Resolve    (strHostName);

  ASSERT (nPortNumber < 0xFFFF);

  unsigned short result = (unsigned short)(nPortNumber);

  // Set the Port number used.
  m_structInfo.sin_port = htons(result);
}

NodeId::NodeId (
  const TextFragment& strHostName, 
  const unsigned int  nPortNumber)
{
  // Convert the name to an IP structure address
  Resolve    (strHostName.Text().c_str());

  ASSERT (nPortNumber < 0xFFFF);

  unsigned short result = (unsigned short)(nPortNumber);

  // Set the Port number used.
  m_structInfo.sin_port = htons(result);
}

NodeId::NodeId (const NodeId& rInfo)
{
  *this = rInfo;
}

//----------------------------------------------------------------------------
// PUBLIC METHODS
//----------------------------------------------------------------------------

bool
  NodeId::operator== (const NodeId& rInfo) const
{
  return ( (m_structInfo.sin_addr.s_addr == rInfo.m_structInfo.sin_addr.s_addr) &&
           (m_structInfo.sin_port        == rInfo.m_structInfo.sin_port) );
}

NodeId&
  NodeId::operator= (const NodeId& rInfo)
{
  // Copy the struct info
  memcpy (&m_structInfo, &rInfo.m_structInfo, sizeof(m_structInfo));

  // Give back our-selves.
  return (*this);
}    

NodeId&
  NodeId::operator= (const struct sockaddr_in& rInfo)
{
  // Copy the struct info
  memcpy (&m_structInfo, &rInfo, sizeof(m_structInfo));

  // Give back our-selves.
  return (*this);
}    

bool 
  NodeId::Valid () const
{
  unsigned long  l_Value = static_cast <unsigned long>  (-1);

  return (memcmp (&(m_structInfo.sin_addr.s_addr), &l_Value, 4) != 0);
}

bool
	NodeId::IsLocal () const
{
	return ((g_LoopBack == *this) || (NodeId::LocalId().HostName() == this->HostName()));
}

String
  NodeId::URL () const
{
  TCHAR portNumber[6];

  // Lets resolve the port number
  _stprintf_s (portNumber, _T("%d"), ntohs(m_structInfo.sin_port));

  return (HostName() + _T(":") + portNumber);
}

String
  NodeId::HostName () const
{
  String result;

  if (Valid () == true)
  {
    unsigned int nAddress = m_structInfo.sin_addr.s_addr;

    // It seems we had the IP number, find the name that goes with this IP number.
    struct hostent* ptrHostId = gethostbyaddr (reinterpret_cast <char *> (&nAddress),
                                               sizeof(m_structInfo), 
                                               AF_INET);

    if ( (ptrHostId != NULL) && (ptrHostId->h_addrtype == AF_INET) )
    {
      if (ptrHostId->h_name != NULL )
      {
		unsigned int EndIndex = static_cast<unsigned int> (strlen (ptrHostId->h_name));

		if (EndIndex > 0)
		{
			// Found a name, return it.
			ToString(ptrHostId->h_name, result);
		}
      }
    }

	if ( result.empty() == true )
    {
      result = HostAddress();
	}
  }

  return (result);
}

String
  NodeId::HostAddress () const
{
  String result;

  if (Valid () != false)
  {
    unsigned long  nAddress = m_structInfo.sin_addr.s_addr;

	  TCHAR identifier[16];

      // Lets resolve it four digits.
	  _stprintf_s (identifier, _T("%d.%d.%d.%d"), ((nAddress & 0x000000FF) >>  0),
                       ((nAddress & 0x0000FF00) >>  8),  
                       ((nAddress & 0x00FF0000) >> 16),  
                       ((nAddress & 0xFF000000) >> 24));

	  result = String(identifier);
  }

  return (result);
}

/* static */ const NodeId&
  NodeId::LocalId ()
{
  if (g_LocalHost.Valid () == false)
  {
    String newName;
    char l_Buffer[1024];

    if (::gethostname (l_Buffer, 1024) != 0)
    {
      TRACE_L5 ("Failed to extract local machine name !! %d", ERRORRESULT);
    }
	
	// Found a name, return it.
	ToString(l_Buffer, newName);

	g_LocalHost = NodeId (newName.c_str(), 0);
  }

  return (g_LocalHost);
}
    
NodeId
  NodeId::Broadcast () const
{ 
  struct sockaddr_in  l_Result;
  unsigned long     l_Address = m_structInfo.sin_addr.s_addr;

  memcpy (&l_Result, &m_structInfo, sizeof (l_Result));

  // The broadcast address is formed by setting the host portion
  // of a network address to all 1's
  if ((l_Address & 0x80000000L) == 0)
  {
    // CLASS A network.
    l_Address |= 0xFFFFFF00L;
  }
  else if ((l_Address & 0x40000000L) == 0)
  {
    // CLASS B network.
    l_Address |= 0xFFFF0000L;
  }
  else
  {
    // CLASS C network.
    l_Address |= 0xFF000000L;
  }

  l_Result.sin_addr.s_addr = l_Address;

  return (NodeId(l_Result));
}

//----------------------------------------------------------------------------
// PROTECTED METHODS
//----------------------------------------------------------------------------

void
  NodeId::Resolve (const TCHAR strHostName[])
{
  std::string  text; ToString(&strHostName[0], text);
  m_structInfo.sin_family      = AF_INET;
  m_structInfo.sin_addr.s_addr = inet_addr(const_cast <char*> (text.c_str()));

  if (Valid () == false)
  {
	  struct hostent* ptrHostId = ::gethostbyname (text.c_str());

    if ( (ptrHostId != NULL) && (ptrHostId->h_addrtype == AF_INET) )
    {
      if (ptrHostId->h_length == 4 )
      { 
        // Copy the found address into the struct.
        memcpy (&m_structInfo.sin_addr.s_addr, ptrHostId->h_addr_list[0], ptrHostId->h_length);  
      }
    }
    else
    {
      TRACE_L5 ("Error %d", ERRORRESULT);
    }
  }
}

} } // namespace Solutions::Generics
