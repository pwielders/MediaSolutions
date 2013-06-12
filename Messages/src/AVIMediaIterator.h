#ifndef __AVIMEDIAITERATOR_H
#define __AVIMEDIAITERATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "RIFFHeader.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
/* bIndexType codes */
#define AVI_INDEX_OF_INDEXES 0x00000000		// when each entry in aIndex
											// array points to an index chunk
#define AVI_INDEX_OF_CHUNKS	 0x00000001		// when each entry in aIndex
											// array points to a chunk in the file
#define AVI_INDEX_IS_DATA	 0x00000080		// when each entry is aIndex is
											// really the data
											// bIndexSubtype codes for INDEX_OF_CHUNKS
#define AVI_INDEX_2FIELD	 0x00000001		// when fields within frames
											// are also indexed

/* Flags for AVI_INDEX_ENTRY */
#define AVIIF_LIST           0x00000001
#define AVIIF_TWOCC          0x00000002
#define AVIIF_NOTIME         0x00000100		// this chunk doesn't affect timing ie palette change
#define AVIIF_COMPUSE        0x0FFF0000
        
#define AVIIF_KEYFRAME       0x00000010		// keyframe doesn't need previous info to be decompressed
#define AVIIF_FIRSTPART      0x00000020		// this chunk needs the frames following it to be used
#define AVIIF_LASTPART       0x00000040		// this chunk needs the frames before it to be used
#define AVIIF_MIDPART        (AVIIF_LASTPART|AVIIF_FIRSTPART)

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace AVI 
{
	typedef enum StreamType
	{
		AUDIO,
		VIDEO,
		TEXT,
		UNKNOWN
	};

	class EXTERNAL DataChunk : public RIFF::ChunkHeader
	{
		private:
			static const uint16 PrefixVideo = ('d' << 8) + 'c';
			static const uint16 PrefixAudio = ('w' << 8) + 'b';
			static const uint16 PrefixText  = ('t' << 8) + 'x';

		public:
			DataChunk() :
				RIFF::ChunkHeader()
			{
			}
			DataChunk(const RIFF::ChunkHeader& buffer) :
				RIFF::ChunkHeader(buffer)
			{
			}
			DataChunk(const DataChunk& copy) :
				RIFF::ChunkHeader(copy)
			{
			}
			~DataChunk()
			{
			}

			DataChunk& operator= (const DataChunk& RHS)
			{
				RIFF::ChunkHeader::operator = (RHS);

				return (*this);
			}

		public:
			inline static RIFF::FourCC Code (const StreamType streamType, const uint8 channel)
			{
				switch (streamType)
				{
				case AUDIO:
					 return (RIFF::FourCC( ('0' + ((channel / 10) % 10)),
						                   ('0' +  (channel % 10)      ),
										   ('w'), ('b') ) );
					 break;
				case VIDEO:
					 return (RIFF::FourCC( ('0' + ((channel / 10) % 10)),
						                   ('0' +  (channel % 10)      ),
										   ('d'), ('c') ) );
					 break;
				case TEXT:
					 return (RIFF::FourCC( ('0' + ((channel / 10) % 10)),
						                   ('0' +  (channel % 10)      ),
										   ('t'), ('w') ) );
					 break;
				default:
					 ASSERT(false);
					 break;
				}

				return (static_cast<uint32>(0));
			}
			inline static bool IsVideo (const RIFF::FourCC& info)
			{
				return ((info.Value() & 0xFF) == PrefixVideo);
			}
			inline static bool IsAudio (const RIFF::FourCC& info)
			{
				return ((info.Value() & 0xFF) == PrefixAudio);
			}
			inline static bool IsText (const RIFF::FourCC& info)
			{
				return ((info.Value() & 0xFF) == PrefixText);
			}
			inline bool IsVideo () const
			{
				return (DataChunk::IsVideo(RIFF::ChunkHeader::ID()));
			}
			inline bool IsAudio () const
			{
				return (DataChunk::IsAudio(RIFF::ChunkHeader::ID()));
			}
			inline bool IsText () const
			{
				return (DataChunk::IsText(RIFF::ChunkHeader::ID()));
			}
			inline uint16 Stream () const
			{
				return ((RIFF::ChunkHeader::ID().Value() >> 16) & 0xFF);
			}
	};

	class EXTERNAL PackageIterator
	{
		private:
			class EXTERNAL LegacyIterator 
			{
				public:
					LegacyIterator ();
					LegacyIterator(const RIFF::ChunkHeader& index, const Generics::DataElement& storage, const StreamType streamType, const uint8 channel);
					LegacyIterator(const LegacyIterator& copy);
					~LegacyIterator ();

					LegacyIterator& operator= (const LegacyIterator& RHS);

				public:
					inline bool IsValid () const
					{
						return ((m_Offset != NUMBER_MAX_UNSIGNED(uint32)) && (m_Offset < m_Index.Size()));
					}
					void Reset();
					bool Next();
					uint32 Size () const;
					uint32 Package (Generics::DataElementContainer& dataFrame);

				private:
					inline uint8 IndexChunkSize () const
					{
						return (4 * 4);
					}
					inline bool IsValidChunk(const uint32 offset) const
					{
						return (RIFF::FourCC(m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(offset)) == m_Type);
					}
					inline bool IsStartChunk(const uint32 offset) const
					{
						return ( ((m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(offset+4) & AVIIF_FIRSTPART) == AVIIF_FIRSTPART) ||
								 ((m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(offset+4) & AVIIF_MIDPART)   == 0) );
					}

				private:
					RIFF::FourCC			m_Type;
					RIFF::ChunkHeader		m_Index;
					uint32					m_Offset;
					Generics::DataElement	m_Storage;

			};

			class EXTERNAL Iterator
			{
				public:
					Iterator () :
						m_Buffer(),
						m_Index(NUMBER_MAX_UNSIGNED(uint32)),
						m_MaxCount(0),
						m_ElementSize(0)
					{
					}
					Iterator(const Generics::DataElement& buffer, const uint32 blockSize, const uint32 maxCount) :
						m_Buffer(buffer),
						m_Index(NUMBER_MAX_UNSIGNED(uint32)),
						m_MaxCount(maxCount),
						m_ElementSize(blockSize)
					{
					}
					Iterator(const Iterator& copy) :
						m_Buffer(copy.m_Buffer),
						m_Index(copy.m_Index),
						m_MaxCount(copy.m_MaxCount),
						m_ElementSize(copy.m_ElementSize)
					{
					}
					~Iterator ()
					{
					}

					Iterator& operator= (const Iterator& RHS)
					{
						m_Buffer = RHS.m_Buffer;
						m_Index = RHS.m_Index;
						m_MaxCount = RHS.m_MaxCount;
						m_ElementSize = RHS.m_ElementSize;

						return (*this);
					}

				public:
					inline bool IsValid () const
					{
						return (m_Index < m_MaxCount);
					}

					void Reset()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint32);
					}

					bool Next()
					{
						if (m_Index == NUMBER_MAX_UNSIGNED(uint32))
						{
							m_Index = 0;
						}
						else if (m_Index < m_MaxCount)
						{
							m_Index++;
						}

						return (IsValid());
					}

					uint32 Size() const
					{
						return (0);
					}

					uint32 Package(Generics::DataElementContainer& /* container */)
					{
						return (0);
					}

				protected:
					template <typename TYPENAME, const enum Generics::NumberEndian ENDIAN>
					inline TYPENAME GetNumber(const uint32 offset) const
					{
						ASSERT (IsValid());

						return (m_Buffer.GetNumber<TYPENAME, ENDIAN>((m_Index*m_ElementSize)+offset));
					}

					template <typename TYPENAME, const enum Generics::NumberEndian ENDIAN>
					inline void SetNumber(const uint32 offset, const TYPENAME number)
					{
						ASSERT (IsValid());

						m_Buffer.SetNumber<TYPENAME, ENDIAN>((m_Index*m_ElementSize)+offset, number);
					}

				private:
					Generics::DataElement		m_Buffer;
					uint32						m_Index;
					uint32						m_MaxCount;	
					uint32						m_ElementSize;	
			};

			class EXTERNAL StandardIterator : public Iterator
			{
				public:
					StandardIterator () :
						Iterator()					
					{
					}
					StandardIterator(const Generics::DataElement& buffer, const uint32 maxCount) :
						Iterator(buffer, 8, maxCount)
					{
					}
					StandardIterator(const StandardIterator& copy) :
						Iterator(copy)
					{
					}
					~StandardIterator ()
					{
					}

					StandardIterator& operator= (const StandardIterator& RHS)
					{
						Iterator::operator= (RHS);

						return (*this);
					}

				public:
					inline uint32 Offset () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(0));
					}

					inline uint32 Size () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(4) & 0x7FFFFFFF);
					}

					inline bool IsKeyFrame () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(4) & 0x8000000) != 0;
					}
			};

			class EXTERNAL FieldIterator : public Iterator
			{
				public:
					FieldIterator () :
						Iterator()
					{
					}
					FieldIterator(const Generics::DataElement& buffer, const uint32 maxCount) :
						Iterator(buffer, 12, maxCount)
					{
					}
					FieldIterator(const FieldIterator& copy) :
						Iterator(copy)
					{
					}
					~FieldIterator ()
					{
					}

					FieldIterator& operator= (const FieldIterator& RHS)
					{
						Iterator::operator= (RHS);

						return (*this);
					}

				public:
					inline uint32 FirstFieldOffset () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(0));
					}

					inline uint32 SecondFieldOffset () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
					}

					inline uint32 Size () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(4) & 0x7FFFFFFF);
					}

			};

			class EXTERNAL SuperIterator : public Iterator
			{
				public:
					SuperIterator () :
						Iterator()					
					{
					}
					SuperIterator(const Generics::DataElement& buffer, const uint32 maxCount) :
						Iterator(buffer, 16, maxCount)
					{
					}
					SuperIterator(const SuperIterator& copy) :
						Iterator(copy)
					{
					}
					~SuperIterator ()
					{
					}

					SuperIterator& operator= (const SuperIterator& RHS)
					{
						Iterator::operator= (RHS);

						return (*this);
					}

				public:
					inline uint64 Offset () const
					{
						return (Iterator::GetNumber<uint64, Generics::ENDIAN_LITTLE>(0));
					}

					inline uint32 Size () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(8));
					}

					inline uint32 Duration () const
					{
						return (Iterator::GetNumber<uint32, Generics::ENDIAN_LITTLE>(12));
					}
			};

		protected:
			static const uint32 g_HeaderSize = 24;

		public:
			PackageIterator();
			PackageIterator(const Generics::DataElement& storage, const RIFF::ChunkHeader& index, const StreamType type, const uint8 channel);
			PackageIterator(const PackageIterator& copy);
			~PackageIterator();

			PackageIterator& operator= (const PackageIterator& RHS);

		public:
			bool IsValid () const;
			void Reset();
			bool Next();
			uint32 Size() const;
			uint32 Package(Generics::DataElementContainer& container);

			// This is the time that the current appointed data should be presented,
			// relative to the time that the first data was aquired in milliseconds.
			inline uint64 PresentationTime () const
			{
				return (m_PresentationTime);
			}

			inline StreamType Type () const
			{
				return (m_StreamType);
			}

		private:
			void Initialize();

		private:
			typedef enum IteratorType
			{
				ILLEGAL,
				LEGACY,
				STANDARD,
				FIELD,
				SUPER_CHUNK_INDEX,
				SUPER_INDEX_INDEX
			};

			IteratorType			m_IteratorType;
			StreamType				m_StreamType;
			uint32					m_Channel;
			LegacyIterator			m_LegacyIndex;
			StandardIterator		m_StandardIndex;
			FieldIterator			m_FieldIndex;
			SuperIterator			m_SuperIndex;
			RIFF::ChunkHeader		m_IndexChunk;
			Generics::DataElement	m_Storage;
			uint64					m_PresentationTime;
	};

} } // namespace Solutions::AVI


#endif // __AVIMEDIAITERATOR_H
