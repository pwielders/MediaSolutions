#ifndef __DATAELEMENT_H
#define __DATAELEMENT_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "DataStorage.h"

namespace Solutions { namespace Generics
{
	// ---- Referenced classes and types ----
	class DataElementContainer;

	// ---- Helper types and constants ----
	 
	// ---- Helper functions ----
	typedef enum NumberEndian
	{
		ENDIAN_LITTLE = 0,
		ENDIAN_MIDDLE = 1,
		ENDIAN_BIG    = 2
	};

	#ifdef LITTLE_ENDIAN_PLATFORM
	#define ENDIAN_PLATFORM			ENDIAN_LITTLE
	#else
	#define ENDIAN_PLATFORM			ENDIAN_BIG
	#endif

	// ---- Class Definition ----
	// The datapackage is the abstract of a package that needs to be send over the line.
	class EXTERNAL DataElement
	{
		protected:
			void UpdateCache (const uint64 offset, const uint64 size, uint8* buffer)
			{
				// Update the cache...
				m_Offset  = offset;
				m_Size    = size;
				m_MaxSize = size + m_Offset;
				m_Buffer  = &(buffer[static_cast<uint32>(m_Offset)]);
			}

		public:
			inline DataElement () : m_Storage(NULL), m_MaxSize(0), m_Size(0), m_Offset(0), m_Buffer(NULL)
			{
			}

			inline DataElement(const uint64 size, uint8* buffer) :
					m_Size(size), m_Buffer(buffer), m_Storage(NULL), m_MaxSize(size), m_Offset(0)
			{
			}
			inline DataElement(ProxyType<IBuffer> buffer) :
					m_Size(buffer->Size()), m_Buffer(buffer->Buffer()), m_Storage(buffer), m_MaxSize(buffer->Size()), m_Offset(0)
			{
				ASSERT (buffer != NULL);
			}
			inline DataElement(ProxyType<IBuffer> buffer, const uint64 offset, const uint64 size = 0) :
				m_Size(buffer->Size()-offset), m_Buffer(&(buffer->Buffer())[offset]), m_Storage(buffer), m_MaxSize(buffer->Size()), m_Offset(offset)
			{
				ASSERT (buffer != NULL);

				// We only a loow a smaller size...
				if (size != 0)
				{
					ASSERT (m_Size >= size);

					m_Size = size;
				}
			}
			inline DataElement(const DataElement& RHS) :
				m_Size(RHS.m_Size), m_Buffer(RHS.m_Buffer), m_Storage (RHS.m_Storage), m_MaxSize(RHS.m_MaxSize), m_Offset(RHS.m_Offset)
			{
			}
			inline DataElement(const DataElement& RHS, const uint64 offset, const uint64 size = 0) :
				m_Size(RHS.m_Size - offset), m_Buffer(&(RHS.m_Buffer[offset])), m_Storage(RHS.m_Storage), m_MaxSize(RHS.m_MaxSize), m_Offset(RHS.m_Offset+offset)
			{
				if (size != 0)
				{
					m_Size = size;
				}

				ASSERT (offset + size <= RHS.m_Size);
			}
			~DataElement ()
			{
			}

			inline DataElement& operator= (const DataElement& RHS)
			{
				m_Size = RHS.m_Size;
				m_Offset = RHS.m_Offset;
				m_Buffer = RHS.m_Buffer;
				m_Storage = RHS.m_Storage;
				m_MaxSize = RHS.m_MaxSize;	

				return (*this);
			}


		public:
			inline bool IsValid () const
			{
				return (m_Buffer != NULL);
			}
			inline bool operator== (const ProxyType<IBuffer>& RHS) const
			{
				return (m_Storage == RHS);
			}  
			inline bool operator!= (const ProxyType<IBuffer>& RHS) const
			{
				return (m_Storage != RHS);
			}  
			inline operator ProxyType<IBuffer>&  ()
			{
				return (m_Storage);
			}  
			inline uint64 Size () const
			{
				return (m_Size);
			}

			inline const uint8* Buffer () const
			{
				return (m_Buffer);
			}

			inline uint8* Buffer ()
			{
				return (m_Buffer);
			}

