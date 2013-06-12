#include "TraceUnit.h"
#include "TraceMedia.h"
#include "TraceControl.h"

namespace Solutions { namespace Trace
{

#pragma warning (disable: 4355)

TraceMedia::TraceMedia(const Generics::NodeId& nodeId) :
	m_Output (Generics::NodeId(_T("0.0.0.0"), nodeId.PortNumber()+1), nodeId, OUTPUTBUFFERSIZE),
	m_CommandHandler(*this, &m_Output)
{
	m_Output.Open(1);
	m_CommandHandler.Run();
}

#pragma warning (default: 4355)

TraceMedia::~TraceMedia()
{
	m_CommandHandler.Block();
	m_Output.Abort();
}

void TraceMedia::SendTraceLine (
	const char			a_FileName[],
	const unsigned int	a_LineNumber,
	const char			a_ClassName[],
	const ITrace*		a_Information)
{
	uint64 current = Generics::Time::Now().Ticks();
	static uint8 traceLine[OUTPUTBUFFERSIZE];
	uint32 size = (sizeof (traceLine) - 13);

	traceLine[0] = 'T';
	traceLine[1] = (current >> 56) & 0xFF;
	traceLine[2] = (current >> 48) & 0xFF;
	traceLine[3] = (current >> 40) & 0xFF;
	traceLine[4] = (current >> 32) & 0xFF;
	traceLine[5] = (current >> 24) & 0xFF;
	traceLine[6] = (current >> 16) & 0xFF;
	traceLine[7] = (current >>  8) & 0xFF;
	traceLine[8] = (current >>  0) & 0xFF;
	traceLine[9] = (a_LineNumber >> 24) & 0xFF;
	traceLine[10] = (a_LineNumber >> 16) & 0xFF;
	traceLine[11] = (a_LineNumber >>  8) & 0xFF;
	traceLine[12] = (a_LineNumber >>  0) & 0xFF;
	uint8* result = CopyText (&traceLine[13], a_FileName, size);
	result = CopyText (result, a_ClassName, size);
	result = CopyText (result, a_Information->GetTraceText(), size);

	size = sizeof (traceLine) - size;
	m_Output.Send(size, traceLine, 0);
}

void TraceMedia::HandleMessage()
{
	uint8		readBuffer[512];
	uint32		size = sizeof (readBuffer);

	if ( (m_Output.PendingTriggers() != 0) &&  (m_Output.Receive(size, readBuffer, 0) == OK) && ((sizeof (readBuffer) - size) >= 6) )
	{
		// See if this is a command
		if (readBuffer[0] == 'C')
		{
			switch (readBuffer[5])
			{
			case 0: // List all trace categories !!
				{
					TraceUnit::Iterator index = TraceUnit::Instance().GetCategories();

					// This will be a response, send out response messages...
					readBuffer[0] = 'R';

					// Sequence number is still in there, no need to replace ;-)

					// Send out all categories/modules
					while (index.Next() == true)
					{
						uint32 size = sizeof (readBuffer) - 8;

						readBuffer[6] = ((*index)->Enabled() == true ? '1' : '0');
						readBuffer[7] = '\0';

						// Sequence is still in the buffer, add line
						uint8* last=CopyText(&readBuffer[8], (*index)->GetCategoryName(), size);
						last = CopyText(last, (*index)->GetModuleName(), size);

						size = sizeof(readBuffer) - size;

						// Give 1s per line to send ;-)
						m_Output.Send(size, readBuffer, 1000);
					}
					break;
				}
			case 1: // Toggle tracing
				{
					char* module = reinterpret_cast<char*>(::memchr (&readBuffer[8], '\0', sizeof(readBuffer)-8));
					if (module != NULL)
					{
						uint32 count = TraceUnit::Instance().SetCategories ((readBuffer[6] == '1'), &module[1], reinterpret_cast<char*>(&readBuffer[8]));

						readBuffer[0] = 'R';

						// affter the command we get the the change count
						readBuffer[6] = (count >> 24) & 0xFF;
						readBuffer[7] = (count >> 16) & 0xFF;
						readBuffer[8] = (count >>  8) & 0xFF;
						readBuffer[9] = (count >>  0) & 0xFF;

						uint32 size = 10;

						m_Output.Send (size, readBuffer, 1000);
					}
					break;
				}
			}
		}
	}
}

uint8* TraceMedia::CopyText (uint8* destination, const char* source, uint32& maxSize)
{
	if (maxSize > 0)
	{
		uint32 size = maxSize - 1;
		const char* end = reinterpret_cast<const char*> (::memchr (source, '\0', maxSize));

		if (end != NULL)
		{
			size = end - source;
		}

		::memcpy (destination, source, size);
		destination = &destination[size];
		*destination++ = '\0';
		maxSize -= (size + 1);
	}

	return (destination);
}

} } // namespace Solutions::Trace

