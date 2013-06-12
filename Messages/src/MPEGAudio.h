#ifndef __MPEGAUDIO_H
#define __MPEGAUDIO_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGHeader.h"
#include "MPEGStream.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace MPEG
{
	class EXTERNAL MPEGAudio
	{
		private:
			class EXTERNAL MPEGElement
			{
				public:
					MPEGElement (const Generics::DataElement& source, const uint64 offset);
					MPEGElement() :
						m_Buffer(),
						m_Index(0),
						m_PlayTime(0),
						m_Frame()
					{
					}
					MPEGElement (const MPEGElement& copy) : 
						m_Buffer(copy.m_Buffer),
						m_Index(copy.m_Index),
						m_PlayTime(copy.m_PlayTime),
						m_Frame(copy.m_Frame)
					{
					}
					inline ~MPEGElement () 
					{
					}

					MPEGElement& operator=  (const MPEGElement& RHS)
					{
						m_Buffer = RHS.m_Buffer;
						m_Index = RHS.m_Index;
						m_PlayTime = RHS.m_PlayTime;
						m_Frame = RHS.m_Frame;

						return (*this);
					}

				public:
					void Reset ()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint64);
						m_PlayTime = 0;
					}
					inline bool IsValid() const
					{
						return (m_Index < m_Buffer.Size());
					}
					inline AudioHeader Frame() const
					{
						ASSERT (IsValid());

						return (m_Frame);
					}
					inline uint64 PlayTime() const
					{
						return (m_PlayTime);
					}
					inline uint32 BitRate() const
					{
						return (m_Frame.IsValid() ? m_Frame.BitRate() : 0);
					}
					bool Next();

				private:
					Generics::DataElement	m_Buffer;
					AudioHeader				m_Frame;
					uint64					m_Index;
					uint64					m_PlayTime;
			};

		public:
			MPEGAudio ();
			MPEGAudio (const MPEGAudio& copy);
			MPEGAudio (const MPEGStream& stream);
			inline ~MPEGAudio() {}

			MPEGAudio& operator= (const MPEGAudio& RHS);

		public:
			void Reset ()
			{
				m_BaseTime = 0;
				m_Index = MPEGElement();
				m_Source.Reset();
			}
					
			inline uint32 BitRate () const
			{
				return (m_Index.BitRate());
			}
			inline uint64 PlayTime () const
			{
				// PlayTime in milliseconds
				return (m_BaseTime + m_Index.PlayTime());
			}
			inline bool IsValid() const
			{
				return (m_Index.IsValid());
			}
			inline AudioHeader Frame() const
			{
				return (m_Index.Frame());
			}
			bool Next();

	private:
		MPEGElement				m_Index;
		MPEGStream				m_Source;
		uint64					m_BaseTime;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGVIDEO_H