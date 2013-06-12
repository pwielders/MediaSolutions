#pragma once

class EchoProtocol
{
	private:
		EchoProtocol(const EchoProtocol&);
		EchoProtocol& operator= (const EchoProtocol&);

	public:
		EchoProtocol(SocketServerType<EchoProtocol>* /* parent */, SocketStream* media) : 
			m_Media(media), 
			m_Receiver(), 
			m_Channel(media)
		{
			m_Channel.Receive (m_Receiver);
		}
		~EchoProtocol()
		{
			// This is the producer/consumer pattern!!!
			// We received the IMedia interface for consumption.
			// We need to delete it if we are done with it !!!
			delete m_Media;
		}

	public:
		void HandleProtocol ()
		{
			// If there is something left to send, send it !
			m_Channel.Send();

			// See if we have a new "request"
			if (m_Channel.Receive() == true)
			{
				// Try to send it out again !!
				if (m_Channel.Send() == true)
				{
					// There is space to send it, send it..
					m_Channel.Send(m_Receiver + _T(" (ECHO)\r\n"));

					// Oke we can receive a new one, clear the current one..
					m_Receiver.clear();

					m_Channel.Receive(m_Receiver);
				}
			}
		}

	private:
		IMedia*								m_Media;
		String								m_Receiver;
		TextStreamerType<char, '\n', '\r'>	m_Channel;
};