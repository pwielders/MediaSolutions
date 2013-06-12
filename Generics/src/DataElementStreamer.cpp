#include "DataElementStreamer.h"

namespace Solutions { namespace Generics
{

DataElementStreamer::DataElementStreamer(IMedia* media) :
    m_Media(media),
	m_ReceiveIterator(),
	m_ReceiveOffset(0),
	m_SendIterator(),
	m_SendOffset(0)
{
	ASSERT (m_Media != NULL);
}

/* virtual */ DataElementStreamer::~DataElementStreamer()
{
}

bool DataElementStreamer::Receive ()
{
	int status = (m_ReceiveIterator.IsValid() ? OK : ERROR_UNAVAILABLE);

	// If there is still spae in the buffer (status) and we have bytes to send, do so...
	while ((m_ReceiveOffset != ~0) && (status == OK))
	{
		uint32 BytesToReceive = static_cast<uint32>(m_ReceiveIterator.Data().Size() - m_ReceiveOffset);

		status = m_Media->Receive(BytesToReceive, &(m_ReceiveIterator.Data().Buffer()[m_ReceiveOffset]), 0);

		if (BytesToReceive == 0)
		{
			MoveReceiveIterator ();
		}
		else
		{
			// Oops not all fitted, remember, the lost part..
			m_ReceiveOffset =  static_cast<uint32>(m_ReceiveIterator.Data().Size() - BytesToReceive);
		}
	} 

	return (m_SendOffset == ~0);
}

bool DataElementStreamer::Receive (DataElementIterator& newBuffer)
{
	m_ReceiveIterator = newBuffer;

	MoveReceiveIterator();

	// There might be some lines left ;-)
	return (Receive());
}

bool DataElementStreamer::Send ()
{
	int status = (m_SendIterator.IsValid() ? OK : ERROR_UNAVAILABLE);

	// If there is still space in the buffer (status) and we have bytes to send, do so...
	while ((m_SendOffset != ~0) && (status == OK))
	{
		uint32 BytesToSend = static_cast<uint32>(m_SendIterator.Data().Size() - m_SendOffset);

		status = m_Media->Send(BytesToSend, &(m_SendIterator.Data().Buffer()[m_SendOffset]), 0);

		if (BytesToSend == 0)
		{
			MoveSendIterator ();
		}
		else
		{
			// Oops not all fitted, remember, the lost part..
			m_SendOffset =  static_cast<uint32>(m_SendIterator.Data().Size() - BytesToSend);
		}
	} 

	return (m_SendOffset == ~0);
}

bool DataElementStreamer::Send (const DataElementIterator& newBuffer)
{
	m_SendIterator = newBuffer;

	MoveSendIterator ();

	// There might be some lines left ;-)
	return (Send());
}

void DataElementStreamer::MoveSendIterator ()
{
	// Seems like all bytes ended up in the buffer, move on to the next...
	if (m_SendIterator.Next() == true)
	{
		m_SendOffset = 0;
	}
	else
	{
		m_SendOffset = NUMBER_MAX_UNSIGNED(uint32);
	}
}

void DataElementStreamer::MoveReceiveIterator ()
{
	// Seems like all bytes ended up in the buffer, move on to the next...
	if (m_ReceiveIterator.Next() == true)
	{
		m_ReceiveOffset = 0;
	}
	else
	{
		m_ReceiveOffset = NUMBER_MAX_UNSIGNED(uint32);
	}
}

} } // namespace Solutions::Generics
