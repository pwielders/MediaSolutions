#ifndef __MPEGSOURCE_H
#define __MPEGSOURCE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGHeader.h"
#include "MPEGStream.h"
#include "MPEGVideo.h"
#include "MPEGAudio.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace MPEG
{
	class EXTERNAL MPEGSource
	{	
		private:	
			typedef struct
			{
				uint64				offset;
				uint8				id;
				enum StreamType		type;

			} StreamInfo;

		public:
			class Iterator
			{
				public:
					Iterator() :
						m_Data(),
						m_Iterator()
					{
					}
					Iterator(const Generics::DataElement& bytestream, std::list<struct StreamInfo>& streams) :
						m_Data(bytestream),
						m_Iterator (streams)
					{
					}
					Iterator(const Iterator& copy) :
						m_Data(copy.m_Data),
						m_Iterator (copy.m_Iterator)
					{
					}	
					~Iterator()
					{
					}
					
					Iterator& operator= (const Iterator& RHS)
					{
						m_Data = RHS.m_Data;
						m_Iterator = RHS.m_Iterator;
						return (*this);
					}

				public:
					inline bool IsValid () const
					{
						return (m_Iterator.IsValid());
					}
					inline uint32 Channels () const
					{
						return (m_Iterator.Count());
					}
					inline void Reset ()
					{
						m_Iterator.Reset(true);
					}
					inline bool Next ()
					{
						return (m_Iterator.Next());
					}
					inline StreamType Type () const
					{
						return ((*m_Iterator).type);
					}
					inline MPEGVideo Video() const
					{
						return (MPEGVideo(MPEGStream(m_Data, (*m_Iterator).offset, (*m_Iterator).id)));
					}					
					inline MPEGAudio Audio() const
					{
						return (MPEGAudio(MPEGStream(m_Data, (*m_Iterator).offset, (*m_Iterator).id)));
					}					

				private:
					Generics::DataElement														m_Data;
					Generics::IteratorType<std::list<struct StreamInfo>, struct StreamInfo>		m_Iterator;
			};

		private:
			MPEGSource(const MPEGSource&);
			MPEGSource operator= (const MPEGSource&);

		public:
			MPEGSource (const Generics::DataElement& dataObject);
			~MPEGSource();

		public:
			inline bool IsValid() const
			{
				return (m_Data.IsValid());
			}
			inline Iterator Streams ()
			{
				return (Iterator(m_Data, m_ChannelsOffset));
			}

		private:
			Generics::DataElement	m_Data;
			std::list<StreamInfo>	m_ChannelsOffset;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGSOURCE_H