			inline uint8& operator[] (const uint32 index)
			{
				return (m_Buffer[index]);
			}

			inline const uint8& operator[] (const uint32 index) const
			{
				return (m_Buffer[index]);
			}

			void Set (const uint8 value, const uint64 offset = 0, const uint64 size = ~0)
			{
				ASSERT ((size == ~0) || ((offset + size) < m_Size));
				if (size == ~0)
				{
					::memset(&m_Buffer[offset], value, static_cast<size_t>(m_Size-offset));
				}
				else
				{
					::memset(&m_Buffer[offset], value, static_cast<size_t>(size));
				}
			}

			bool Expand (const uint64 offset, const uint32 size)
			{
				bool expanded = false;

				// Make sure we are not shrinking beyond the size boundary
				ASSERT (offset <= m_Size);

				if (Size(size) == true)
				{
					// Shift all data back the beginning in..
					::memrcpy (&m_Buffer[static_cast<size_t>(offset)], &m_Buffer[static_cast<size_t>(offset)+size], static_cast<size_t>(m_Size-offset));

					// Now the total size is smaller, adjust
					m_Size += size;
				}

				return (expanded);
			}

			bool Shrink (const uint64 offset, const uint32 size)
			{
				// Make sure we are not shrinking beyond the size boundary
				ASSERT (m_Size >= (offset + size));

				// Now the toal size is smaller, adjust
				m_Size -= size;

				// Shift all data back the beginning in..
				::memcpy (&m_Buffer[static_cast<size_t>(offset)], &m_Buffer[static_cast<size_t>(offset)+size], static_cast<size_t>(m_Size-offset));

				return (true);
			}

			uint64 Copy (const DataElement& RHS, const uint64 offset = 0)
			{
				uint64 copiedSize = RHS.m_Size;
 
				// see if we need to resize
				if ((RHS.Size() + offset) < m_Size)
				{
					if (Size (offset + RHS.Size()) == true)
					{
						::memcpy(&(m_Buffer[offset]), RHS.m_Buffer, static_cast<size_t>(RHS.m_Size));
					}
					else
					{
						copiedSize = 0;
					}
				}
				else
				{
					::memcpy(&(m_Buffer[offset]), RHS.m_Buffer, static_cast<size_t>(RHS.m_Size));
				}

				return (copiedSize);
			}


