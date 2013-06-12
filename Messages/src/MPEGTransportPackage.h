#ifndef __MPEGTRANSPORTPACKAGE_H
#define __MPEGTRANSPORTPACKAGE_H

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
	class EXTERNAL AdaptationField
	{
		private:
			AdaptationField ();

		public:
			inline AdaptationField (const Generics::DataElement& data) :
				m_Package (data)
			{
			}
			inline AdaptationField (const AdaptationField& copy) :
				m_Package (copy.m_Package)
			{
			}
			inline ~AdaptationField ()
			{
			}

			inline AdaptationField& operator= (const AdaptationField& RHS)
			{
				m_Package = RHS.m_Package;

				return (*this);
			}

		public:
			bool IsDiscontinuedState () const
			{
				return ((m_Package[0] & 0x80) == 0x80);
			}
			bool IsRandomAccess () const
			{
				return ((m_Package[0] & 0x40) == 0x40);
			}
			inline bool IsPriority() const
			{
				return ((m_Package[0] & 0x20) == 0x20);
			}
			inline Generics::OptionalType<ReferenceClock> ProgramClock () const
			{
				if (HasProgramClock())
				{
					uint64 clock = ((m_Package[1] << 25) | (m_Package[2] << 17) | (m_Package[3] << 9) | (m_Package[4] << 1) | (m_Package[5] >> 7));
					uint16 ext   = (((m_Package[5] & 0x01) << 8) | m_Package[6]);

					return (Generics::OptionalType<ReferenceClock>(ReferenceClock(clock, ext)));
				}

				return (Generics::OptionalType<ReferenceClock>());
			}
			inline Generics::OptionalType<ReferenceClock> OriginalProgramClock () const
			{
				if (HasOriginalProgramClock())
				{
					uint32 offset = OriginalProgramClockOffset(); 
					uint64 clock = ((m_Package[offset] << 25) | (m_Package[offset + 1] << 17) | (m_Package[offset + 2] << 9) | (m_Package[offset + 3] << 1) | (m_Package[offset + 4] >> 7));
					uint16 ext   = (((m_Package[offset + 4] & 0x01) << 8) | m_Package[offset + 5]);

					return (Generics::OptionalType<ReferenceClock>(ReferenceClock(clock, ext)));
				}

				return (Generics::OptionalType<ReferenceClock>());
			}
			inline Generics::OptionalType<uint8> SpliceCountDown () const
			{
				if (HasSplicing())
				{
					return (Generics::OptionalType<uint8>(m_Package[SplicingOffset()]));
				}
				return (Generics::OptionalType<uint8>());
			}
			inline Generics::OptionalType<Generics::DataElement> PrivateData () const
			{
				if (HasPrivateData())
				{
					uint32 offset = PrivateDataOffset();
					
					return (Generics::OptionalType<Generics::DataElement>(Generics::DataElement(m_Package, offset+1, m_Package[offset])));
				}
				return (Generics::OptionalType<Generics::DataElement>());
			}
			inline Generics::OptionalType<uint16> LegalWindow () const
			{
				if (HasLegalWindow())
				{
					return (Generics::OptionalType<uint16>(m_Package.GetNumber<uint16, Generics::ENDIAN_BIG>(LegalWindowOffset())));
				}

				return (Generics::OptionalType<uint16>());
			}
			inline Generics::OptionalType<uint32> HypotheticalBitRate() const
			{
				if (HasValidLegalWindow() && HasHypoptheticalBitrate())
				{
					// TO BE COMPLETED
				}
				return (Generics::OptionalType<uint32>());
			}

		private:
			inline bool HasProgramClock () const
			{
				return ((m_Package[0] & 0x10) == 0x10);
			}
			inline bool HasOriginalProgramClock () const
			{
				return ((m_Package[0] & 0x08) == 0x08);
			}
			inline bool HasSplicing () const
			{
				return ((m_Package[0] & 0x04) == 0x04);
			}
			inline bool HasPrivateData () const
			{
				return ((m_Package[0] & 0x02) == 0x02);
			}
			inline bool HasExtension () const
			{
				return (((m_Package[0] & 0x01) == 0x01) && (m_Package[ExtendedExtensionOffset()]>0)); // Check Size..
			}
			inline bool HasLegalWindow () const
			{
				uint32 offset = ExtendedExtensionOffset();
				return (HasExtension() && (m_Package[offset] >= 3) && ((m_Package[ExtendedExtensionOffset()+1] & 0x80) == 0x80));
			}
			inline bool HasValidLegalWindow () const
			{
				return (HasLegalWindow() && ((m_Package[ExtendedExtensionOffset()+2] & 0x80) == 0x80));
			}
			inline bool HasHypoptheticalBitrate() const
			{
				uint32 offset = ExtendedExtensionOffset();
				return (HasExtension() && (m_Package[offset] >= 5) && ((m_Package[offset+1] & 0x40) == 0x40));
			}
			inline uint32 OriginalProgramClockOffset() const
			{
				return (HasProgramClock() ? 1 : 7); 
			}
			inline uint32 SplicingOffset () const
			{
				return (OriginalProgramClockOffset() + (HasOriginalProgramClock() ? 6 : 0)); 
			}
			inline uint32 PrivateDataOffset () const
			{
				return (SplicingOffset() + (HasSplicing() ? 1 : 0)); 
			}
			inline uint32 ExtendedExtensionOffset () const
			{
				return (PrivateDataOffset() + (HasPrivateData() ? 1 + m_Package[PrivateDataOffset()] : 0)); 
			}
			inline uint32 LegalWindowOffset () const
			{
				return (ExtendedExtensionOffset()+2);
			}
			inline uint32 HypoptheticalBitrateOffset ()
			{
				//TO BE FIXED
				return (LegalWindowOffset());
			}

		private:
			Generics::DataElement	m_Package;
	};

	class EXTERNAL TransportPackage
	{
		public:
			typedef enum Scrambled
			{
				SCRAMBLING_NONE     = 0x00,
				SCRAMBLING_EVEN     = 0x80,
				SCRAMBLING_RESERVED = 0x40,
				SCRAMBLING_ODD      = 0xC0
			};

		private:
			TransportPackage ();

		public:
			inline TransportPackage (const Generics::DataElement& data) :
				m_Package (data)
			{
			}
			inline TransportPackage (const TransportPackage& copy) :
				m_Package (copy.m_Package)
			{
			}
			inline ~TransportPackage ()
			{
			}

			inline TransportPackage& operator= (const TransportPackage& RHS)
			{
				m_Package = RHS.m_Package;

				return (*this);
			}

		public:
			static uint8 ID ()
			{
				return (0x47);
			}
			inline bool IsValid() const
			{
				return ( (m_Package[0] == TransportPackage::ID()) && ((m_Package[1] & 0x80) == 0) && ((m_Package[3] & 0x30) != 0x00) );
			}
			inline bool IsPayLoadStart () const
			{
				return ((m_Package[1] & 0x40) == 0x40);
			}
			inline bool HasPriority() const
			{
				return ((m_Package[1] & 0x20) == 0x20);
			}
			inline uint16 Pid () const
			{
				return (((m_Package[1] & 0x1F) << 8) | m_Package[2]);
			}
			inline enum Scrambled Scrambling() const
			{
				return static_cast<enum Scrambled>(m_Package[3] & 0xC0);
			}
			inline bool HasAdaptionField() const
			{
				return ((m_Package[3] & 0x20) != 0);
			}
			inline bool HasPayload() const
			{
				return ((m_Package[3] & 0x10) != 0);
			}
			inline bool HasRemainder () const
			{
				return (IsPayLoadStart() && HasPayload() && (m_Package[PayLoadOffset()] != 0));
			}
			inline uint8 ContinuityCounter () const
			{
				return (m_Package[3] & 0x0F);
			}
			inline AdaptationField Adaptation() const
			{
				return (AdaptationField (Generics::DataElement(m_Package, 5, m_Package[4]))); 
			}
			inline Generics::DataElement Remainder () const
			{
				Generics::DataElement	result;

				if (IsPayLoadStart() == true)
				{
					uint32 offset = PayLoadOffset();

					if (m_Package[offset] != 0)
					{
						result = Generics::DataElement(m_Package, offset + 1, m_Package[offset]);
					}
				}

				return (result);
			}
			inline Generics::DataElement PayLoad () const
			{
				return (Generics::DataElement(m_Package,PayLoadOffset()));
			}
			inline Generics::DataElement SILoad () const
			{
				uint32 offset = PayLoadOffset();
				offset += (IsPayLoadStart() ? m_Package[offset] + 1 : 0);

				return (Generics::DataElement(m_Package,offset));
			}

		private:
			inline bool HasAdaptationField () const
			{
				return ((m_Package[3] & 0x20) != 0);
			}
			inline uint32 PayLoadOffset () const
			{
				return (HasAdaptationField() ? 5 + m_Package[4] : 4);
			}

		private:
			Generics::DataElement	m_Package;
	};

} } // namespace Solutions::DVB

#endif // __MPEGTRANSPORTPACKAGE_H