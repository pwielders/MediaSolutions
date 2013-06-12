#ifndef __TEXTSTREAMERTYPE_H
#define __TEXTSTREAMERTYPE_H

#include "IMedia.h"

namespace Solutions { namespace Generics
{
	template<typename BUFFERCHAR, const BUFFERCHAR DELIMITER, const BUFFERCHAR DROPCHARACTER = ~0>
	class TextStreamerType 
	{	
		public:
			typedef enum Status
			{
				RECEIVED_TEXT,
				SEND_TEXT,
				MEDIA_FAILURE
			};

		private:
			TextStreamerType(const TextStreamerType<BUFFERCHAR, DELIMITER,DROPCHARACTER>&);
			TextStreamerType<BUFFERCHAR, DELIMITER,DROPCHARACTER>& operator= (const TextStreamerType<BUFFERCHAR, DELIMITER, DROPCHARACTER>&);

		public:
			TextStreamerType(IMedia* media, const uint32 bufferSize = 0) :
				m_Media(media),
				m_ReceiveBuffer(NULL),
				m_ReceiveMultiByteOffset(0),
				m_SendBuffer(),
				m_SendOffset(NUMBER_MAX_UNSIGNED(uint32)),
				m_FilledSize(0),
				m_StreamBufferOffset(0),
				m_StreamBufferSize(0),
				m_SendDynamicBuffer(NULL),
				m_StreamBuffer(NULL),
				m_Admin()
			{
				ASSERT (m_Media != NULL);

				// Buffer the output, or flush it right out!!!
				if (bufferSize != 0)
				{
					// Seems like we want to buffer, before we send..
					m_SendDynamicBuffer = new BUFFERCHAR[bufferSize];
					m_StreamBuffer = m_SendDynamicBuffer;
					m_StreamBufferSize = bufferSize * sizeof(BUFFERCHAR);
				}
				else
				{
					m_StreamBuffer = &m_SendMultiByte[0];
					m_StreamBufferSize = sizeof (m_SendMultiByte);
				}
			}
			~TextStreamerType()
			{
				if (m_SendDynamicBuffer != NULL)
				{
					delete [] m_SendDynamicBuffer;
				}
			}

		public:
			// Description: This method will reacive any left information. After this call is made and there
			//              is still some data left for reading, the method returns false to indicate that not
			//              all information was received.
			//              Always call this method before requesting new data, just to make sure that the 
			//              streamer is available to receive new data.
			bool Receive ()
			{
				m_Admin.Lock();

				int status = (m_ReceiveBuffer == NULL ? ERROR_UNAVAILABLE : OK);

				// The system is in a blocking mode if m_MultiOffset == ~0 !!!
				while ((m_ReceiveMultiByteOffset != ~0) && (status == OK))
				{
					// Make sure there is enough space left to load a character.
					if ((m_ReceiveBuffer->length() + (sizeof(m_ReceiveMultiByte)/sizeof(BUFFERCHAR))) > m_ReceiveBuffer->max_size())
					{
						// Oops we can not store an additional character
						m_ReceiveBuffer->resize(m_ReceiveBuffer->max_size() * 2);
					}

					NodeId remoteEnd;
					uint32 leftBytes = sizeof(BUFFERCHAR);

					// Read a character.
					status = m_Media->Receive (leftBytes, &(reinterpret_cast<uint8*>(&m_ReceiveMultiByte[0])[m_ReceiveMultiByteOffset]), 0);
					
					m_ReceiveMultiByteOffset += (sizeof(BUFFERCHAR) - leftBytes);

					// If we are on a character boundary, process it..
					if ((leftBytes != sizeof(BUFFERCHAR)) && ((sizeof(BUFFERCHAR) == 1) || ((m_ReceiveMultiByteOffset % sizeof(BUFFERCHAR)) == 0)))
					{
						BUFFERCHAR loadedChar = m_ReceiveMultiByte[(m_ReceiveMultiByteOffset / sizeof(BUFFERCHAR)) - 1];

						// Seems like we have a full character, See if it is not a DROP character.
						if ((DROPCHARACTER != ~0) && (loadedChar == DROPCHARACTER))
						{
							// Drop this character, move the character buffer back !!
							m_ReceiveMultiByteOffset -= sizeof(BUFFERCHAR);
						}
						else if ((m_ReceiveMultiByteOffset == sizeof(BUFFERCHAR)) && (loadedChar == DELIMITER))
						{
							// We loaded a DELIMITER on the beginning of a character array...
							m_ReceiveMultiByteOffset = static_cast<uint32> (~0);
						}
						else
						{
							TCHAR convertedChar[sizeof(m_ReceiveMultiByte)];

							int convertedSize = ToCharacter(&m_ReceiveMultiByte[0], &convertedChar[0], (m_ReceiveMultiByteOffset / sizeof (BUFFERCHAR)));

							if (convertedSize != -1)
							{
								int index = 0;

								// Seems like we could convert it..
								while (convertedSize != index)
								{
									// It is converetd, Add the info.
									m_ReceiveBuffer->push_back(convertedChar[index]);

									index++;
								}
								m_ReceiveMultiByteOffset = 0;
							}
						}
					}
				}

				m_Admin.Unlock ();

				return (m_ReceiveMultiByteOffset == ~0);
			}

