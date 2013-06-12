#ifndef __MPEGASSEMBLER_H
#define __MPEGASSEMBLER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGTransportPackage.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace MPEG
{
	template<const bool SIASSEMBLER, typename POLICYCLASS>
	class AssemblerType
	{
		private:
			AssemblerType(const AssemblerType<SIASSEMBLER,POLICYCLASS>& copy);
			AssemblerType<SIASSEMBLER,POLICYCLASS>& operator= (const AssemblerType<SIASSEMBLER,POLICYCLASS>&);

		public:
			AssemblerType(const uint16 pid = NUMBER_MAX_UNSIGNED(uint16)) : 
				m_ActiveBuffer(),
				m_Completed(0),
				m_PID(pid),
				m_Continuity(NUMBER_MAX_UNSIGNED(uint8)),
				m_LastOffset(0),
				m_Policy()
			{
				m_ActiveBuffer = Generics::DataElement(m_Policy.LeftBuffer());

				// We start from scratch..
				m_ActiveBuffer.Size(0);
			}
			~AssemblerType()
			{
			}

		public:
			inline uint16 PID () const
			{
				return (m_PID);
			}
			inline void Reset (const uint16 pid = PID())
			{
				m_Continuity = NUMBER_MAX_UNSIGNED(uint8);
				m_LastOffset = 0;
				m_ActiveBuffer.Size(0);
				m_Completed = 0;
				m_PID = pid;
			}
			bool Assemble (const TransportPackage& package)
			{
				bool myPackage = (package.Pid() == m_PID);

				// Only feed us good packages..
				ASSERT (package.IsValid());

				if (myPackage)
				{
					// Make sure the continuity is correct
					if (m_Continuity != NUMBER_MAX_UNSIGNED(uint8))
					{
						// Seems like we are building a new section. Up till now 
						// everything is oke. Check the COntinuity counter..
						if (((m_Continuity+1) % 0x0F) == package.ContinuityCounter())
						{
							// It's a valid one, and the next one, move on...
							m_LastOffset = static_cast<uint16>(m_ActiveBuffer.Size());
						}
						else if (m_Continuity != package.ContinuityCounter())
						{
							// It's an invalid package..Continuity fails. Wait for a new start !!!
							m_Continuity = NUMBER_MAX_UNSIGNED(uint8);
						}
					}

					// First see if we have some remianing stuff to process.
					if ( (m_Continuity != NUMBER_MAX_UNSIGNED(uint8)) || (package.IsPayLoadStart()) )
					{
						// We added the package, move on to the next !!!
						m_Continuity = package.ContinuityCounter();

						// Turn off the active buffer bit for completion
						m_Completed &= (m_ActiveBuffer == m_Policy.LeftBuffer() ? 0xFE :0xFD);

						// This might trigger the filling up of a previous package or triggering a complete package.
						if (package.IsPayLoadStart())
						{
							// If this is a "Start" we might need to close another part..
							if (SIASSEMBLER == true)
							{
								// If it is SI, there might be a remainder in this start frame..
								HandleRemainder(package.Remainder());
							}

							if ((SIASSEMBLER == false) && (IsElementComplete (true)))
							{
								BufferComplete();
							}

							// Prepare for the next package, the last was is swapped or failed !!
							m_LastOffset = 0;
							m_ActiveBuffer.Size(0);
						}

						Generics::DataElement payLoad(SIASSEMBLER == true ? package.SILoad() : package.PayLoad());

						if (payLoad.Size() != 0)
						{
							if (m_ActiveBuffer.Size(m_LastOffset + payLoad.Size()) == true)
							{
								// It seems to fit, add the data
								m_ActiveBuffer.Copy(payLoad, m_LastOffset);

								// Check if it is now completed
								if ( ((SIASSEMBLER == true)  && (IsSIComplete()           == true)) ||
									 ((SIASSEMBLER == false) && (IsElementComplete(false) == true)) )
								{
									BufferComplete();
								}
							}
							else
							{
								// Loose the package it failed, out of memory !!!
								m_Continuity = NUMBER_MAX_UNSIGNED(uint8);
								m_LastOffset = 0;
								m_ActiveBuffer.Size(0);
							}
						}
					}
				}

				return (myPackage);
			}

			// Used by derived classes to handle completed data....
			virtual void Assembled (const Generics::DataElement& /* data */)
			{
			}

			Generics::OptionalType<Generics::DataElement> CompletedFrame (const bool clearOnRetrieval)
			{
				if ((m_ActiveBuffer == m_Policy.LeftBuffer()) && ((m_Completed & 0x02) != 0))
				{
					if (clearOnRetrieval) {	m_Completed &= 0xFD; }

					return (Generics::OptionalType<Generics::DataElement>(Generics::DataElement(m_Policy.RightBuffer())));
				}
				else if ((m_ActiveBuffer == m_Policy.RightBuffer()) && ((m_Completed & 0x01) != 0))
				{
					if (clearOnRetrieval) {	m_Completed &= 0xFE; }

					return (Generics::OptionalType<Generics::DataElement>(Generics::DataElement(m_Policy.LeftBuffer())));
				}

				return (Generics::OptionalType<Generics::DataElement>());
			}

			Generics::OptionalType<Generics::DataElement> CurrentFrame (const bool clearOnRetrieval)
			{
				// There is no use, calling this, if it is not an SI assembler !!!
				ASSERT (SIASSEMBLER == true);

				if (((m_ActiveBuffer == m_Policy.LeftBuffer()) && ((m_Completed & 0x01) != 0)) || 
					((m_ActiveBuffer == m_Policy.RightBuffer()) && ((m_Completed & 0x02) != 0)) )
				{
					if (clearOnRetrieval) {	m_Completed &= (m_ActiveBuffer == m_Policy.LeftBuffer() ? 0xFE :0xFD); }

					return (Generics::OptionalType<Generics::DataElement>(m_ActiveBuffer));
				}

				return (Generics::OptionalType<Generics::DataElement>());
			}

		private:
			void HandleRemainder(const Generics::DataElement& remainder)
			{
				// Add the "remaining" stuff (if needed) to the "remaining" buffer and switch.
				if ( (remainder.Size() != 0) && (m_ActiveBuffer.Size(m_LastOffset + remainder.Size()) == true) )
				{
					// It seems to fit, add the data
					m_ActiveBuffer.Copy(remainder, m_LastOffset);
				}

				// See if we have a completed package..
				if (IsSIComplete())
				{
					BufferComplete();
				}
			}
			void BufferComplete ()
			{
				// This is a ready buffer, swap it.. and report it
				Assembled (m_ActiveBuffer);

				// Swap buffers, if possible..
				if (m_ActiveBuffer == m_Policy.LeftBuffer())
				{
					// Report back the active buffer, it might have changed due to an incremented size
					m_Policy.LeftBuffer(m_ActiveBuffer);
					m_Completed |= 0x01;
					m_ActiveBuffer = Generics::DataElement(m_Policy.RightBuffer()); 
				}
				else
				{
					// Report back the active buffer, it might have changed due to an incremented size
					m_Policy.RightBuffer(m_ActiveBuffer);
					m_Completed |= 0x02;
					m_ActiveBuffer = Generics::DataElement(m_Policy.LeftBuffer()); 
				}

				// Prepare for the next package..
				m_LastOffset = 0;
				m_Continuity = NUMBER_MAX_UNSIGNED(uint8);
			}
			bool IsSIComplete ()
			{
				if (m_ActiveBuffer.Size() >= 3)
				{
					uint16 requested = (((m_ActiveBuffer[1] & 0x0F) << 8) | (m_ActiveBuffer[2])) + 3;

					// Check if we need to trim off padding data..
					if (requested <= m_ActiveBuffer.Size())
					{
						// Report this section to the ones interested..
						m_ActiveBuffer.Size(requested);
						
						return(true);
					}
				}

				return (false);
			}
			bool IsElementComplete (bool newStart)
			{
				if (m_ActiveBuffer.Size() >= 6)
				{
					uint16 requested = ((m_ActiveBuffer[4] << 8) | (m_ActiveBuffer[5]));

					// Check if we need to trim off padding data..
					if (requested == 0)
					{
						if (newStart == true)						
						{
							// Seems like we are done with the package, give it a length..
							m_ActiveBuffer.SetNumber<uint16, Generics::ENDIAN_BIG>(4, static_cast<uint16>(m_ActiveBuffer.Size() - 6));

							return (true);
						}

						return (false);
					}
					else if ( (requested + 6) <= m_ActiveBuffer.Size())
					{
						// Report this section to the ones interested..
						m_ActiveBuffer.Size(requested + 6);

						return(true);
					}						
				}

				return (false);
			}

		private:
			POLICYCLASS								m_Policy;
			Generics::DataElement					m_ActiveBuffer;
			uint16									m_PID;
			uint8									m_Continuity;
			uint16									m_LastOffset;
			uint8									m_Completed;
	};

	template <const uint32 SECTIONSIZE>
	class SectionAssemblerPolicy
	{
		private:
			SectionAssemblerPolicy(const SectionAssemblerPolicy<SECTIONSIZE>&);
			SectionAssemblerPolicy<SECTIONSIZE>& operator= (const SectionAssemblerPolicy<SECTIONSIZE>& RHS);

		public:
			SectionAssemblerPolicy() :
				m_LeftBuffer(),
				m_RightBuffer()
			{
			}
			~SectionAssemblerPolicy()
			{
			}

		public:	
			inline Generics::ProxyType<Generics::IBuffer> LeftBuffer()
			{
				return (m_LeftBuffer);
			}
			inline Generics::ProxyType<Generics::IBuffer> RightBuffer()
			{
				return (m_RightBuffer);
			}
			inline void LeftBuffer(Generics::ProxyType<Generics::IBuffer>& buffer)
			{
				// No way these can change
				ASSERT (static_cast<Generics::IProxy*>(&m_LeftBuffer) == static_cast<Generics::IProxy*>(&(*buffer)));
			}
			inline void RightBuffer(Generics::ProxyType<Generics::IBuffer>& buffer)
			{
				// No way these can change
				ASSERT (static_cast<Generics::IProxy*>(&m_RightBuffer) == static_cast<Generics::IProxy*>(&(*buffer)));
			}


		private:
			Generics::ScopedStorage<SECTIONSIZE>	m_LeftBuffer;
			Generics::ScopedStorage<SECTIONSIZE>	m_RightBuffer;
	};

	template <const uint32 SECTIONSIZE>
	class StreamAssemblerPolicy
	{
		private:
			StreamAssemblerPolicy(const StreamAssemblerPolicy<SECTIONSIZE>&);
			StreamAssemblerPolicy<SECTIONSIZE>& operator= (const StreamAssemblerPolicy<SECTIONSIZE>& RHS);

		public:
			StreamAssemblerPolicy() :
				m_LeftBuffer(Generics::ExtendableStorage<Generics::StandAloneStorage>::Create(SECTIONSIZE)),
				m_RightBuffer(Generics::ExtendableStorage<Generics::StandAloneStorage>::Create(SECTIONSIZE))
			{
			}
			~StreamAssemblerPolicy()
			{
			}

		public:	
			inline Generics::ProxyType<Generics::IBuffer> LeftBuffer()
			{
				return (m_LeftBuffer);
			}
			inline Generics::ProxyType<Generics::IBuffer> RightBuffer()
			{
				return (m_RightBuffer);
			}
			inline void LeftBuffer(Generics::ProxyType<Generics::IBuffer>& buffer)
			{
				m_LeftBuffer = buffer;
			}
			inline void RightBuffer(Generics::ProxyType<Generics::IBuffer>& buffer)
			{
				m_RightBuffer = buffer;
			}

		private:
			Generics::ProxyType<Generics::IBuffer>	m_LeftBuffer;
			Generics::ProxyType<Generics::IBuffer>	m_RightBuffer;
	};

} } // namespace Solutions::DVB

#endif // __MPEGASSEMBLER_H