#include "Session.h"
#include "Channel.h"
#include "TraceCategories.h"

namespace Solutions { 
	
ENUM_CONVERSION_TABLE(Source::EnumStreamType) 
{
	{ Source::AUDIO, _TXT("audio") },
	{ Source::VIDEO, _TXT("video") },
	{ Source::TEXT,  _TXT("text")  }
};

namespace Sessions
{

/* static */  String Channel::Name (const uint8 index)
{
	return (String("Track") + Generics::NumberType<uint8,false>(index).Text());
}

Channel::Channel(Channel*& root, Source::ISource* source, Sink::ISink* sink) :
	m_Source(source),
	m_Sink(sink),
	m_NextChannel(NULL),
	m_Active(false)
{
	uint8 index = 0;

	// Make sure this new channel is added in the line..
	if (root == NULL)
	{
		root = this;
	}
	else
	{
		index++;

		// Walk the line till we find NULL and add
		while (root->m_NextChannel != NULL)
		{
			index++;
			root = root->m_NextChannel;
		}

		root->m_NextChannel = this;
	}

	m_Identifier = Generics::TextFragment(Channel::Name(index));
}

/* virtual */ Channel::~Channel()
{
	// This will cause the whole chain to be deleted..
	if (m_NextChannel != NULL)
	{
		delete m_NextChannel;
		m_NextChannel = NULL;
	}

	if (m_Sink != NULL)
	{
		delete m_Sink;
		m_Sink = NULL;
	}

	// We are also the top most level user of the IStream, delete it..
	delete m_Source;
	m_Source = NULL;
}

uint64 Channel::SinkFrames (const uint64 presentationTime)
{
	uint64 result = NUMBER_MAX_UNSIGNED(uint64);

	if (m_Active)
	{
		// When should this be presented ?
		result = m_Source->PresentationTime();

		// Does it fit the "timed" presentation time?
		if (result <= presentationTime)
		{
			// Create a frame to fill and sink....
			Generics::DataElementContainer newFrame(m_Sink->SinkSize());

			// Source the new frame
			if (m_Source->NextPackage(newFrame) != NUMBER_MAX_UNSIGNED(uint32))
			{
				// Sink the newly created frame..
				m_Sink->Sink (result,  newFrame);

				// get the next presentation time
				result = m_Source->PresentationTime();
			}
			else
			{
				result = NUMBER_MAX_UNSIGNED(uint64);

				// We are done streaming on this channel, nothing to drop anymore !!
				m_Active = false;
			}
		}
	}

	// See if we need to open more ?
	if (m_NextChannel != NULL)
	{
		uint64 value = m_NextChannel->SinkFrames(presentationTime);

		if (result < result)
		{
			result = value;
		}
	}

	return (result);
}

} } // namespace Solutions::Sessions