			// Description: This method starts a request to receive new data. Only call this if there is no other 
			//              receive pending. This can be checked by calling the Receive method without parameters, 
			//              that method will return true if the last buffer set is filled.
			inline bool Receive (String& newBuffer)
			{
				m_Admin.Lock ();

				m_ReceiveBuffer = &newBuffer;
				m_BufferMaintainer = newBuffer;
				m_ReceiveMultiByteOffset = 0;

				m_Admin.Unlock ();

				// There might be some lines left ;-)
				return (Receive());
			}

			// Description: This method will send any left information. After this call is made and there
			//              is still some data left to send, the method returns false to indicate that not
			//              all information was send.
			//              Always call this method before inserting new data, just to make sure that the 
			//              streamer is available to send data out.
			bool Send ()
			{
				m_Admin.Lock ();

				int status = (m_SendOffset == ~0 ? ERROR_UNAVAILABLE : OK);

				// If there is still space in the buffer (status) and we have bytes to send, do so...
				if ((status == OK) && (m_FilledSize == 0))
				{
					// Convert the characters into the buffer
					while ( (m_SendOffset != ~0) && (m_FilledSize <= (m_StreamBufferSize - sizeof(m_SendMultiByte))) )
					{
						// See if we are on the end character
						if (m_SendBuffer.length() == m_SendOffset)
						{
							m_SendOffset = static_cast<uint32> (~0);

							m_StreamBuffer[m_FilledSize/sizeof(BUFFERCHAR)] = DELIMITER;
							m_FilledSize   += sizeof(BUFFERCHAR);

							if (DROPCHARACTER != ~0)
							{
								m_StreamBuffer[m_FilledSize/sizeof(BUFFERCHAR)] = DROPCHARACTER;
								m_FilledSize   += sizeof(BUFFERCHAR);
							}
						}
						else
						{
							int count = 1;
							int additional;
							const BUFFERCHAR* character = &(m_SendBuffer.c_str()[m_SendOffset]);

							// Convert this character to the character set as we send it.
							while ( ((additional = ToCharacter(character, &m_StreamBuffer[m_FilledSize/sizeof(BUFFERCHAR)], count)) == -1) &&
									(count < sizeof (m_SendMultiByte)) )
							{
								count++;
							}

							// See if we could convert the character
							if (additional != -1)
							{
								m_FilledSize += (additional * sizeof (BUFFERCHAR));
								m_SendOffset += count;
							}
							else
							{
								// Do not know what this character is, skip it..
								m_SendOffset++;
							}
						}
					}
				}

				// Publish the buffer we have !!!
				if (m_FilledSize != 0)
				{
					uint32 size = (m_FilledSize - m_StreamBufferOffset);
					status = m_Media->Send (size, reinterpret_cast<uint8*>(&m_StreamBuffer[m_StreamBufferOffset]), 0);
					if (size == 0)
					{
						// The message has been send, start the next one..
						m_FilledSize = 0;
						m_StreamBufferOffset = 0;
					}
					else
					{
						m_StreamBufferOffset += ((m_FilledSize - m_StreamBufferOffset) - size);
					}
				}

				m_Admin.Unlock ();

				return (m_SendOffset == ~0);
			}

			// Description: This method starts a request to send supplied data. Only call this if there is no other 
			//              send pending. This can be checked by calling the Send method without parameters, 
			//              that method will return true if there is no more data left to send.
			inline bool Send (const String& newBuffer)
			{
				m_Admin.Lock ();

				m_SendBuffer = newBuffer;
				m_SendOffset = 0;

				m_Admin.Unlock();

				// There might be some lines left ;-)
				return (Send());
			}

		private:
			IMedia*			m_Media;

			String*			m_ReceiveBuffer;
			String			m_BufferMaintainer;
			uint32			m_ReceiveMultiByteOffset;
			BUFFERCHAR		m_ReceiveMultiByte[4];

			// Keep a reference to the string we want to send..
			String			m_SendBuffer;
			uint32			m_SendOffset;
			uint32			m_FilledSize;
			uint32			m_StreamBufferOffset;
			uint32			m_StreamBufferSize;
			BUFFERCHAR		m_SendMultiByte[4];
			BUFFERCHAR*		m_SendDynamicBuffer;
			BUFFERCHAR*		m_StreamBuffer;

			CriticalSection	m_Admin;
	};

} } // namespace Solutions::Generics

#endif // __TEXTSTREAMERTYPE_H
