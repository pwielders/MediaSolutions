#ifndef __MPEGHEADER_H
#define __MPEGHEADER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace MPEG 
{
	const uint8 PICTURE_MARKER					=	0x00;
	const uint8 SEQUENCE_MARKER					=	0xB3;
	const uint8 SEQUENCE_EXTENDED_MARKER		=	0xB5;
	const uint8 GROUP_OF_PICTURES_MARKER		=	0xB8;
	const uint8 USER_DATA_MARKER				=	0xB2;
	const uint8 PADDING_STREAM_MARKER			=	0xBE;
	const uint8 SYSTEM_STREAM_MARKER			=	0xBB;
	const uint8 PACK_STREAM_MARKER				=	0xBA;

	typedef enum StreamType
	{
		AUDIO,
		VIDEO,
		UNKNOWN
	};	

	class EXTERNAL ReferenceClock
	{
		public:
			inline ReferenceClock() :
				m_ReferenceClock(NUMBER_MAX_UNSIGNED(uint64)),
				m_Extension(NUMBER_MAX_UNSIGNED(uint16))
			{
			}
			inline ReferenceClock(const uint64 clock, const uint16 extension) :
				m_ReferenceClock(clock),
				m_Extension(extension)
			{
			}
			inline ReferenceClock(const ReferenceClock& copy) :
				m_ReferenceClock(copy.m_ReferenceClock),
				m_Extension(copy.m_Extension)
			{
			}
			inline ~ReferenceClock()
			{
			}

			inline ReferenceClock& operator= (const ReferenceClock& RHS)
			{
				m_ReferenceClock = RHS.m_ReferenceClock;
				m_Extension = RHS.m_Extension;

				return (*this);
			}

		public:
			inline uint64 Clock () const
			{
				return (m_ReferenceClock);
			}
			inline uint16 Extension () const
			{
				return (m_Extension);
			}

		private:
			uint64	m_ReferenceClock;
			uint16	m_Extension;

	};

	class EXTERNAL AudioHeader
	{
		public:
			static uint64 FindTag (const Generics::DataElement& data, const uint64 offset);

			AudioHeader(const Generics::DataElement& value, const uint64 offset);

			AudioHeader() :
				m_Value()
			{
			}
			AudioHeader(const AudioHeader& copy) : 
				m_Value(copy.m_Value)
			{
			}

			~AudioHeader ()
			{
			}

			AudioHeader& operator= (const AudioHeader& RHS)
			{
				m_Value = RHS.m_Value;

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return ( (m_Value.Size() >= 4) && (m_Value[0] == 0xFF) && ((m_Value[1] & 0xE0) == 0xE0) && ((m_Value[1] & 0x06) != 0x00) && ((m_Value[2] & 0x0C) != 0x0C) );
			}

			inline bool IsMPEG2 () const
			{
				return (IsMPEG25() || ((m_Value[1] & 0x18) == 0x10));
			}

			inline bool IsMPEG25 () const
			{
				return ((m_Value[1] & 0x18) == 0x00);
			}

			inline bool IsFreeFormat () const
			{
				return ((m_Value[2] & 0xF0) == 0x00);
			}

			inline bool IsStereo () const
			{
				return ((m_Value[3] & 0xC0) != 0xC0);
			}

			inline bool IsCopyrighted () const
			{
				return ((m_Value[3] & 0x08) == 0x08);
			}

			inline bool IsOriginal () const
			{
				return ((m_Value[3] & 0x04) == 0x04);
			}

			inline bool HasCRC () const
			{	
				return ((m_Value[1] & 0x1) == 0x00);
			}

			inline bool HasPadding () const
			{
				return ((m_Value[2] & 0x02) == 0x02);
			}

			inline bool HasExtension () const
			{
				return ((m_Value[2] & 0x1) == 0x01);
			}

			inline uint8 Layer () const
			{
				return (4 - ((m_Value[1] & 0x06) >> 1));
			}

 			inline uint8 Mode () const
			{
				return ((m_Value[3] & 0xC0) >> 6);  
			}

 			inline uint8 ModeExtended () const
			{
				return ((m_Value[3] & 0x60) >> 4);
			}

			inline uint8 InfoSize() const
			{
				uint8 result = (HasCRC() ? 2 : 0);

				if (IsMPEG2())
				{
					result += (IsStereo() ? 17 : 9);
				}
				else
				{
					result += (IsStereo() ? 32 : 17);
				}

				return (result);
			}

			inline uint8 Emphasis () const
			{
				return (m_Value[3] & 0x03);
			}

			inline uint32 FrameSize () const
			{
				return (BlockSize() - 4);
			}

			uint16 SamplingFrequency () const;
			uint16 BitRate () const;
			uint32 FrameDuration () const;

			Generics::DataElement Data () const
			{
				return (m_Value);
			}

		private:
			uint32 BlockSize() const;

		private:
			Generics::DataElement	m_Value;
	};

	class EXTERNAL MPEGHeader
	{
		public:
			static uint64 FindTag (const Generics::DataElement& data, const uint64 offset);

			static uint8 TagId (const Generics::DataElement& data, const uint64 offset)
			{
				return ((data.IsValid() && (data.Size() >= 4)) ? (data[static_cast<uint32>(offset + 3)]) : NUMBER_MAX_UNSIGNED(uint8) );
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (data.IsValid() && (data.Size() >= 4) && (data[static_cast<uint32>(offset)] == 0) && (data[static_cast<uint32>(offset)+1] == 0) && (data[static_cast<uint32>(offset)+2] == 1) );
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset, const uint8 tagId)
			{
				return (MPEGHeader::IsValid(data, offset) && (data[static_cast<uint32>(offset)+3] == tagId));
			}

		public:
			// Just find the first applicable MPEGHEader...
			MPEGHeader(const Generics::DataElement& data, const uint64 offset);

			// Finde the requested MPEG header.
			MPEGHeader(const uint8 tagId, const Generics::DataElement& data, const uint64 offset);

			// Default constructors for the derived classes 
			MPEGHeader() :
				m_Value(),
				m_TagId (NUMBER_MAX_UNSIGNED(uint16))
			{
			}
			MPEGHeader(const MPEGHeader& copy, const uint16 tagId = NUMBER_MAX_UNSIGNED(uint16) ) : 
				m_Value(copy.m_Value),
				m_TagId (tagId)
			{
			}
			~MPEGHeader ()
			{
			}

			MPEGHeader& operator= (const MPEGHeader& RHS)
			{
				m_Value = RHS.m_Value;
				m_TagId = RHS.m_TagId;

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (m_TagId == NUMBER_MAX_UNSIGNED(uint16) ? MPEGHeader::IsValid (m_Value, 0) : MPEGHeader::IsValid (m_Value, 0, static_cast<uint8>(m_TagId)) );
			}

			inline uint8 TagId () const
			{
				return (MPEGHeader::TagId(m_Value, 0));
			}

			const TCHAR* TagText () const;

			inline uint8& operator[] (const uint32 index)
			{
				return (m_Value[index]);
			}

			inline const uint8& operator[] (const uint32 index) const
			{
				return (m_Value[index]);
			}

			template <typename TYPENAME, const enum Generics::NumberEndian ENDIAN>
			inline TYPENAME GetNumber(const uint64 offset) const
			{
				return (m_Value.GetNumber<TYPENAME, ENDIAN>(offset));
			}

			inline uint32 Size () const
			{
				return (static_cast<uint32>(m_Value.Size()));
			}

			inline Generics::DataElement& Data()
			{
				return (m_Value);
			}

		protected:
			const Generics::DataElement& Data() const
			{
				return (m_Value);
			}
			inline uint64 FindTag (uint64 offset) const
			{
				return (MPEGHeader::FindTag(m_Value, offset));
			}
			void ExpectedLength (const uint32 length);

		private:
			Generics::DataElement		m_Value;
			uint16						m_TagId;			
	};

	class EXTERNAL UserDataHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = USER_DATA_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (tagId == UserDataHeader::ID);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, UserDataHeader::ID));
			}

		public:
			UserDataHeader(const MPEGHeader& base);
			UserDataHeader(const Generics::DataElement& data, const uint64 offset);

			UserDataHeader() :
				MPEGHeader(),
				m_UserText()
			{
			}
			UserDataHeader(const UserDataHeader& copy) : 
				MPEGHeader(copy, UserDataHeader::ID),
				m_UserText(copy.m_UserText)
			{
			}

			~UserDataHeader ()
			{
			}

			UserDataHeader& operator= (const UserDataHeader& RHS)
			{
				MPEGHeader::operator=(RHS);
				
				m_UserText = RHS.m_UserText;

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (UserDataHeader::IsValid(MPEGHeader::Data(), 0));
			}

			const Generics::TextFragment& UserData () const
			{
				return (m_UserText);
			}

		private:
			void UserDataHeader::ExtractUserData();

		private:
			Generics::TextFragment		m_UserText;
	};

	class EXTERNAL PictureHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = PICTURE_MARKER;

		public:
			typedef enum
			{
				UNKNOWN		= 0,
				INITIAL		= 1,
				PARTIAL		= 2,
				BACKWARD    = 3,
				FORWARD		= 4,

			}	enumFrameType;
			
		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (tagId == PictureHeader::ID);
			}
			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, PictureHeader::ID) && ((data[5] & 0x38) != 0) && (((data[5] & 0x38) >> 3) <= 4));
			}

		public:
			PictureHeader(const MPEGHeader& base);
			PictureHeader(const Generics::DataElement& data, const uint64 offset);

			PictureHeader() :
				MPEGHeader()
			{
			}

			PictureHeader(const PictureHeader& copy) : 
				MPEGHeader(copy, PictureHeader::ID)
			{
			}

			~PictureHeader ()
			{
			}

			PictureHeader& operator= (const PictureHeader& RHS)
			{
				MPEGHeader::operator=(RHS);
				
				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (PictureHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline uint16 SequenceNumber () const
			{
				return (Size() >= 6 ? GetNumber<uint16, Generics::ENDIAN_BIG>(4) >> 6 : 0);
			}

			inline enumFrameType FrameType () const
			{
				return (Size() >= 6 ? static_cast<enumFrameType> (((*this)[5] & 0x38) >> 3) : UNKNOWN);
			}

			inline uint16 Delay () const
			{
				return (Size() >= 8 ? ((((*this)[5]& 0x07) << 13) | ((*this)[6] << 5) | (((*this)[7]& 0xF8) >> 3)) : 0);
			}

			inline bool HasForwardVector () const
			{
				return (Size() >= 8 ? (((FrameType() == PARTIAL) || (FrameType() == BACKWARD)) ? (((*this)[7] & 0x04) != 0) : false) : false);
			}

			inline uint8 ForwardCode () const
			{
				return (Size() >= 9 ? (((FrameType() == PARTIAL) || (FrameType() == BACKWARD)) ? ((((*this)[7] & 0x03) << 1) | (((*this)[8] & 0x80) >> 7)) : 0) : 0);
			}

			inline bool BackwardVector () const
			{
				return (Size() >= 9 ? ((FrameType() == BACKWARD) ? (((*this)[8] & 0x40) != 0) : false) : false);
			}

			inline uint8 BackwardCode () const
			{
				return (Size() >= 9 ? ((FrameType() == BACKWARD) ? (((*this)[8] & 0x38) >> 3) : 0) : 0);
			}

		private:
			uint32 CalculateLength() const;
	};

	class EXTERNAL SliceHeader : public MPEGHeader
	{
		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return ((tagId >= 0x01) && (tagId <= 0xAF));
			}
			static bool IsValid (const MPEGHeader& data)
			{
				return (data.IsValid() && SliceHeader::IsValid(data.TagId()));
			}
			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset) && SliceHeader::IsValid (MPEGHeader::TagId(data,offset)));
			}

		public:
			SliceHeader(const MPEGHeader& data);
			SliceHeader(const Generics::DataElement& data, const uint64 offset);

			SliceHeader() :
				MPEGHeader()
			{
			}

			SliceHeader(const SliceHeader& copy) : 
				MPEGHeader(copy)
			{
			}

			~SliceHeader ()
			{
			}

			SliceHeader& operator= (const SliceHeader& RHS)
			{
				MPEGHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (SliceHeader::IsValid(MPEGHeader::Data(), 0));
			}

		private:
			inline uint32 CalculateLength() const;
	};

	class EXTERNAL SequenceHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = SEQUENCE_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (SequenceHeader::ID == tagId);
			}
			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, SequenceHeader::ID) && ((data[static_cast<uint32>(offset) + 7] & 0x0F) != 0x00) && 
                                                                                ((data[static_cast<uint32>(offset) + 7] & 0x0F) <= 0x08) && 
                                                                                ((data[static_cast<uint32>(offset) + 7] & 0xF0) != 0x00) && 
                                                                                ((data[static_cast<uint32>(offset) + 7] & 0xF0) <= 0x40) );
			}

		public:
			SequenceHeader(const MPEGHeader& data);
			SequenceHeader(const Generics::DataElement& data, const uint64 offset);

			SequenceHeader() :
				MPEGHeader()
			{
			}
			SequenceHeader(const SequenceHeader& copy) : 
				MPEGHeader(copy, SequenceHeader::ID)
			{
			}

			~SequenceHeader ()
			{
			}

			SequenceHeader& operator= (const SequenceHeader& RHS)
			{
				MPEGHeader::operator=(RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (SequenceHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline bool HasConstrainedParameters () const
			{
				return (((*this)[11] & 0x04)  != 0);
			}

			inline bool HasIntraQuantiserMatrix () const
			{
				return (((*this)[11] & 0x02) != 0);
			}

			inline bool HasNonIntraQuantiserMatrix () const
			{
				return (((*this)[11] & 0x01) != 0);
			}

			inline uint32 HorizontalSize () const
			{
				return ((GetNumber<uint16, Generics::ENDIAN_BIG>(4) >> 4));
			}

			inline uint32 VerticalSize () const
			{
				return (GetNumber<uint16, Generics::ENDIAN_BIG>(5) & 0x0FFF);
			}

			inline uint16 AspectRatio () const
			{
				return ((*this)[7] & 0xF0) >> 4;
			}

			inline double FrameRate () const
			{
				static double mpeg_frame_rate_index[] = { 0., 24000./1001., 24., 25., 30000./1001., 30., 50., 60000./1001., 60. };

				return (mpeg_frame_rate_index[(*this)[7] & 0x0F]);
			}

			inline uint32 BitRate () const
			{
				return (((GetNumber<uint32, Generics::ENDIAN_BIG>(8)) >> 14) * 400);
			}

			inline uint16 BufferSize () const
			{
				return ((GetNumber<uint16, Generics::ENDIAN_BIG>(10) >> 3) & 0x3FF);
			}

		private:
			uint32 CalculateLength() const;
	};

	class EXTERNAL ExtendedSequenceHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = SEQUENCE_EXTENDED_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (ExtendedSequenceHeader::ID == tagId);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, ExtendedSequenceHeader::ID) && ( ((data[static_cast<uint32>(offset) + 4] & 0xF0) == 0x10) || 
																	                       ((data[static_cast<uint32>(offset) + 4] & 0xF0) == 0x20) || 
																                           ((data[static_cast<uint32>(offset) + 4] & 0xF0) == 0x80) ));
			}

		public:
			ExtendedSequenceHeader(const MPEGHeader& data);
			ExtendedSequenceHeader(const Generics::DataElement& data, const uint64 offset);

			ExtendedSequenceHeader() :
				MPEGHeader()
			{
			}

			ExtendedSequenceHeader(const ExtendedSequenceHeader& copy) : 
				MPEGHeader(copy, ExtendedSequenceHeader::ID)
			{
			}

			~ExtendedSequenceHeader ()
			{
			}

			ExtendedSequenceHeader& operator= (const ExtendedSequenceHeader& RHS)
			{
				MPEGHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (ExtendedSequenceHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline bool IsSequenceExtension () const
			{
				return (((*this)[4] & 0xF0) == 0x10); 
			}

			inline bool IsDisplayExtension () const
			{
				return (((*this)[4] & 0xF0) == 0x20); 
			}

			inline bool IsPictureExtension () const
			{
				return (((*this)[4] & 0xF0) == 0x80); 
			}
			//-------------------------------------------------------------------
			// These are the SequenceExtension properties
			//-------------------------------------------------------------------
			inline bool IsLowDelay () const
			{
				return (((*this)[9] & 0x80) != 0);
			}

			inline bool IsProgressive () const
			{
				return (((*this)[5] & 0x08) != 0);
			}

			inline uint8 ExtendedVideoFormat () const
			{
				return (((*this)[4] & 0x0E) >> 1);
			}

			inline uint32 HorizontalSize () const
			{
				return (((((*this)[5] & 0x01) << 1) | (((*this)[6] & 0x80) >> 7)) << 12);
			}

			inline uint32 VerticalSize () const
			{
				return (((*this)[6] & 0x60) << 7);
			}

			inline uint32 BitRate () const
			{
				return (((((*this)[6] & 0x1F) << 7) | (((*this)[7] & 0xFE) >> 1)) << 18);
			}

			inline double FrameRate () const
			{
				return (((((*this)[9] & 0x60) >> 5)+1) / (((*this)[9] & 0x1F)+1));
			}

			inline uint8 ChromaFormat () const
			{
				return (((*this)[5] & 0x06) >> 1);
			}

			//-------------------------------------------------------------------
			// These are the DisplayExtension properties
			//-------------------------------------------------------------------
			inline bool HasColorDescription () const
			{
				return (((*this)[4] & 0x01) != 0);
			}
			inline uint8 ColorPrimaries () const
			{
				return ((((*this)[4] & 0x01) == 0) ? 0 : (*this)[5]);
			}

			inline uint8 TransferCharacteristics () const
			{
				return ((((*this)[4] & 0x01) == 0) ? 0 : (*this)[6]);
			}

			inline uint8 MatrixCoefficient () const
			{
				return ((((*this)[4] & 0x01) == 0) ? 0 : (*this)[7]);
			}

			inline uint16 HorizontalDisplaySize () const
			{
				uint32 index = ((((*this)[4] & 0x01) == 0) ? 5 : 8);
			
				return ( ((*this)[index] << 6) | (((*this)[index+1] & 0xFC) >> 2));
			}

			inline uint32 VerticalDisplaySize () const
			{
				uint32 index = ((((*this)[0] & 0x01) == 0) ? 6 : 9);
			
				return ((((*this)[index] & 0x01) << 13) | ((*this)[index+1] << 5) | (((*this)[index+2] & 0xF8) >> 3));
			}

			//-------------------------------------------------------------------
			// These are the PictureExtension properties
			//-------------------------------------------------------------------
			inline bool HasCompositeDisplay () const
			{
				return (((*this)[8] & 0x40) != 0);
			}

		private:
			uint32 CalculateLength() const;
	};

	class EXTERNAL GroupOfPicturesHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = GROUP_OF_PICTURES_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (GroupOfPicturesHeader::ID == tagId);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset,  GroupOfPicturesHeader::ID));
			}

		public:
			GroupOfPicturesHeader(const MPEGHeader& data);
			GroupOfPicturesHeader(const Generics::DataElement& data, const uint64 offset);

			GroupOfPicturesHeader() :
				MPEGHeader()
			{
			}

			GroupOfPicturesHeader(const GroupOfPicturesHeader& copy) : 
				MPEGHeader(copy, GroupOfPicturesHeader::ID)
			{
			}

			~GroupOfPicturesHeader ()
			{
			}

			GroupOfPicturesHeader& operator= (const GroupOfPicturesHeader& RHS)
			{
				MPEGHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (GroupOfPicturesHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline bool IsDropFrame () const
			{
				return (((*this)[4] & 0x80)  != 0);
			}

			inline bool IsClosed () const
			{
				return (((*this)[7] & 0x40) != 0);
			}

			inline bool IsBroken () const
			{
				return (((*this)[7] & 0x20) != 0);
			}

			inline uint8 Frame () const
			{
				return ((((*this)[6] & 0x1F) << 1) | ((*this)[7] >>7));
			}

			// Time in mS
			inline uint32 TimeStamp () const 
			{
				return (  ((((*this)[4] >> 2) & 0x1F) * 60 * 60) +					// Hours to seconds
                         (((((*this)[4] & 0x03) << 4) | ((*this)[5] >> 4)) * 60) +	// Minutes to seconds
						  ((((*this)[5] & 0x07) << 3) | ((*this)[6] >> 5))       ) * 1000; // turn seconds into milliseconds
			}

		private:
			inline uint32 CalculateLength() const
			{
				return (MPEGHeader::IsValid() ? 8 : 0);
			}
	};

	class EXTERNAL ElementaryStreamHeader : public MPEGHeader
	{
		public:
			static bool IsValid (const uint8 tag)
			{
				return ((tag >= 0xBD) && (tag <= 0xEF));
			}

			static bool IsValid (const MPEGHeader& data)
			{
				return (data.IsValid() && ElementaryStreamHeader::IsValid(data.TagId()));
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (ElementaryStreamHeader::IsValid(MPEGHeader(data,offset)));
			}

		public:
			ElementaryStreamHeader(const MPEGHeader& data);
			ElementaryStreamHeader(const Generics::DataElement& data, const uint64 offset);
			ElementaryStreamHeader(const Generics::DataElement& data, const uint64 offset, const uint8 streamId);

			ElementaryStreamHeader() :
				MPEGHeader()
			{
			}

			ElementaryStreamHeader(const ElementaryStreamHeader& copy) : 
				MPEGHeader(copy)
			{
			}

			~ElementaryStreamHeader ()
			{
			}

			ElementaryStreamHeader& operator= (const ElementaryStreamHeader& RHS)
			{
				MPEGHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (ElementaryStreamHeader::IsValid(static_cast<const MPEGHeader&>(*this)));
			}

			inline bool IsAudio () const
			{
				return (((TagId() & 0xF0) == 0xC0) || ((TagId() & 0xF0) == 0xD0));
			}

			inline bool IsVideo () const
			{
				return ((TagId() & 0xF0) == 0xE0);
			}

			inline Generics::DataElement Frame () const
			{
				return (IsValid() && (ElementaryStreamHeader::HeaderSize() < Size()) ? Generics::DataElement (Data(), ElementaryStreamHeader::HeaderSize()) : Generics::DataElement ());
			}

			//---------------------------------------------------------------------
			// Does this Stream/Package have an Extension
			//---------------------------------------------------------------------
			inline bool HasExtension () const
			{
				return ((IsAudio () || IsVideo() || (TagId() == 0xBD)) && (ExtendedExtensionOffset() + ExtendedExtensionSize() <= static_cast<uint8>((*this)[8] + static_cast<uint8>(9))));
			}

			inline bool IsScambled () const
			{
				return (((*this)[6] & 0x30) != 0);
			}

			inline bool HasPriority () const
			{
				return (((*this)[6] & 0x08) != 0);
			}

			inline bool IsDataAlignment () const
			{
				return (((*this)[6] & 0x04) != 0);
			}

			inline bool IsCopyrighted () const
			{
				return (((*this)[6] & 0x02) != 0);
			}

			inline bool IsOriginal () const
			{
				return (((*this)[6] & 0x01) != 0);
			}

			//---------------------------------------------------------------------
			// Does this Stream/Package have an Extension
			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint64> PresentationTimeStamp () const
			{
				if (HasPresentationTimeStamp())
				{
					return Generics::OptionalType<uint64> ( (((*this)[9] & 0x0E)  << 30) | 
															 ((*this)[10] << 22)         | 
															(((*this)[11] & 0xFE) << 14) | 
															 ((*this)[12] << 7)          | 
															(((*this)[13] & 0xFE) >> 1)  );
				}

				return(Generics::OptionalType<uint64>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint64> DecodeTimeStamp () const
			{
				if (HasDecoderTimeStamp())
				{
					return Generics::OptionalType<uint64>( (((*this)[14] & 0x0E)  << 30) | 
															((*this)[15] << 22)         | 
														   (((*this)[16] & 0xFE) << 14) | 
															((*this)[17] << 7)          | 
														   (((*this)[18] & 0xFE) >> 1)  );
				}

				return(Generics::OptionalType<uint64>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<ReferenceClock> StreamClock () const
			{
				if (HasStreamClock())
				{
					uint32 index = StreamClockOffset();

					uint64 clock = ((((*this)[index]   >> 3) & 0x07) << 30) | 
									(((*this)[index]   & 0x3) << 28)        | 
									 ((*this)[index+1] << 20)               | 
									(((*this)[index+2] & 0xF8) << 15)       | 
									(((*this)[index+2] & 0x03) << 13)       | 
									 ((*this)[index+3] << 5)                | 
									(((*this)[index+4] & 0xF8) >> 3)        ;
					uint16 ext =   ((((*this)[index+4] & 0x03) << 7)        |
									 ((*this)[index+5] >> 1))               ;

					return (Generics::OptionalType<ReferenceClock>(ReferenceClock(clock, ext)));
				}

				return (Generics::OptionalType<ReferenceClock>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint32> StreamRate () const
			{
				if (HasStreamRate())
				{
					uint32 index = StreamRateOffset();

					return Generics::OptionalType<uint32> ((((*this)[index]   << 14) |
															((*this)[index+1] << 6)  |
															((*this)[index+2] >> 2)  ) * 50);
				}

				return (Generics::OptionalType<uint32>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint8> CopyInfo () const
			{
				if (HasCopyInfo())
				{
					uint32 index = CopyInfoOffset();

					return Generics::OptionalType<uint8> ((*this)[index] & 0x7F);
				}

				return (Generics::OptionalType<uint8>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint32> CRC () const
			{
				if (HasCRC())
				{
					uint32 index = CRCOffset ();

					return Generics::OptionalType<uint32>((*this).GetNumber<uint16, Generics::ENDIAN_BIG>(index));
				}

				return (Generics::OptionalType<uint32>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint32> UserData () const
			{
				if (HasUserData())
				{
					return (Generics::OptionalType<uint32>(UserDataOffset()));
				}

				return(Generics::OptionalType<uint32>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint8> PackSize () const
			{
				if (HasPackSize())
				{
					return (Generics::OptionalType<uint8>((*this)[PackSizeOffset()]));
				}

				return (Generics::OptionalType<uint8>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<uint8> PacketSequenceCounter () const
			{
				if (HasPacketSequenceCounter())
				{
					return (Generics::OptionalType<uint8>((*this)[PacketSequenceOffset()] & 0x7F));
				}

				return (Generics::OptionalType<uint8>());
			}

			inline bool IsMPEG2 () const
			{
				return (HasPacketSequenceCounter() && (((*this)[PacketSequenceOffset() + 1] & 0x40) != 0));
			}

			inline Generics::OptionalType<uint8> StuffingLength () const
			{
				if (HasPacketSequenceCounter())
				{
					return (Generics::OptionalType<uint8>((*this)[PacketSequenceOffset() + 1] & 0x3F));
				}

				return (Generics::OptionalType<uint8>());
			}

			//---------------------------------------------------------------------
			inline uint8 Is128Buffer () const
			{
				return (HasBuffer() ? (((*this)[BufferOffset()] & 0x20) == 0) : false);
			}

			inline uint8 Is1024Buffer () const
			{
				return (HasBuffer() ? (((*this)[BufferOffset()] & 0x20) != 0) : false);
			}

			inline Generics::OptionalType<uint8> BufferSize () const
			{
				if (HasBuffer())
				{
					uint32 index = BufferOffset();

					return (Generics::OptionalType<uint8>((((*this)[index] & 0x1F) << 8) | (*this)[index+1]));
				}

				return (Generics::OptionalType<uint8>());
			}

			//---------------------------------------------------------------------

			inline Generics::OptionalType<Generics::DataElement> ExtendedExtension () const
			{
				if (HasExtendedExtension())
				{
					uint32 offset = ExtendedExtensionOffset();
					uint8 size = (*this)[offset] & 0x7F;
				
					return (Generics::OptionalType<Generics::DataElement>(Generics::DataElement(Data(), offset + 2, size)));
				}

				return (Generics::OptionalType<Generics::DataElement>());
			}

			//---------------------------------------------------------------------

			inline uint32 HeaderSize() const
			{
				return (HasExtension() ? (*this)[8] + 9 : 6);
			}

		private:
			inline bool HasPresentationTimeStamp () const
			{
				return (((*this)[7] & 0x80) == 0x80);
			}
			inline bool HasDecoderTimeStamp () const
			{
				return (((*this)[7] & 0x40) == 0x40);
			}
			inline bool HasStreamClock () const
			{
				return (((*this)[7] & 0x20) == 0x20);
			}
			inline bool HasStreamRate() const
			{
				return (((*this)[7] & 0x10) == 0x10);
			}
			inline bool HasCopyInfo () const
			{
				return (((*this)[7] & 0x04) == 0x04);
			}
			inline bool HasCRC () const
			{
				return (((*this)[7] & 0x02) == 0x02);
			}
			inline bool HasPackSize () const
			{
				return (PESExtensionFlag(0x40));
			}
			inline bool HasPacketSequenceCounter () const
			{
				return (PESExtensionFlag(0x20));
			}
			inline bool HasBuffer () const
			{
				return (PESExtensionFlag(0x10));
			}
			inline bool HasUserData () const
			{
				return (PESExtensionFlag(0x80));
			}
			inline bool HasExtendedExtension () const
			{
				return (PESExtensionFlag(0x01));
			}
			inline uint32 StreamClockOffset () const
			{
				return (((*this)[7] & 0xC0) == 0 ? 9 : (((*this)[7] & 0xC0) == 0xC0 ? 19 : 14));
			}
			inline uint32 StreamRateOffset () const
			{
				return (StreamClockOffset() + (HasStreamClock() ? 6 : 0));
			}
			inline uint32 CopyInfoOffset () const
			{
				return (StreamRateOffset() + (HasStreamRate() ? 3 : 0));
			}
			inline uint32 CRCOffset () const
			{
				return (CopyInfoOffset() + (HasCopyInfo() ? 1 : 0));
			}
			inline uint32 PESExtensionOffset () const
			{
				return (CRCOffset() + (HasCRC() ? 2 : 0));
			}
			inline bool PESExtensionFlag(const uint8 flag) const
			{
				return (((*this)[7] & 0x01) == 0 ? false : (((*this)[PESExtensionOffset()] & flag) != 0));
			}
			inline uint32 UserDataOffset () const
			{
				return (PESExtensionOffset() + (HasUserData() ? 1 : 0));
			}
			inline uint32 PackSizeOffset () const 
			{
				return (UserDataOffset() + (HasUserData() ? 16 : 0));
			}
			inline uint32 PacketSequenceOffset () const 
			{
				return (PackSizeOffset() + (HasPackSize() ? 1 : 0));
			}
			inline uint32 BufferOffset () const 
			{
				return (PacketSequenceOffset() + (HasPacketSequenceCounter() ? 2 : 0));
			}
			inline uint32 ExtendedExtensionOffset () const 
			{
				return (BufferOffset() + (HasBuffer() ? 2 : 0));
			}
			inline uint8 ExtendedExtensionSize() const
			{
				return (HasExtendedExtension() ? (*this)[ExtendedExtensionOffset()] & 0x7F : 0);
			}

		private:
			uint32 CalculateLength() const;
			uint32 CalculatedHeaderSize() const;
	};

	class EXTERNAL PackStreamHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = PACK_STREAM_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (PackStreamHeader::ID == tagId);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, PackStreamHeader::ID) && ( IsPacked(data, offset)  ||
					                                                               (((data[static_cast<uint32>(offset) +  4] & 0xC4) == 0x44) && 
																	                ((data[static_cast<uint32>(offset) +  8] & 0x04) == 0x04) && 
																	                ((data[static_cast<uint32>(offset) + 12] & 0x03) == 0x03) )));
			}

		public:
			PackStreamHeader(const MPEGHeader& data);
			PackStreamHeader(const Generics::DataElement& data, const uint64 offset);

			PackStreamHeader() :
				MPEGHeader()
			{
			}
			PackStreamHeader(const PackStreamHeader& copy) : 
				MPEGHeader(copy, PackStreamHeader::ID)
			{
			}

			~PackStreamHeader ()
			{
			}

			PackStreamHeader& operator= (const PackStreamHeader& RHS)
			{
				MPEGHeader::operator=(RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (PackStreamHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline uint64 SystemClock () const
			{
				if (IsPacked ())
				{
					return ((*this)[4] << 25) | 
						   ((*this)[5] << 17) |
						   ((*this)[6] <<  9) |
						   ((*this)[7] <<  1) |
						   (((*this)[8] & 0x80) >> 7);
				}

				return ((((*this)[4]   >> 3) & 0x07) << 30) | 
						(((*this)[4]  & 0x3) << 28)         | 
						 ((*this)[5] << 20)                 | 
						(((*this)[6] & 0xF8) << 15)         | 
						(((*this)[6] & 0x03) << 13)         | 
						 ((*this)[7] << 5)                  | 
						(((*this)[8] & 0xF8) >> 3)          ;
			}

			inline uint16 ExternalSystemClock () const
			{
				if (IsPacked ())
				{
					return ((((*this)[8] & 0x7F) << 2) |
							 ((*this)[9] >> 6)         );
				}
				return ((((*this)[8] & 0x03) << 7) |
						 ((*this)[9] >> 1)          );
			}

			inline uint32 MuxRate () const
			{
				if (IsPacked ())
				{
					return ((((*this)[9] & 0x3F) << 16) |
						   	 ((*this)[10] << 8)  |
							 ((*this)[11]) ) * 50;
				}
				return (((*this)[10] << 14) |
						((*this)[11] << 6)  |
						((*this)[12] >> 2)  ) * 50;
			}

		private:
			static bool IsPacked (const Generics::DataElement& data, const uint64 offset)
			{
				return ((data[static_cast<uint32>(offset + 12)] & 0x03) != 0x03);
			}
			inline bool IsPacked () const
			{
				return (IsPacked((*this).Data(), 0));
			}
			uint32 CalculateLength() const;
	};

	class EXTERNAL SystemStreamHeader : public MPEGHeader
	{
		public:
			class Iterator
			{
				public:
					Iterator() :
						m_Audio(0),
						m_Video(0),
						m_Index(0xFF),
						m_End(0),
						m_Value()
					{
					}
					Iterator(const Generics::DataElement& data, const uint64 offset) :
						m_Audio(0),
						m_Video(0),
						m_Index(0xFF),
						m_End(0),
						m_Value(data, offset)
					{
						while ( (m_End < m_Value.Size ()) && ((m_Value[m_End] & 0x80) == 0x80) )
						{
							if (((m_Value[m_End] & 0xF0) == 0xC0) || ((m_Value[m_End] & 0xF0) == 0xD0))
							{
								++m_Audio;
							}
							else if ((m_Value[m_Index] & 0xF0) == 0xE0)
							{
								++m_Video;
							}

							m_End += 3;
						}

					}
					Iterator(const Iterator& copy) :
						m_Audio(copy.m_Audio),
						m_Video(copy.m_Video),
						m_Index(0xFF),
						m_End(copy.m_End),
						m_Value(copy.m_Value)
					{
					}
					~Iterator()
					{
					}
				
					Iterator& operator= (const Iterator& RHS)
					{
						m_Audio = RHS.m_Audio;
						m_Video = RHS.m_Video;
						m_Index = RHS.m_Index;
						m_End = RHS.m_End;
						m_Value = RHS.m_Value;

						return (*this);
					}

				public:
					inline bool IsValid () const
					{
						return ((m_Index != 0xFF) && (m_Index < m_End));
					}
					inline void Reset ()
					{
						m_Index = 0xFF;
					}

					inline bool Next ()
					{
						if (m_Index == 0xFF)
						{
							m_Index = 0;
						}
						else if (m_Index < m_End)
						{
							m_Index += 3;
						}

						return (IsValid());
					} 

					//---------------------------------------------------
					// currently pointed at element.
					//---------------------------------------------------
					inline uint8 ID () const
					{
						return (m_Value[m_Index]);
					}

					inline bool IsAudio () const
					{
						return (((m_Value[m_Index] & 0xF0) == 0xC0) || ((m_Value[m_Index] & 0xF0) == 0xD0));
					}

					inline bool IsVideo () const
					{
						return ((m_Value[m_Index] & 0xF0) == 0xE0);
					}

					inline bool HasBufferBoundScale () const
					{
						return ((m_Value[m_Index+1] & 0x20) != 0);
					}

					inline uint16 BufferSize () const
					{
						return (((m_Value[m_Index+1] & 0x1F) << 8) | m_Value[m_Index+2]);
					}

					//---------------------------------------------------
					// Iterator overall numbers.
					//---------------------------------------------------
					inline uint16 Count() const
					{
						return (m_End / 3);
					}

					inline uint8 AudioStreams () const
					{
						return (m_Audio);
					}

					inline uint8 VideoStreams () const
					{
						return (m_Video);
					}

				private:
					uint8						m_Audio;
					uint8						m_Video;
					uint16						m_Index;
					uint16						m_End;
					Generics::DataElement		m_Value;
			};

		private:
			static const uint8 ID = SYSTEM_STREAM_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (SystemStreamHeader::ID == tagId);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, SystemStreamHeader::ID) && ((data[static_cast<uint32>(offset)+11] & 0x7F) == 0x7F));
			}

		public:
			SystemStreamHeader(const MPEGHeader& data);
			SystemStreamHeader(const Generics::DataElement& data, const uint64 offset);

			SystemStreamHeader() :
				MPEGHeader(),
				m_Streams()
			{
			}
			SystemStreamHeader(const SystemStreamHeader& copy) : 
				MPEGHeader(copy, SystemStreamHeader::ID),
				m_Streams(copy.m_Streams)
			{
			}

			~SystemStreamHeader ()
			{
			}

			SystemStreamHeader& operator= (const SystemStreamHeader& RHS)
			{
				MPEGHeader::operator=(RHS);

				m_Streams = RHS.m_Streams;
				
				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (SystemStreamHeader::IsValid(MPEGHeader::Data(), 0));
			}

			inline uint32 Length () const
			{
				return (IsValid() ? (*this).GetNumber<uint16, Generics::ENDIAN_BIG>(4) : 0);
			}

			inline uint32 RateBoundary () const
			{
				return (((*this)[6] & 0x7F) << 15) | ((*this)[7] << 7) | ((*this)[8] >> 1);
			}

			inline uint8 AudioStreams () const
			{
				return (((*this)[9] & 0xFC) >> 2);
			}

			inline uint8 VideoStreams () const
			{
				return ((*this)[10] & 0x1F);
			}

			inline bool HasFixedBitrate () const
			{
				return (((*this)[9] & 0x02) != 0);
			}

			inline bool HasConstrainedSystemParameters () const
			{
				return (((*this)[9] & 0x01) != 0);
			}

			inline bool HasSystemAudioLock () const
			{
				return (((*this)[9] & 0x80) != 0);
			}

			inline bool HasSystemVideoLock () const
			{
				return (((*this)[10] & 0x40) != 0);
			}

			inline bool HasPacketRateRestriction () const
			{
				return (((*this)[11] & 0x80) != 0);
			}

			inline Iterator Streams () const
			{
				return (m_Streams);
			}

		private:
			uint32 CalculateLength() const;

		private:
			mutable Iterator			m_Streams;
	};

	class EXTERNAL PaddingStreamHeader : public MPEGHeader
	{
		private:
			static const uint8 ID = PADDING_STREAM_MARKER;

		public:
			inline static bool IsValid (const uint8 tagId)
			{
				return (PaddingStreamHeader::ID == tagId);
			}

			static bool IsValid (const Generics::DataElement& data, const uint64 offset)
			{
				return (MPEGHeader::IsValid(data, offset, PaddingStreamHeader::ID));
			}

		public:
			PaddingStreamHeader(const MPEGHeader& data);
			PaddingStreamHeader(const Generics::DataElement& data, const uint64 offset);

			PaddingStreamHeader() :
				MPEGHeader()
			{
			}
			PaddingStreamHeader(const PaddingStreamHeader& copy) : 
				MPEGHeader(copy, PaddingStreamHeader::ID)
			{
			}

			~PaddingStreamHeader ()
			{
			}

			PaddingStreamHeader& operator= (const PaddingStreamHeader& RHS)
			{
				MPEGHeader::operator=(RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				// Check if the buffer is valid, the header is OK, and certain faulty bits do not exits.
				return (PaddingStreamHeader::IsValid(MPEGHeader::Data(), 0));
			}

		private:
			uint32 CalculateLength() const;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGHEADER_H