#ifndef __MPEGSTREAM_H
#define __MPEGSTREAM_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGHeader.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace MPEG
{
	class EXTERNAL MPEGStream
	{
		private:
			class EXTERNAL MPEGElement
			{
				public:
					MPEGElement();
					MPEGElement (const MPEGElement& copy);
					MPEGElement (const Generics::DataElement& source);
					MPEGElement (const Generics::DataElement& source, const GroupOfPicturesHeader& groupOfPictureHeader, const PictureHeader& pictureHeader);
					inline ~MPEGElement () {}

					MPEGElement& operator=  (const MPEGElement& RHS);

				public:
					inline bool IsValid() const
					{
						return ((m_IndexSlice < m_Buffer.Size()) && (m_GroupOfPicturesHeader.IsValid()) && (m_PictureHeader.IsValid()));
					}
					inline const GroupOfPicturesHeader& CurrentGroupOfPicturesHeader () const
					{
						return (m_GroupOfPicturesHeader);
					}
					inline const PictureHeader& CurrentPictureHeader () const
					{
						return (m_PictureHeader);
					}
					inline bool PassedGroupOfPicturesHeader () const
					{
						return (m_NewGroupOfPicturesHeader);
					}
					inline bool PassedPictureHeader () const
					{
						return (m_NewPictureHeader);
					}
					bool NextGOP();
					bool NextPicture();
					bool NextSlice();

					SliceHeader CurrentSlice() const;

				private:
					Generics::DataElement	m_Buffer;
					uint64					m_IndexGOP;
					uint64					m_IndexPicture;
					uint64					m_IndexSlice;
					GroupOfPicturesHeader	m_GroupOfPicturesHeader;
					PictureHeader			m_PictureHeader;
					bool					m_NewGroupOfPicturesHeader;
					bool					m_NewPictureHeader;

			};

		public:
			MPEGStream ();
			MPEGStream (const MPEGStream& copy);
			MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint8 streamId = 0);
			MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint16 pid, const uint8 frameSize);
			inline ~MPEGStream() {}

			MPEGStream& operator= (const MPEGStream& RHS);

		public:
			inline bool IsValid() const
			{
				return (m_Index.IsValid());
			}
			inline const GroupOfPicturesHeader& CurrentGroupOfPicturesHeader () const
			{
				return (m_Index.CurrentGroupOfPicturesHeader());
			}
			inline const PictureHeader& CurrentPictureHeader () const
			{
				return (m_Index.CurrentPictureHeader());
			}
			inline SliceHeader CurrentSlice() const
			{
				return (m_Index.CurrentSlice());
			}
			inline bool PassedGroupOfPicturesHeader () const
			{
				return (m_Index.NewGroupOfPicturesHeader());
			}
			inline bool PassedPictureHeader () const
			{
				return (m_Index.NewPictureHeader());
			}

			bool NextGOP();
			bool NextPicture();
			bool NextSlice();

	private:
			bool GetNextStreamChunk(ElementaryStreamHeader& chunk);

	private:
		uint8					m_StreamId;
		MPEGElement				m_Index;
		Generics::DataElement	m_Buffer;
		uint64					m_Offset;
		uint16					m_Pid;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGSTREAM_H