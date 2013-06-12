#ifndef __AVIMEDIAHEADER_H
#define __AVIMEDIAHEADER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "RIFFHeader.h"
#include "AVIMediaIterator.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace AVI 
{
	class EXTERNAL VideoFormat : public RIFF::ChunkHeader
	{
		FOURCCCODE('s', 't', 'r', 'f');

		protected:
			static const uint32 g_HeaderSize = 40;

		public:
			VideoFormat() :
				ChunkHeader()
			{
			}
			VideoFormat(const RIFF::ChunkHeader& buffer) :
				RIFF::ChunkHeader(buffer)
			{
			}
			VideoFormat(const VideoFormat& copy) :
				RIFF::ChunkHeader (copy)
			{
			}
			~VideoFormat()
			{
			}

			VideoFormat& operator= (const VideoFormat& RHS)
			{
				RIFF::ChunkHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return (RIFF::ChunkHeader::Size() >= g_HeaderSize);
			}

			inline uint32 Size () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(0));
			}
			inline void Size (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, value);
			}

			inline uint32 Width () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(4));
			}
			inline void Width (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(4, value);
			}

			inline uint32 Height () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
			}
			inline void Height (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(8, value);
			}

			inline uint16 Planes () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(12));
			}
			inline void Planes (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(12, value);
			}

			inline uint16 BitCount () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(14));
			}
			inline void BitCount (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(14, value);
			}

			inline uint32 Compression () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(16));
			}
			inline void Compression (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(16, value);
			}

			inline uint32 ImageSize () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(20));
			}
			inline void ImageSize (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(20, value);
			}

			inline sint32 XPelsPerMeter () const
			{
				return (RIFF::ChunkHeader::GetNumber<sint32, Generics::ENDIAN_LITTLE>(24));
			}
			inline void XPelsPerMeter (const sint32 value)
			{
				RIFF::ChunkHeader::SetNumber<sint32, Generics::ENDIAN_LITTLE>(24, value);
			}

			inline sint32 YPelsPerMeter () const
			{
				return (RIFF::ChunkHeader::GetNumber<sint32, Generics::ENDIAN_LITTLE>(28));
			}
			inline void YPelsPerMeter (const sint32 value)
			{
				RIFF::ChunkHeader::SetNumber<sint32, Generics::ENDIAN_LITTLE>(28, value);
			}

			inline uint32 ClrUsed () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(32));
			}
			inline void ClrUsed (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(32, value);
			}

			inline uint32 ClrImportant () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(36));
			}
			inline void ClrImportant (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(36, value);
			}

		private:
			Generics::DataElement		m_Buffer;
	};

	class EXTERNAL AudioFormat : public RIFF::ChunkHeader
	{
		FOURCCCODE('s', 't', 'r', 'f');

		protected:
			static const uint32 g_HeaderSize = 18;

		public:
			AudioFormat() :
				RIFF::ChunkHeader ()
			{
			}
			AudioFormat(const RIFF::ChunkHeader& buffer) :
				RIFF::ChunkHeader (buffer)
			{
			}
			AudioFormat(const AudioFormat& copy) :
				RIFF::ChunkHeader (copy)
			{
			}
			~AudioFormat()
			{
			}

			AudioFormat& operator= (const AudioFormat& RHS)
			{
				RIFF::ChunkHeader::operator= (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return (RIFF::ChunkHeader::Size() >= g_HeaderSize);
			}

			// 0x0161	Windows Media Audio. This format is valid for versions 2 through 9
			// 0x0162	Windows Media Audio 9 Professional
			// 0x0163	Windows Media Audio 9 Lossless
			// 0x0001	PCM audio
			// 0xFFFE	Indicates that the format is specified in the WAVEFORMATEXTENSIBLE.SubFormat member.
			inline uint16 FormatTag () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(0));
			}
			inline void FormatTag (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(0, value);
			}

			inline uint16 Channels () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(2));
			}
			inline void Channels (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(2, value);
			}

			inline uint32 SampleRate () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(4));
			}
			inline void SampleRate (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(4, value);
			}

			inline uint32 ByteRate () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
			}
			inline void ByteRate (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(8, value);
			}

			inline uint16 BlockAlignSize () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(12));
			}
			inline void BlockAlignSize (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(12, value);
			}

			inline uint16 BitRate () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(14));
			}
			inline void BitRate (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(14, value);
			}

			/* the count in bytes of the size of extra information (after cbSize) */
			inline uint16 Size () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(16));
			}
			inline void Size (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(16, value);
			}
	};

	class EXTERNAL MPEGAudioFormat : public AudioFormat
	{
		FOURCCCODE('s', 't', 'r', 'f');

		protected:
			static const uint32 g_HeaderSize = AudioFormat::g_HeaderSize + 10;

		public:
			MPEGAudioFormat() :
				AudioFormat()
			{
			}
			MPEGAudioFormat(const RIFF::ChunkHeader& buffer) :
				AudioFormat(buffer)
			{
			}
			MPEGAudioFormat(const MPEGAudioFormat& copy) :
				AudioFormat(copy)
			{
			}
			~MPEGAudioFormat()
			{
			}

			MPEGAudioFormat& operator= (const MPEGAudioFormat& RHS)
			{
				AudioFormat::operator = (RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return (RIFF::ChunkHeader::Size() >= g_HeaderSize); 
			}

			inline uint16 ID () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize));
			}
			inline void ID (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize, value);
			}

			inline uint32 Flags () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint32, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+2));
			}
			inline void Flags (const uint32 value)
			{
				RIFF::ChunkHeader::SetNumber<uint32, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+2, value);
			}

			inline uint16 BlockSize () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+4));
			}
			inline void BlockSize (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+4, value);
			}

			inline uint16 FramesPerBlock () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+6));
			}
			inline void FramesPerBlock (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+6, value);
			}

			inline uint16 CodecDelay () const
			{
				return (RIFF::ChunkHeader::GetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+8));
			}
			inline void CodecDelay (const uint16 value)
			{
				RIFF::ChunkHeader::SetNumber<uint16, Generics::ENDIAN_LITTLE>(AudioFormat::g_HeaderSize+8, value);
			}
	};

	class EXTERNAL Stream
	{
		FOURCCCODE('s', 't', 'r', 'h');

		private:
			static const uint32 g_HeaderSize = 56;

		public:
			Stream() :
			    m_Channel(0xFF),
				m_Storage (),
				m_Buffer (),
				m_Format()
			{
			}
			Stream(const RIFF::LISTHeader::Iterator& indexer, const Generics::DataElement& storage, const uint8 channel);
			Stream(const Stream& copy) :
				m_Channel(copy.m_Channel),
				m_Storage(copy.m_Storage),
				m_Buffer(copy.m_Buffer),
				m_Format(copy.m_Format),
				m_Index(copy.m_Index),
				m_Name(copy.m_Name)
			{
			}
			~Stream()
			{
			}

			Stream& operator= (const Stream& RHS)
			{
				m_Channel = RHS.m_Channel;
				m_Storage = RHS.m_Storage;
				m_Buffer = RHS.m_Buffer;
				m_Format = RHS.m_Format;
				m_Index = RHS.m_Index;
				m_Name = RHS.m_Name;

				return (*this);
			}

		public:
			class EXTERNAL Rectangle
			{
				public:
					Rectangle () :
						m_Left (0), m_Top(0), m_Right(0), m_Bottom(0)
					{
					}
					Rectangle (const uint16 left, const uint16 top, const uint16 right, const uint16 bottom) :
						m_Left (left), m_Top(top), m_Right(right), m_Bottom(bottom)
					{
					}
					Rectangle (const Rectangle& copy) :
						m_Left (copy.m_Left), m_Top(m_Top), m_Right(m_Right), m_Bottom(m_Bottom)
					{
					}
					~Rectangle ()
					{
					}
						
					Rectangle& operator= (const Rectangle& RHS)
					{
						m_Left = RHS.m_Left;
						m_Top = RHS.m_Top;
						m_Right = RHS.m_Right;
						m_Bottom = RHS.m_Bottom;

						return (*this);
					}

				public:
					inline uint16 Top () const
					{
						return (m_Top);
					}
					inline uint16 Bottom () const
					{
						return (m_Bottom);
					}
					inline uint16 Left () const
					{
						return (m_Left);
					}
					inline uint16 Right () const
					{
						return (m_Right);
					}
					inline void Top (const uint16 value)
					{
						m_Top = value;
					}
					inline void Bottom (const uint16 value)
					{
						m_Bottom = value;
					}
					inline void Left (const uint16 value)
					{
						m_Left = value;
					}
					inline void Right (const uint16 value)
					{
						m_Right = value;
					}

				private:
					uint16	m_Left;
					uint16	m_Top;
					uint16	m_Right;
					uint16	m_Bottom;
			};

			inline bool IsValid () const
			{
				return (m_Buffer.Size() >= g_HeaderSize); 
			}

			inline StreamType Type () const
			{
				RIFF::FourCC type (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(0));

				if (type == RIFF::FourCC(_T("vids")))
				{
					return (VIDEO);
				}
				else if (type == RIFF::FourCC(_T("auds")))
				{
					return (AUDIO);
				}
				else if (type == RIFF::FourCC(_T("txts")))
				{
					return (TEXT);
				}

				return (UNKNOWN);
			}

			inline void Type (const StreamType type)
			{
				if (type == VIDEO)
				{
					m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, RIFF::FourCC(_T("vids")).Value());
				}
				else if (type == AUDIO)
				{
					m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, RIFF::FourCC(_T("auds")).Value());
				}
				else if (type == TEXT)
				{
					m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, RIFF::FourCC(_T("txts")).Value());
				}
			}

			inline RIFF::FourCC Handler () const
			{
				return (RIFF::FourCC(m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(4)));
			}
			inline void Handler (const RIFF::FourCC& value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(4, value.Value());
			}

			inline uint32 Flags () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
			}
			inline void Flags (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(8, value);
			}

			inline uint16 Priority () const
			{
				return (m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(12));
			}
			inline void Priority (const uint16 value)
			{
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(12, value);
			}

			inline uint16 Language () const
			{
				return (m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(14));
			}
			inline void Language (const uint16 value)
			{
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(14, value);
			}

			inline uint32 InitialFrames () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(16));
			}
			inline void InitialFrames (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(16, value);
			}

			inline uint32 Scale () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(20));
			}
			inline void Scale (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(20, value);
			}

			inline uint32 Rate () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(24));
			}
			inline void Rate (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(24, value);
			}

			inline uint32 Start () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(28));
			}
			inline void Start (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(28, value);
			}

			inline uint32 Length () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(32));
			}
			inline void Length (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(32, value);
			}

			inline uint32 SuggestedBufferSize () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(36));
			}
			inline void SuggestedBufferSize (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(36, value);
			}

			inline uint32 Quality () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(40));
			}
			inline void Quality (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(40, value);
			}

			inline uint32 SampleSize () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(44));
			}
			inline void SampleSize (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(44, value);
			}

			inline Rectangle Window () const
			{
				uint16 left = m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(48);
				uint16 top = m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(50);
				uint16 right = m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(52);
				uint16 bottom = m_Buffer.GetNumber<uint16, Generics::ENDIAN_LITTLE>(54);

				return (Rectangle (left, top, right, bottom));
			}
			inline void Window (const Rectangle& value)
			{
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(48, value.Left());
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(50, value.Top());
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(52, value.Right());
				m_Buffer.SetNumber<uint16, Generics::ENDIAN_LITTLE>(54, value.Bottom());
			}
			inline const RIFF::ChunkHeader& Format () const
			{
				return (m_Format);
			}

			inline PackageIterator Packages()
			{
				return (PackageIterator (m_Storage, m_Index, Type(), m_Channel));
			}

			inline const PackageIterator Packages() const
			{
				return (PackageIterator (m_Storage, m_Index, Type(), m_Channel));
			}

		private:
			uint8					m_Channel;
			Generics::DataElement	m_Storage;
			RIFF::ChunkHeader		m_Buffer;
			RIFF::ChunkHeader		m_Format;
			RIFF::LISTHeader		m_Index;
			Generics::String		m_Name;
	};

	class EXTERNAL StreamIterator
	{
		public:
			StreamIterator () :
				m_CountIndex(0xFF),
				m_Iterator(),
				m_Storage()
			{
			}
			StreamIterator (const RIFF::LISTHeader::Iterator& iterator, const Generics::DataElement& storage) :
				m_CountIndex(0xFF),
				m_Iterator(iterator),
				m_Storage(storage)
			{
				m_Iterator.Reset();
			}
			StreamIterator (const StreamIterator& copy) :
				m_CountIndex(copy.m_CountIndex),
				m_Iterator(copy.m_Iterator),
				m_Storage(copy.m_Storage)
			{
			}
			~StreamIterator()
			{
			}

			StreamIterator& operator= (const StreamIterator& RHS)
			{
				m_CountIndex = RHS.m_CountIndex;
				m_Iterator = RHS.m_Iterator;
				m_Storage = RHS.m_Storage;

				return (*this);
			}

		public:
			void Reset()
			{
				m_Iterator.Reset();
				m_CountIndex = 0xFF;
			}

			bool IsValid () const
			{
				return ( m_Iterator.IsValid());
			}

			bool Next ()
			{
				while (m_Iterator.Next() && ((m_Iterator.IsList() == false) || (m_Iterator.ID() != RIFF::FourCC(_T("strl")))))
				{
					/* Nothing todo */;
				}

				m_CountIndex++;

				if (IsValid())
				{
					m_CurrentStream = Stream(m_Iterator.List().Elements(), m_Storage, m_CountIndex);
				}

				return (IsValid());
			}

			const AVI::Stream& GetStream() const
			{
				ASSERT (IsValid ());

				return (m_CurrentStream);
			}

			inline AVI::StreamType Type () const
			{
				ASSERT (IsValid ());

				return (m_CurrentStream.Type());
			}

		private:
			uint8						m_CountIndex;
			RIFF::LISTHeader::Iterator	m_Iterator;
			Generics::DataElement		m_Storage;
			AVI::Stream					m_CurrentStream;
	};

	class EXTERNAL Main
	{
		FOURCCCODE('a', 'v', 'i', 'h');

		private:
			static const uint32 g_HeaderSize = 40;

		public:
			Main();
			Main(const Generics::DataElement& buffer);
			Main(const Main& copy);
			~Main();

			Main& operator= (const Main& RHS);

		public:
			inline bool IsValid () const
			{
				return (m_Buffer.Size() >= g_HeaderSize); 
			}

			inline uint32 MicroSecondsPerFrame () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(0));
			}
			inline void MicroSecondsPerFrame (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, value);
			}

			inline uint32 MaximumBytesPerSecond () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(4));
			}
			inline void MaximumBytesPerSecond (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(4, value);
			}

			inline uint32 PaddingGranularity () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
			}
			inline void PaddingGranularity (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(8, value);
			}

			inline uint32 Flags () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(12));
			}
			inline void Flags (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(12, value);
			}

			inline uint32 TotalFrames () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(16));
			}
			inline void TotalFrames (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(16, value);
			}

			inline uint32 InitialFrames () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(20));
			}
			inline void InitialFrames (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(20, value);
			}

			inline uint8 StreamCount () const
			{
				return (m_Buffer.IsValid() ? static_cast<uint8>(m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(24)) : 0);
			}
			inline void StreamCount (const uint8 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(24, value);
			}

			inline uint32 SuggestedBufferSize () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(28));
			}
			inline void SuggestedBufferSize (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(28, value);
			}

			inline uint32 Width () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(32));
			}
			inline void Width (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(32, value);
			}

			inline uint32 Heigth () const
			{
				return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(36));
			}
			inline void Heigth (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(36, value);
			}

			// Return the duration of the calculated length, given in MilliSeconds.
			inline uint64 Duration () const
			{
				return (TotalFrames() * MicroSecondsPerFrame ()) / 1000;
			}

			StreamIterator Streams ()
			{
				return (StreamIterator(m_HeaderIterator, m_Storage));
			}

			const StreamIterator Streams () const
			{
				return (StreamIterator(m_HeaderIterator, m_Storage));
			}

			RIFF::LISTHeader::Iterator OpenDML ()
			{
				return (m_ODMLList);
			}
			const RIFF::LISTHeader::Iterator OpenDML () const
			{
				return (m_ODMLList);
			}
			
			
		private:
			RIFF::ChunkHeader			m_Buffer;
			RIFF::LISTHeader::Iterator	m_ODMLList;
			RIFF::LISTHeader::Iterator	m_HeaderIterator;
			Generics::DataElement		m_Storage;
	};

} } // namespace Solutions::AVI


#endif // __AVIMEDIAHEADER_H