			uint64 Search (const uint64 offset, const uint8 pattern[], const uint32 size) const
			{
				bool found = false;
				uint64 index = offset;

				// Only on package level allowed to pass the boundaries!!!
				ASSERT (offset < m_Size);

				// If the search object is larger than the buffer, we will not find it ;-)
				while ( (found == false) && ((index + size) < m_Size) )
				{
					uint8* entry = &m_Buffer[index];

					// 90% of the time we might start on the right character, check it befor setting up a whole call
					if (*entry != pattern[0])
					{
						entry = static_cast<uint8*> (::memchr (&m_Buffer[index], pattern[0], static_cast<uint32>(m_Size - index)));
					}
		
					// If we can not find the first character, we will not find anything at all..
					if (entry == NULL)
					{
						index = m_Size;
					}
					else 
					{
						index += (entry - &(m_Buffer[index]));
						found  = (::memcmp(&(entry[1]), &(pattern[1]), size - 1) == 0);

						if (found == false)
						{
							index++;
						}
					}
				}

				return (found == true ? index : m_Size);
			}

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			uint64 SearchNumber (const uint64 offset, const TYPENAME info) const
			{
				uint8			pattern[sizeof(TYPENAME)];
				const uint8*	numberAccess = reinterpret_cast<const uint8*>(&info);

	#ifdef LITTLE_ENDIAN_PLATFORM
				if (ENDIAN == Generics::ENDIAN_BIG)
	#else
				if (ENDIAN == Generics::ENDIAN_LITTLE)
	#endif
				{
					// The first one is independent of 
					pattern[sizeof(TYPENAME)-1] = *numberAccess++;

					if (sizeof(TYPENAME) > 1)
					{
						pattern[sizeof(TYPENAME)-2] = *numberAccess++;

						if (sizeof(TYPENAME) > 2)
						{
							pattern[sizeof(TYPENAME)-3] = *numberAccess++;
							pattern[sizeof(TYPENAME)-4] = *numberAccess++;

							if (sizeof(TYPENAME) > 4)
							{
								pattern[sizeof(TYPENAME)-5] = *numberAccess++;
								pattern[sizeof(TYPENAME)-6] = *numberAccess++;
								pattern[sizeof(TYPENAME)-7] = *numberAccess++;
								pattern[sizeof(TYPENAME)-8] = *numberAccess++;

							}
						}

						numberAccess = &pattern[0];
					}
				}

				return (Search (offset, numberAccess, sizeof(TYPENAME)));
			}

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			inline TYPENAME GetNumber(const uint64 offset) const
			{
				// Only on package level allowed to pass the boundaries!!!
				ASSERT ((offset + sizeof(TYPENAME)) <= m_Size);

				TYPENAME result;

				if (sizeof(TYPENAME) == 1)
				{
					result = static_cast <const TYPENAME>(m_Buffer[offset]);
				}
	#ifdef LITTLE_ENDIAN_PLATFORM
				else if (ENDIAN == Generics::ENDIAN_BIG)
	#else
				else if (ENDIAN == Generics::ENDIAN_LITTLE)
	#endif
				{
					const uint8* source      = &m_Buffer[offset];
					uint8*       destination = &(reinterpret_cast<uint8*>(&result)[sizeof(TYPENAME)-1]);

					*destination-- = *source++;
					*destination-- = *source++;

					if ( (sizeof(TYPENAME) == 4) || (sizeof(TYPENAME) == 8))
					{
						*destination-- = *source++;
						*destination-- = *source++;

						if (sizeof(TYPENAME) == 8)
						{
							*destination-- = *source++;
							*destination-- = *source++;
							*destination-- = *source++;
							*destination-- = *source++;
						}
					}
				}

	#ifdef LITTLE_ENDIAN_PLATFORM
				else if (ENDIAN == Generics::ENDIAN_LITTLE)
	#else
				else if (ENDIAN == Generics::ENDIAN_BIG)
	#endif
				{
					const uint8* source      = &m_Buffer[offset];
					uint8*       destination = reinterpret_cast<uint8*>(&result);

					*destination++ = *source++;
					*destination++ = *source++;

					if ( (sizeof(TYPENAME) == 4) || (sizeof(TYPENAME) == 8))
					{
						*destination++ = *source++;
						*destination++ = *source++;

						if (sizeof(TYPENAME) == 8)
						{
							*destination++ = *source++;
							*destination++ = *source++;
							*destination++ = *source++;
							*destination++ = *source++;
						}
					}
				}
				return (result);
			}

			template <typename TYPENAME>
			TYPENAME GetBitNumber(const uint64 offset, const unsigned int bitIndex, const unsigned char count) const
			{
				// Bit numbers are not allowed over package boundaries. Make sure it fits in
				// this element.
				ASSERT ((offset + ((bitIndex + count) / 8)) <= m_Size);

				// Make sure that no more bits are requetsed than fit...
				ASSERT (count <= (sizeof (TYPENAME) << 3));

				// Head of the bytestream fetch
				uint8*   buffer   = m_Buffer[offset + (bitIndex / 8)];
				TYPENAME result   = (*buffer & (0xFF >> (bitIndex % 8)));
				int      bitsleft = count - (8 - (bitIndex % 8));

				if (bitsleft >= 0)
				{
					// Middle of the bytestream fetch.. All bytes as as a full byte shift.
					while (bitsleft > 8)
					{
						bitsleft -= 8;
						buffer++;
						result = (result << 8) + (*buffer & 0xFF);
					}

					// Tail of the bytestream fetch.. Time to add the last part, if needed;
					if (bitsleft > 0)
					{
						buffer++;
						result = (result << bitsleft) | ((*buffer >> (8 - bitsleft)) & 0xFF);
					}
				}
				else
				{
					// Oops all information was contained in the first byte. 
					result = result >> (8 - (bitIndex % 8) - count);
				}

				return result;
			}

