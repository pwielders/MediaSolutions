// SocketServer.h: interface for the SocketServerType class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SOCKETSERVER_H
#define __SOCKETSERVER_H

#include "Module.h"
#include "Proxy.h"

namespace Solutions { namespace Generics
{
	template <typename HANDLER>
	class SocketServerType
	{
		private:
			template <typename HANDLER>
			class SynchronisationObject : public SynchronisationContextType<SocketPort*>
			{
				public:
					SynchronisationObject(SocketServerType<HANDLER>& parent, const ProxyType<HANDLER>& handler, SocketStream* client) : 
						SynchronisationContextType<SocketPort*> (client), m_Handler(handler), m_Parent(parent)
					{
					}

					SynchronisationObject(SocketServerType<HANDLER>& parent, SocketListner* server) : 
						SynchronisationContextType<SocketPort*> (server), m_Handler(NULL), m_Parent(parent)
					{
					}
				
					SynchronisationObject(const SynchronisationObject<HANDLER>& copy) : 
						SynchronisationContextType<SocketPort*> (copy), m_Handler (copy.m_Handler), m_Parent(copy.m_Parent)
					{
					}

					~SynchronisationObject()
					{
					}

					SynchronisationObject<HANDLER>& operator=(const SynchronisationObject<HANDLER>& RHS)
					{
						SynchronisationContextType<SocketPort*>::operator= (RHS);
						m_Handler = RHS.m_Handler;

						return (*this);
					}

				public:
					inline void Process()
					{
						BYTE pending = Context()->PendingTriggers();

						if (m_Handler == NULL)
						{
							if ((pending & SocketPort::ACCEPT) != 0)
							{
								SocketStream* newSocket = new SocketStream();

								// Get the socket and add it to the dispatcher
								if (static_cast<SocketListner*>(Context())->Accept (*newSocket, 0) == OK)
								{
									// Seems like we have an accept, accept.
									SynchronisationObject<HANDLER> newClient (m_Parent, ProxyType<HANDLER>::Create(&m_Parent, newSocket), newSocket);

									// The handler is the consumer ;-)
									m_Parent.Processor().AddSynchronization(newClient);
								}
							}
						}
						else if ((pending & SocketPort::CLOSE) != 0)
						{
							// The otherside requested a close, we do a cloe..
							Context()->Close(INFINITE);

							// First remove the synchronisation, so we will never be 
							// triggered again !!
							m_Parent.Processor().RemoveSynchronization (*this);

							// Time to get ride of this handler..
							m_Handler = ProxyType<HANDLER>();
						}
						else
						{
							m_Handler->HandleProtocol();
						}
					}

				private:
					SocketServerType<HANDLER>&	m_Parent;
					ProxyType<HANDLER>			m_Handler;
			};

		public:
			SocketServerType (const NodeId& listeningAddress) :
				m_ServerPort (listeningAddress),
				m_Running    (false),
				m_Processor  (_T("SocketServer"))
			{
			}
			~SocketServerType ()
			{
			}

			void Start ()
			{
				if (m_Running == false)
				{
					m_ServerPort.Open (1);

					SynchronisationObject<HANDLER> handler (*this, &m_ServerPort);

					m_Processor.AddSynchronization(handler);
					m_Running = true;
				}
			}

			void Stop ()
			{
				if (m_Running == true)
				{
					m_ServerPort.Close (INFINITE);
					m_Processor.RemoveSynchronization(SynchronisationObject<HANDLER>(*this, &m_ServerPort));

					//// Now we definitely know that there are only client ports...
					SynchronizationDispatcherType<SynchronisationObject<HANDLER>>::ContextIterator iterator = m_Processor.GetSynchronisationIterator();

					while (iterator.Next() == true)
					{
						(*iterator).Context()->Close(INFINITE);
					}

					m_Processor.Clear ();

					// All open connections triggere

					m_Running = false;
				}
			}

			SynchronizationDispatcherType<SynchronisationObject<HANDLER>>& Processor ()
			{
				return (m_Processor);
			}

			const NodeId& ListeningNode () const
			{
				return (m_ServerPort.LocalNode());
			}

		private:
			bool															m_Running;
			SocketListner													m_ServerPort;
			SynchronizationDispatcherType<SynchronisationObject<HANDLER>>	m_Processor;
	};

} } // namespace Solutions::Generics

#endif // __SOCKETSERVER_H