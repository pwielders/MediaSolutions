#ifndef __MPEGTRANSPORT_H
#define __MPEGTRANSPORT_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGHeader.h"
#include "MPEGStream.h"
#include "MPEGTable.h"
#include "MPEGVideo.h"
#include "MPEGAudio.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace MPEG
{
	class EXTERNAL MPEGTransport
	{	
		public:
			class Iterator
			{
				public:
					Iterator() :
						m_Data(),
						m_MainIterator(),
						m_Iterator(),
						m_FrameSize(0),
						m_Channels(0)
					{
					}
					Iterator(const Generics::DataElement& data, Table& table, const uint8 frameSize) :
						m_Data(data),
						m_MainIterator(table.Elements()),
						m_Iterator(),
						m_FrameSize(frameSize),
						m_Channels(0)
					{
						while (m_MainIterator.Next())
						{
							PMT::Iterator internalLoop(m_MainIterator.Element<PMT>().Streams());

							while (internalLoop.Next())
							{
								m_Channels++;
							}
						}

						m_MainIterator.Reset();
					}
					Iterator(const Iterator& copy) :
						m_Data(copy.m_Data),
						m_MainIterator(copy.m_MainIterator),
						m_Iterator(copy.m_Iterator),
						m_FrameSize(copy.m_FrameSize),
						m_Channels(copy.m_Channels)
					{
					}	
					~Iterator()
					{
					}
					
					Iterator& operator= (const Iterator& RHS)
					{
						m_Data = RHS.m_Data;
						m_MainIterator = RHS.m_MainIterator;
						m_FrameSize = RHS.m_FrameSize;
						m_Iterator = RHS.m_Iterator;
						m_Channels = RHS.m_Channels;

						return (*this);
					}

				public:
					inline bool IsValid () const
					{
						return (m_Iterator.IsValid());
					}

					inline uint32 Channels () const
					{
						return (m_Channels);
					}

					inline void Reset ()
					{
						m_MainIterator.Reset();
					}

					inline bool Next ()
					{
						while ( (m_Iterator.Next() == false) && (m_MainIterator.Next() == true) )
						{
							m_Iterator = m_MainIterator.Element<PMT>().Streams();
						}
						return (m_Iterator.IsValid());
					}

					inline StreamType Type () const
					{
						uint8 type = m_Iterator.StreamType();

						return (static_cast<StreamType>(type));
					}

					inline MPEGVideo Video() const
					{
						return (MPEGVideo(MPEGStream(m_Data, 0, m_Iterator.Pid(), m_FrameSize)));
					}					

					inline MPEGAudio Audio() const
					{
						return (MPEGAudio(MPEGStream(m_Data, 0, m_Iterator.Pid(), m_FrameSize)));
					}					

				private:
					Generics::DataElement		m_Data;
					Table::Iterator				m_MainIterator;
					PMT::Iterator				m_Iterator;
					uint8						m_FrameSize;
					uint32						m_Channels;
			};

		private:
			MPEGTransport(const MPEGTransport&);
			MPEGTransport operator= (const MPEGTransport&);

		public:
			MPEGTransport (const Generics::DataElement& dataObject, const uint64 offset);
			~MPEGTransport();

		public:
			inline bool IsValid() const
			{
				return ( (m_Data.IsValid()) && (m_PMTTable.IsValid()) );
			}
			inline Iterator Streams ()
			{
				return (Iterator(m_Data, m_PMTTable, m_FrameSize));
			}

		private:
			uint8 DeterminePackageSize (Generics::DataElement& copy);

		private:
			Generics::DataElement			m_Data;
			Generics::ScopedStorage<256>	m_Space;	// 256 bytes for a single program PMT should be sufficient.
			Table							m_PMTTable;
			uint8							m_FrameSize;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGTRANSPORT_H