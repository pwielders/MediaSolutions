
#include "MediaStreamer.h"

namespace Solutions { namespace Generics
{
	
MediaSender::MediaSender(IMedia* media) :
	m_Media(media),
	m_SendBuffer(Generics::DataStorage::Create(2048)),
	m_SendOffset(0),
	m_Admin()
{
}

/* virtual */ MediaSender::~MediaSender()
{
}

IMedia* MediaSender::Media()
{
	return (m_Media);
}

// Description: This method will send any left information. After this call is made and there
//              is still some data left to send, the method returns false to indicate that not
//              all information was send.
//              Always call this method before inserting new data, just to make sure that the 
//              streamer is available to send data out.
bool MediaSender::Send ()
{
	m_Admin.Lock();

	if (m_SendBuffer.Size() != 0)
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

	m_Admin.Unlock();

	return (m_SendBuffer.Size() == 0);
}

// Description: This method starts a request to send supplied data. Only call this if there is no other 
//              send pending. This can be checked by calling the Send method without parameters, 
//              that method will return true if there is no more data left to send.
bool MediaSender::Send (const DataElement& buffer)
{
	bool accepted = false;

	m_Admin.Lock();

	if (m_SendBuffer.Size() == 0)
	{
		accepted = true;
		m_SendBuffer.Size(buffer.Size());
		m_SendBuffer.Copy(buffer);
		m_SendOffset = 0;

		Send();
	}

	m_Admin.Unlock();

	return (accepted);
}

// Description: This method starts a request to send supplied data. Only call this if there is no other 
//              send pending. This can be checked by calling the Send method without parameters, 
//              that method will return true if there is no more data left to send.
bool MediaSender::Send (const DataElementContainer& buffer)
{
	bool accepted = false;

	m_Admin.Lock();

	if (m_SendBuffer.Size() == 0)
	{
		accepted = true;
		m_SendBuffer.Size(buffer.Size());
		m_SendBuffer.Copy(0, buffer);
		m_SendOffset = 0;

		Send();
	}

	m_Admin.Unlock();

	return (accepted);
}

} } // namespace Solutions::Generics
