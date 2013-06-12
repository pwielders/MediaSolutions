#ifndef __DATAELEMENTSTREAMERTYPE_H
#define __DATAELEMENTSTREAMERTYPE_H

#include "IMedia.h"
#include "DataElement.h"

namespace Solutions { namespace Generics
{
	template<typename PREAMBLEHANDLER>
	class DataElementStreamSendType
	{	
		private:
			DataElementStreamSendType(const DataElementStreamSendType<PREAMBLEHANDLER>&);
			DataElementStreamSendType<PREAMBLEHANDLER>& operator= (const DataElementStreamSendType<PREAMBLEHANDLER>&);

		public:
			DataElementStreamSendType(PREAMBLEHANDLER& handler, IMedia* media) :
				m_Handler(handler),
				m_Media(media),
				m_SendBuffer(),
				m_SendOffset(0),
				m_Admin()

			{
			}
			virtual ~DataElementStreamSendType()
			{
			}

		public:
			IMedia* Media()
			{
				return (m_Media);
			}

			// Description: This method will send any left information. After this call is made and there
			//              is still some data left to send, the method returns false to indicate that not
			//              all information was send.
			//              Always call this method before inserting new data, just to make sure that the 
			//              streamer is available to send data out.
			bool Send ()
			{
				m_Admin.Lock();

				if (m_SendBuffer.Size() != 0)
				{
					// Do we need to load the preamble first ?
					if (m_SendPreamble.Size() != 0)
					{
						// Sends as much as we can, untill we send it all....
						uint32 requestedSize = (static_cast<uint32> (m_SendPreamble.Size()) - m_SendOffset);

						m_Media->Send (requestedSize, &(m_SendPreamble.Buffer()[m_SendOffset]), 0);

						m_SendOffset += (static_cast<uint32>(m_SendPreamble.Size()) - requestedSize);

						if (m_SendOffset == m_SendPreamble.Size())
						{
							// Clear the Send Preamble...
							m_SendPreamble.Size(0);
							m_SendOffset   = 0;
						}
					}

					if (m_SendPreamble.Size() == 0)
					{
						// Read as much as we can, untill we filled  the buffer
						uint32 requestedSize = (static_cast<uint32> (m_SendBuffer.Size()) - m_SendOffset);

						m_Media->Send (requestedSize, &(m_SendBuffer.Buffer()[m_SendOffset]), 0);

						m_SendOffset += (static_cast<uint32>(m_SendBuffer.Size()) - requestedSize);

						if (m_SendOffset == m_SendBuffer.Size())
						{
							// Clear the Send buffer...
							m_SendBuffer.Size(0);
							m_SendOffset   = 0;
						}
					}
				}

				m_Admin.Unlock();

				return (m_SendBuffer.Size() == 0);
			}

			// Description: This method starts a request to send supplied data. Only call this if there is no other 
			//              send pending. This can be checked by calling the Send method without parameters, 
			//              that method will return true if there is no more data left to send.
			bool Send (const DataElement& buffer)
			{
				m_Admin.Lock();

				m_SendBuffer = buffer;
				m_SendPreamble = preamble;
				m_SendOffset = 0;

				// Determine the preamble..
				m_Handler.Preamble(m_SendPreamble, m_SendBuffer);

				m_Admin.Unlock();

				return (Send());
			}


		private:
			PREAMBLEHANDLER&	m_Handler;
			IMedia*				m_Media;

			DataElement			m_SendBuffer;
			DataElement			m_SendPreamble;
			uint32				m_SendOffset;

			CriticalSection		m_Admin;
	};

	template<typename PREAMBLEHANDLER>
	class DataElementStreamReceiveType
	{	
		private:
			DataElementStreamReceiveType(const DataElementStreamReceiveType<PREAMBLEHANDLER>&);
			DataElementStreamReceiveType<PREAMBLEHANDLER>& operator= (const DataElementStreamReceiveType<PREAMBLEHANDLER>&);

		public:
			DataElementStreamReceiveType(PREAMBLEHANDLER& handler, IMedia* media) :
				m_Handler(handler),
				m_Media(media),
				m_ReceiveBuffer(),
				m_ReceiveOffset(0),
				m_Admin()
			{
			}
			virtual ~DataElementStreamReceiveType()
			{
			}

		public:
			IMedia* Media()
			{
				return (m_Media);
			}

			// Description: This method will recieve any left information. After this call is made and there
			//              is still some data left for reading, the method returns false to indicate that not
			//              all information was received.
			//              Always call this method before requesting new data, just to make sure that the 
			//              streamer is available to receive new data.
			bool Receive ()
			{
				m_Admin.Lock();

				if (m_ReceiveBuffer.Size() != 0)
				{
					// Do we need to load the preamble first ?
					if (m_ReceivePreamble.Size() != 0)
					{
						// Sends as much as we can, untill we send it all....
						uint32 requestedSize = (static_cast<uint32> (m_ReceivePreamble.Size()) - m_ReceiveOffset);

						m_Media->Receive (requestedSize, &(m_ReceiveBuffer.Buffer()[m_ReceiveOffset]), 0);

						m_ReceiveOffset += (static_cast<uint32>(m_ReceivePreamble.Size()) - requestedSize);

						if (m_ReceiveOffset == m_ReceivePreamble.Size())
						{
							// Check out if the preamble is ready..
							m_Handler.ReceivePreamble(m_ReceivePreamble, m_ReceiveBuffer);

							// Clear the Send Preamble...
							m_ReceivePreamble = Generics::DataElement();
							m_ReceiveOffset   = 0;
						}
					}

					if (m_ReceivePreamble.Size() == 0)
					{
						// Read as much as we can, untill we filled  the buffer
						uint32 requestedSize = (static_cast<uint32> (m_ReceiveBuffer.Size()) - m_ReceiveOffset);

						m_Media->Receive (requestedSize, &(m_ReceiveBuffer.Buffer()[m_ReceiveOffset]), 0);

						m_ReceiveOffset += (static_cast<uint32>(m_ReceiveBuffer.Size()) - requestedSize);

						if (m_ReceiveOffset == m_ReceiveBuffer.Size())
						{
							// Clear the Send buffer...
							m_ReceiveBuffer = Generics::DataElement();
							m_ReceiveOffset   = 0;
						}
					}
				}

				m_Admin.Unlock();

				return (m_ReceiveBuffer.Size() == 0);
			}

			// Description: This method starts a request to receive new data. Only call this if there is no other 
			//              receive pending. This can be checked by calling the Receive method without parameters, 
			//              that method will return true if the last buffer set is filled.
			bool Receive (const DataElement& buffer)
			{
				m_Admin.Lock();

				m_ReceiveBuffer = buffer;
				m_ReceivePreamble = preamble;
				m_ReceiveOffset = 0;

				m_Admin.Unlock();

				return (Receive());
			}

		private:
			PREAMBLEHANDLER&	m_Handler;
			IMedia*				m_Media;

			DataElement			m_ReceiveBuffer;
			DataElement			m_ReceivePreamble;
			uint32				m_ReceiveOffset;

			CriticalSection		m_Admin;
	};

} } // namespace Solutions::Generics

#endif // __DATAELEMENTSTREAMERTYPE_H
