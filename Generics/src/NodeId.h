// NodeId.h: interface for the NodeId class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __NODEID_H
#define __NODEID_H

#include "Module.h"
#include "String.h"

#ifdef __WIN32__
#include "winsock2.h"
#pragma comment(lib, "wsock32.lib")
#endif

#ifdef __UNIX__
#include <netinet/in.h>
#endif

namespace Solutions { namespace Generics
{
	class EXTERNAL NodeId
	{
	  //------------------------------------------------------------------------
	  // Constructors/Destructors
	  //------------------------------------------------------------------------
	  public:
		NodeId ();
		NodeId (const struct sockaddr_in& rInfo);
		NodeId (const TCHAR strHostName[], const unsigned int nPortNumber = 0);
		NodeId (const TextFragment& strHostName, const unsigned int nPortNumber = 0);
		NodeId (const NodeId& rInfo);

	  //------------------------------------------------------------------------
	  // Public Methods
	  //------------------------------------------------------------------------
	  public:
		inline uint16            PortNumber          () const { return (ntohs(m_structInfo.sin_port));    }
		inline void              PortNumber          (const uint16 portNumber) { m_structInfo.sin_port = ntohs(portNumber); }

		String                   HostName            () const;
		void                     HostName            (const TCHAR strHostName[]);

		String	                 HostAddress         () const;
		String	                 URL                 () const;
		NodeId					 Broadcast           () const;
		bool                     Valid               () const;
		bool					 IsLocal			 () const;

		inline operator const    struct sockaddr_in& () const { return (m_structInfo); }

		bool                     operator==          (const NodeId& rInfo) const;    
		inline bool              operator!=          (const NodeId& rInfo) const { return (!NodeId::operator== (rInfo)); }

		NodeId&                  operator=           (const NodeId& rInfo);    
		NodeId&                  operator=           (const struct sockaddr_in& rInfo);    

		static const NodeId&     LocalId             ();

	  //------------------------------------------------------------------------
	  // Protected Methods
	  //------------------------------------------------------------------------
	  protected:
		void                     Resolve              (const TCHAR strName[]);

	  private:
		struct sockaddr_in       m_structInfo;
	};

} } // namespace Solutions::Generics

#endif // __NODEID_H