			inline void GetBuffer (const uint64 offset, const uint32 size, uint8* buffer) const
			{
				// Check if we cross a boundary for the read..
				ASSERT ((offset + size) <= m_Size);

				// Nope, one plain copy !!!
				::memcpy (buffer, &m_Buffer[offset], size);
			}

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			void SetNumber(const uint64 offset, const TYPENAME number)
			{
				// Only on package level allowed to pass the boundaries!!!
				ASSERT ((offset + sizeof(TYPENAME)) <= m_Size);

				if (sizeof(TYPENAME) == 1)
				{
					m_Buffer[offset] = static_cast <const uint8> (number);
				}
	#ifdef LITTLE_ENDIAN_PLATFORM
				else if (ENDIAN == Generics::ENDIAN_BIG)
	#else
				else if (ENDIAN == Generics::ENDIAN_LITTLE)
	#endif
				{
					const uint8* source      = reinterpret_cast<const uint8*>(&number);
					uint8*       destination = &m_Buffer[offset+sizeof(TYPENAME)-1];

					*destination-- = *source++;
					*destination-- = *source++;

					if ( (sizeof(TYPENAME) == 4) || (sizeof(TYPENAME) == 8))
					{
						*destination-- = *source++;
						*destination-- = *source++;

						if (sizeof(TYPENAME) == 8)
						{
							*destination-- = *source++;
							*destination-- = *source++;
							*destination-- = *source++;
							*destination-- = *source++;
						}
					}
				}

	#ifdef LITTLE_ENDIAN_PLATFORM
				if (ENDIAN == Generics::ENDIAN_LITTLE)
	#else
				if (ENDIAN == Generics::ENDIAN_BIG)
	#endif
				{
					const uint8* source      = reinterpret_cast<const uint8*>(&number);
					uint8*       destination = &m_Buffer[offset];

					*destination++ = *source++;
					*destination++ = *source++;

					if ( (sizeof(TYPENAME) == 4) || (sizeof(TYPENAME) == 8))
					{
						*destination++ = *source++;
						*destination++ = *source++;

						if (sizeof(TYPENAME) == 8)
						{
							*destination++ = *source++;
							*destination++ = *source++;
							*destination++ = *source++;
							*destination++ = *source++;
						}
					}
				}
			}

			template <typename TYPENAME>
			void SetBitNumber(const uint64 offset, const unsigned int bitIndex, const unsigned char count, const TYPENAME value)
			{
				// Bit numbers are not allowed over package boundaries. Make sure it fits in
				// this element.
				ASSERT ((offset + ((bitIndex + count) / 8)) <= m_Size);

				// Make sure that no more bits are requetsed than fit...
				ASSERT (count <= (sizeof (TYPENAME) << 3));

				// Remove all bits that do not matter	
				TYPENAME       Number = (value & ~((static_cast<TYPENAME>(~0)) << count));
				uint8*		   Buffer = &m_Buffer[offset + (bitIndex/8)];
				unsigned int   Rest   = 8 - (bitIndex % 8);

				// Maybe it fits within the current byte (keep before and after).
				if (count < Rest)
				{
					Buffer[0] = static_cast<unsigned char> (
								/* BEFORE */ (Buffer[0] &  (0xFF << Rest)) |
                				/* NUMBER */ (Number    << (Rest - count)) |
								/* AFTER  */ (Buffer[0] &  (0xFF >> (8-Rest+count)))
								);
				}
				else
				{
					unsigned int BitCount = count - Rest;

					// Start with the BEFORE...
					Buffer[0] = static_cast<unsigned char> (
								/* BEFORE */ (Buffer[0] & (0xFF << Rest)) | 
								/* NUMBER */ (Number >> (count - Rest))
								);

					// Fill up all complete bytes
					while (BitCount >= 8)
					{
						// Move to the correct locations
						Buffer++;
						BitCount -= 8;

						// Fill the bytes
						Buffer[0] = static_cast<unsigned char> ((Number >> (count - BitCount)) & 0xFF);
					}

					// See if we need to fill the tail...
					if (BitCount != 0)
					{
						Buffer[1]= static_cast<unsigned char> (
								   /* NUMBER */ ((Number << (8 - BitCount)) & 0xFF) |
								   /* AFTER  */ (Buffer[1] & (0xFF >> BitCount))
								   );
					}
				}
			}

