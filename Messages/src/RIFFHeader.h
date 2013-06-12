#ifndef __RIFFHEADER_H
#define __RIFFHEADER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
#define FOURCCCODE(a,b,c,d)									\
		public: static Solutions::RIFF::FourCC& FCC()		\
		{													\
			static Solutions::RIFF::FourCC code (a,b,c,d);	\
															\
			return (code);									\
		}

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace RIFF
{
	class EXTERNAL FourCC
	{
		private:
			FourCC();

		public:
			FourCC(const uint8 a, const uint8 b, const uint8 c, const uint8 d);
			FourCC(const TCHAR code[]);
			FourCC(const uint32 code);

			FourCC& operator= (const FourCC& RHS);

		public:
			inline static const uint8 Size ()
			{
				return (4);
			}
			inline const uint8* Code () const
			{
				return (reinterpret_cast<const uint8*> (&m_Code));
			}
			inline uint32 Value () const
			{
				return (m_Code);
			}
			inline bool operator== (const FourCC& RHS) const
			{
				return (m_Code == RHS.m_Code);
			}
			inline bool operator== (const uint32 code) const
			{
				return (m_Code == ::ntohl(code));
			}
			inline bool operator!= (const FourCC& RHS) const
			{
				return (m_Code != RHS.m_Code);
			}
			inline bool operator!= (const uint32 code) const
			{
				return (m_Code != ::ntohl(code));
			}

		private:
			uint32		m_Code;	
	};

	class EXTERNAL Header
	{
		public:
			Header() :
				m_Buffer()
			{
			}

			Header(const Generics::DataElement& buffer) :
				m_Buffer (buffer)
			{
			}

			Header(const Header& copy) :
				m_Buffer (copy.m_Buffer)
			{
			}

			~Header()
			{
			}

			Header operator= (const Header& RHS)
			{
				m_Buffer = RHS.m_Buffer;
				
				return(*this);
			}

		public:
			bool Clear ()
			{
				bool cleared = false;

				if (m_Buffer.Size() >= (FourCC::Size() + 4))
				{
					cleared = true;

					// Clear everything we own.
					m_Buffer.Set(0, 0, FourCC::Size() + 4);
				}

				return (cleared);
			}

			inline FourCC ID() const
			{
				return (FourCC(m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(0)));
			}
			inline void ID(const FourCC& value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, value.Value());
			}
			inline bool IsValid () const
			{
				return (m_Buffer.Size() >= (FourCC::Size() + 4));
			}
			inline static uint32 Size (const Generics::DataElement& buffer)
			{
				uint32 result = 0;

				if (buffer.Size() >= (FourCC::Size() + 4))
				{
					// According to the spec, only the FileType field is included in 
					result = buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(FourCC::Size());

					if (buffer.Size() < (result + FourCC::Size() + 4))
					{
						result = 0;
					}
				}
				return (result);
			}
			inline uint32 Space () const
			{
				return (static_cast<uint32>(m_Buffer.Size()) - (FourCC::Size() + 4));
			}
			inline uint32 Allocated () const
			{
				return (Size(m_Buffer) + FourCC::Size() + 4 + 1 /* For word alignment */) & 0xFFFFFFFE;
			}
			inline uint32 Size () const
			{
				// According to the spec, only the FileType field is included in 
				return (Size(m_Buffer));
			}
			inline void Size (const uint32 value)
			{
				m_Buffer.SetNumber<uint32, Generics::ENDIAN_LITTLE>(FourCC::Size(), value);
			}
			inline Generics::DataElement Data (const uint64 offset = 0, const uint64 size = 0) const
			{
				if (size == 0)
				{
					return (Generics::DataElement(m_Buffer, FourCC::Size() + 4 + offset, m_Buffer.GetNumber<uint32, Generics::ENDIAN_LITTLE>(FourCC::Size()) - offset));
				}

				return (Generics::DataElement(m_Buffer, FourCC::Size() + 4 + offset, size));
			}

			template <typename TYPENAME, const enum Generics::NumberEndian ENDIAN>
			inline TYPENAME GetNumber(const uint64 offset) const
			{
				return (m_Buffer.GetNumber<TYPENAME, ENDIAN>(FourCC::Size() + 4 + offset));
			}

			template <typename TYPENAME, const enum Generics::NumberEndian ENDIAN>
			inline void SetNumber(const uint64 offset, const TYPENAME number)
			{
				m_Buffer.SetNumber<TYPENAME, ENDIAN>(FourCC::Size() + 4 + offset, number);
			}

		private:
			Generics::DataElement		m_Buffer;
	};

	class EXTERNAL RIFFHeader : public Header
	{
		FOURCCCODE('R', 'I', 'F', 'F');

		public:
			RIFFHeader() :
				Header()
			{
			}

			RIFFHeader(const Generics::DataElement& buffer) :
				Header (buffer)
			{
			}

			RIFFHeader(const RIFFHeader& copy) :
				Header (copy)
			{
			}

			~RIFFHeader()
			{
			}

			RIFFHeader operator= (const RIFFHeader& RHS)
			{
				Header::operator= (RHS);
				
				return(*this);
			}

		public:
			inline bool IsValid () const
			{
				return ((Header::Size() > FourCC::Size()) && (RIFFHeader::FCC() == Header::ID())); 
			}
			inline FourCC FileType () const
			{
				return (FourCC(Header::GetNumber<uint32, Generics::ENDIAN_LITTLE>(0)));
			}

			inline void FileType (const FourCC& value)
			{
				SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, value.Value());
			}

			inline Generics::DataElement Data () const
			{
				return (Header::Data(FourCC::Size()));
			}
	};

	class EXTERNAL ChunkHeader : public Header
	{
		public:
			ChunkHeader() :
				Header()
			{
			}
			ChunkHeader(const Generics::DataElement& buffer) :
				Header (buffer)
			{
			}
			ChunkHeader(const Header& header) :
				Header (header)
			{
			}
			ChunkHeader(const ChunkHeader& copy) :
				Header (copy)
			{
			}
			~ChunkHeader()
			{
			}

			ChunkHeader& operator= (const ChunkHeader& RHS)
			{
				Header::operator= (RHS);

				return (*this);
			}
	};

	class EXTERNAL LISTHeader : public Header
	{
		FOURCCCODE('L', 'I', 'S', 'T');

		public:
			class Iterator
			{
				public:
					Iterator () :
						m_Data(),
						m_Current(),
						m_Offset(NUMBER_MAX_UNSIGNED(uint64))
					{
					}
					Iterator (const Generics::DataElement& buffer) :
						m_Data(buffer),
						m_Current(),
						m_Offset(NUMBER_MAX_UNSIGNED(uint64))
					{
					}
					Iterator (const Iterator& copy) :
						m_Data(copy.m_Data),
						m_Current(copy.m_Current),
						m_Offset(copy.m_Offset)
					{
					}
					~Iterator()
					{
					}

					Iterator& operator= (const Iterator& RHS)
					{
						m_Data = RHS.m_Data;
						m_Current = RHS.m_Current;
						m_Offset = RHS.m_Offset;

						return (*this);
					}

				public:
					void Reset()
					{
						m_Offset = NUMBER_MAX_UNSIGNED(uint64);
					}

					bool IsValid () const
					{
						return (m_Current.IsValid());
					}

					bool Next ()
					{
						if (m_Offset == NUMBER_MAX_UNSIGNED(uint64))
						{
							m_Offset = 0;
							m_Current = Header (Generics::DataElement(m_Data, m_Offset));
						}
						else if (m_Offset  < m_Data.Size())
						{
							ASSERT (m_Current.IsValid ());

							m_Offset += m_Current.Allocated();
							m_Current = Header (Generics::DataElement(m_Data, m_Offset));
						}

						return (IsValid ());
					}

					const FourCC ID () const
					{
						FourCC result (m_Data.GetNumber<uint32, Generics::ENDIAN_LITTLE>(m_Offset));

						if (result == LISTHeader::FCC())
						{
							// This is a list, get the type of list as the ID..
							result = LISTHeader(m_Current).ListType();
						}

						return (result);
					}

					bool IsChunk () const
					{
						return ((m_Current.IsValid()) && (LISTHeader::FCC() != m_Current.ID()));
					}

					bool IsList () const
					{
						return ((m_Current.IsValid()) && (LISTHeader::FCC() == m_Current.ID()));
					}

					LISTHeader List()
					{
						return (LISTHeader(m_Current));
					}

					const LISTHeader List() const
					{
						return (LISTHeader(m_Current));
					}

					ChunkHeader Chunk ()
					{
						return (ChunkHeader(m_Current));
					}

					const ChunkHeader Chunk () const
					{
						return (ChunkHeader(m_Current));
					}

				private:
					Generics::DataElement	m_Data;
					Header					m_Current;
					uint64					m_Offset;
			};

		public:
			LISTHeader() :
				Header()
			{
			}

			LISTHeader(const Generics::DataElement& buffer) :
				Header (buffer)
			{
			}
			LISTHeader(const Header& header) :
				Header (header)
			{
			}
			LISTHeader(const LISTHeader& copy) :
				Header (copy)
			{
			}
			~LISTHeader()
			{
			}

			LISTHeader& operator= (const LISTHeader& RHS)
			{
				Header::operator =(RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return ((Header::Size() >= FourCC::Size()) && (FCC() == Header::ID())); 
			}

			inline FourCC ListType () const
			{
				return (FourCC(Header::GetNumber<uint32, Generics::ENDIAN_LITTLE>(0)));
			}

			inline void ListType (const FourCC& value)
			{
				Header::SetNumber<uint32, Generics::ENDIAN_LITTLE>(0, value.Value());
			}

			inline Iterator Elements ()
			{
				return (Iterator(Header::Data(FourCC::Size())));
			}

			inline const Iterator Elements () const
			{
				return (Iterator(Header::Data(FourCC::Size())));
			}
	};
} } // namespace Solutions::RIFF

#endif // __RIFFHEADER_H