			void SetBuffer (const uint64 offset, const uint32 size, const uint8* buffer)
			{
				// Check if we cross a boundary for the write..
				ASSERT ((offset + size) <= m_Size);

				// Nope, one plain copy !!!
				::memcpy (&m_Buffer[offset], buffer, size);
			}

			bool Size (const uint64 size)
			{
				if (size == NUMBER_MAX_UNSIGNED(uint64))
				{
					// Reset the size to the maxSize...
					m_Size = m_MaxSize - m_Offset;
				}
				else if ((size + m_Offset) < m_MaxSize)
				{
					// It fits the allocated buffer, accept and reduce..
					m_Size = size;
				}
				else if (m_Storage != NULL)
				{
					// We need to "extend" the buffer, this is only possible if we control
					// the buffer lifetime..
					// Create a new buffer					
					m_Storage = m_Storage->Clone(m_Offset + size);
					m_Buffer = &(m_Storage->Buffer()[static_cast<uint32>(m_Offset)]);
					m_Size = size;
					m_MaxSize = m_Storage->Size();
				}

				return (size == m_Size);
			}

			uint64 Copy (const uint64 offset, const DataElementContainer& copy);
			uint32 CRC32(const uint64 offset, const uint64 size) const;

		private:
			ProxyType<IBuffer>	m_Storage;

			// Cached values
			uint8*				m_Buffer;
			uint64				m_Offset;
			uint64				m_Size;
			uint64				m_MaxSize;
	};

	class EXTERNAL LinkedDataElement : public DataElement
	{
		public:
			inline LinkedDataElement () :
				 DataElement(), m_Next(NULL)
			{
			}
			explicit inline LinkedDataElement(const DataElement& RHS, const uint64 offset = 0, const uint64 size = 0) :
				DataElement (RHS, offset, size), m_Next (NULL)
			{
			}
			inline LinkedDataElement(const LinkedDataElement& RHS, const uint64 offset = 0, const uint64 size = 0) :
				DataElement(RHS, offset, (offset + size > RHS.Size() ? 0 : size)), m_Next(RHS.m_Next)
			{
				// Don't set the size bigger than the cummulated one!!!
				ASSERT (offset + size < RHS.LinkedSize());
			}
			inline LinkedDataElement (const uint64 Size, uint8* Buffer, LinkedDataElement* Enclosure) :
				DataElement (Size, Buffer), m_Next(Enclosure)
			{
			}
			inline ~LinkedDataElement()
			{
			}

			inline LinkedDataElement& operator= (const LinkedDataElement& RHS)
			{
				DataElement::operator= (RHS);
				m_Next = RHS.m_Next;

				return (*this);
			}

			inline LinkedDataElement& operator= (const DataElement& RHS)
			{
				DataElement::operator= (RHS);

				return (*this);
			}

		public:
			inline LinkedDataElement* Enclosed ()
			{
				return (m_Next);
			}

			inline void Enclosed (LinkedDataElement* next)
			{
				m_Next = next;
			}

			inline const LinkedDataElement* Enclosed () const
			{
				return (m_Next);
			}

			inline uint64 LinkedSize () const
			{
				return (Size() + (m_Next != NULL ? m_Next->LinkedSize() : 0));
			}

			inline uint32 LinkedElements () const
			{
				return (1 + (m_Next != NULL ? m_Next->LinkedElements() : 0));
			}

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			uint64 SearchNumber (const uint64 offset, TYPENAME pattern) const
			{
				uint64 result = DataElement::SearchNumber<TYPENAME, ENDIAN> (offset, pattern);

				if ((result == NUMBER_MAX_UNSIGNED(uint64)) && (m_Next != NULL))
				{
					bool found = false;
					result = m_Size - sizeof(TYPENAME) + 1;

					while ((found == false ) && (offset != m_Size))
					{
						// Check if the N-1 characters are in the next buffer.
						found = (GetNumber<TYPENAME>(offset++) == pattern);
					}
				}

				return ((result == m_Size) ? NUMBER_MAX_UNSIGNED(uint64) : result);
			}

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			inline TYPENAME GetNumber(const uint64 offset) const
			{
				// Only on package level allowed to pass the boundaries!!!
				ASSERT (offset + sizeof(TYPENAME) < m_Size);

				TYPENAME result;

				if (sizeof(TYPENAME) == 1)
				{
					result = static_cast<TYPENAME> (m_Buffer[offset]);
				}
				else
				{
					// Use the SetBuffer of this class, that can go over boundaries...
					LinkedDataElement::GetBuffer (offset, sizeof(TYPENAME), static_cast<uint8*>(&result));
				}

				return (result);
			}
			void GetBuffer (const uint64 offset, const uint32 size, uint8* buffer) const;

			template <typename TYPENAME, const enum NumberEndian ENDIAN>
			inline void SetNumber(const uint64 offset, const TYPENAME number)
			{
				// Only on package level allowed to pass the boundaries!!!
				ASSERT (offset + sizeof(TYPENAME) < m_Size);

				if (sizeof(TYPENAME) == 1)
				{
					m_Buffer[offset] = static_cast<const uint8*>(&number);
				}
				else
				{
					// Use the SetBuffer of this class, that can go over boundaries...
					LinkedDataElement::SetBuffer (offset, sizeof(TYPENAME), static_cast<const uint8*>(&number));
				}
			}
			void SetBuffer (const uint64 offset, const uint32 size, const uint8* buffer);

			uint64 Copy (const uint64 offset, const LinkedDataElement& copy);

		private:
			LinkedDataElement*	m_Next;
	};

	// Use this class if you want to "read" and forward on access or write and forward on access..
	class DataElementParser 
	{
		private:
			DataElementParser ();
			DataElementParser(const DataElementParser&);
			DataElementParser& operator= (const DataElementParser&);

		public:
			inline DataElementParser (DataElement& element, const unsigned int offset = 0) :
				m_Buffer(element), m_Offset(offset)
			{
				ASSERT (offset < m_Buffer.Size());
			}

		public:
			inline bool IsValid () const
			{
				return (m_Offset < m_Buffer.Size ());
			}

			inline uint64 Size () const
			{
				return (m_Offset - m_Buffer.Size());
			}

			inline void SkipBytes (const unsigned int bytes)
			{
				m_Offset += bytes;
			}

			template <typename TYPENAME>
			bool SearchNumber (TYPENAME number) const
			{
				// As long as there are packages and we did not find the sequence, continue..
				m_Offset = m_Buffer->SearchNumber(m_Offset, number);

				return (m_Offset < m_Buffer.Size);
			}

			template <typename TYPENAME>
			inline TYPENAME GetNumber() const
			{
				// Only on element level allowed to extract !!
				ASSERT ((m_Offset + sizeof(TYPENAME)) < m_Buffer.Size());

				TYPENAME result =  m_Buffer.GetBytes<TYPENAME>(m_Offset);

				m_Offset += sizeof(TYPENAME);

				return (result);
			}

			void GetBuffer (const unsigned int size, uint8* buffer) const
			{
				// Only on element level allowed to extract !!
				ASSERT ((m_Offset + size) < m_Buffer.Size());

				m_Buffer.GetBuffer(m_Offset, size, buffer);

				m_Offset += size;
			}

			template <typename TYPENAME>
			inline void SetNumber(const TYPENAME number)
			{
				// Only on element level allowed to set !!
				ASSERT ((m_Offset + sizeof(TYPENAME)) < m_Buffer.Size());

				m_Buffer.SetBytes<TYPENAME>(m_Offset, number);
				m_Offset += sizeof(TYPENAME);
			}

			void SetBuffer (const unsigned int size, const uint8* buffer)
			{
				// Only on element level allowed to set !!
				ASSERT ((m_Offset + size) < m_Buffer.Size());

				m_Buffer.SetBuffer(m_Offset, size, buffer);

				m_Offset += size;
			}

		private:
			mutable uint64	m_Offset;
			DataElement&	m_Buffer;
	};

} } // namespace Solutions::Generics

#endif // __DATAELEMENT_H
