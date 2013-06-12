#ifndef __SLOTTEDSTORAGE_H
#define __SLOTTEDSTORAGE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	template<typename SLOTTEDOBJECT>
	class SlotInfoType
	{
		public:
			inline SlotInfoType () :
				m_StartSlot (0), m_Duration(0), m_RealInfo()
			{
			}
			inline SlotInfoType (const uint64 startTime, const uint32 duration, const SLOTTEDOBJECT& slotInfo) :
				m_StartSlot (startTime), m_Duration(duration), m_RealInfo(slotInfo)
			{
			}
			inline SlotInfoType(const SlotInfoType<SLOTTEDOBJECT>& copy) :
				m_StartSlot (copy.m_StartSlot), m_Duration (copy.m_Duration), m_RealInfo (copy.m_RealInfo)
			{
			}
			inline ~SlotInfoType()
			{
			}

			SlotInfoType<SLOTTEDOBJECT>& operator= (const SlotInfoType<SLOTTEDOBJECT>& RHS)
			{
				m_StartSlot = RHS.m_StartSlot;
				m_Duration = RHS.m_Duration;
				m_RealInfo = RHS.m_RealInfo;

				return (*this);
			}
		public:
			inline uint64 BeginSlot () const
			{
				return (m_StartSlot);
			}
			inline uint64 EndSlot () const
			{
				return (m_StartSlot + m_Duration);
			}
			inline uint32 Duration () const
			{
				return (m_Duration);
			}
			inline SLOTTEDOBJECT& Data ()
			{
				return (m_RealInfo);
			}
			inline const SLOTTEDOBJECT& Data () const
			{
				return (m_RealInfo);
			}
			inline bool operator< (uint64 slotTime) const
			{
				return ((m_StartSlot + m_Duration) < slotTime) 
			}
			inline bool operator> (uint64 slotTime) const
			{
				return (m_StartSlot > slotTime) 
			}
			inline bool operator<= (uint64 slotTime) const
			{
				return ((m_StartSlot + m_Duration) <= slotTime) 
			}
			inline bool operator>= (uint64 slotTime) const
			{
				return (m_StartSlot >= slotTime);
			}
			inline bool operator== (uint64 slotTime) const
			{
				return ((m_StartSlot <= slotTime) && ((m_StartSlot + m_Duration) > slotTime));
			}
			inline bool operator!= (uint64 slotTime) const
			{
				return (!operator==(slotTime));
			}

		private:
			uint64			m_StartSlot;
			uint32			m_Duration;
			SLOTTEDOBJECT	m_RealInfo;
	};

	template<typename SLOTTEDOBJECT, const uint16 LENGTH>
	class SlottedStorage
	{
		private:
			SlottedStorage(const SlottedStorage<SLOTTEDOBJECT,LENGTH>&);
			SlottedStorage<SLOTTEDOBJECT,LENGTH>& operator= (const SlottedStorage<SLOTTEDOBJECT,LENGTH>&);

		private:
			template<typename SLOTTEDOBJECT>
			class InternalSlotInfoType
			{
				public:
					inline InternalSlotInfoType () :
						m_Slot (0), m_RealInfo()
					{
					}
					inline InternalSlotInfoType (const uint32 slot, const SLOTTEDOBJECT& slotInfo) :
						m_Slot (slot), m_RealInfo(slotInfo)
					{
					}
					inline InternalSlotInfoType(const InternalSlotInfoType<SLOTTEDOBJECT>& copy) :
						m_Slot (copy.m_Slot), m_RealInfo (copy.m_RealInfo)
					{
					}
					inline ~InternalSlotInfoType()
					{
					}

					InternalSlotInfoType<SLOTTEDOBJECT>& operator= (const InternalSlotInfoType<SLOTTEDOBJECT>& RHS)
					{
						m_Slot = RHS.m_Slot;
						m_RealInfo = RHS.m_RealInfo;

						return (*this);
					}
				public:
					inline uint32 Slot () const
					{
						return (m_Slot);
					}
					inline SLOTTEDOBJECT& Data ()
					{
						return (m_RealInfo);
					}
					inline const SLOTTEDOBJECT& Data () const
					{
						return (m_RealInfo);
					}

				private:
					uint32			m_Slot;
					SLOTTEDOBJECT	m_RealInfo;
			};

		public:
			template <typename SLOTTEDOBJECT, const uint16 LENGTH>
			class SlotInfoArray
			{
				private:
					SlotInfoArray (const SlotInfoArray<SLOTTEDOBJECT,LENGTH>&);
					SlotInfoArray<SLOTTEDOBJECT,LENGTH>& operator= (const SlotInfoArray<SLOTTEDOBJECT,LENGTH>&);

				public:
					SlotInfoArray () :
						m_NextSlot (0),
						m_NextArray(NULL)
					{
					}
					~SlotInfoArray ()
					{
						if (m_NextArray != NULL)
						{
							delete m_NextArray;
						}
					}

					SlotInfoArray<SLOTTEDOBJECT,LENGTH>* InsertSlot (const SlotInfoType<SLOTTEDOBJECT>& newEntry)
					{
						SlotInfoArray<SLOTTEDOBJECT,LENGTH>* result = this;

						if (m_NextSlot != LENGTH)
						{
							uint32 slot = static_cast<uint32> (newEntry.EndSlot () - m_MinTime);
							m_SlottedArray[m_NextSlot++] = InternalSlotInfoType<SLOTTEDOBJECT>(slot, newEntry.Data());
						}
						else
						{
							m_NextArray = new SlotInfoArray<SLOTTEDOBJECT,LENGTH>();
							m_NextArray->InsertSlot(newEntry);
							m_NextArray->m_MinTime = newEntry.BeginSlot();
							result = m_NextArray;
						}

						return (result);
					}
					
					inline bool HasData () const
					{
						return (m_NextSlot != 0);
					}
					inline uint32 Length () const
					{
						return (m_NextSlot);
					}
					uint64 BeginSlot (const uint32 index) const
					{
						uint64 result = NULL;

						if (index < LENGTH)
						{
							uint32 baseOffset = 0;

							if (index > 0)
							{
								baseOffset = m_SlottedArray[index-1].Slot();
							}

							result = m_MinTime + baseOffset;
						}
						else if (m_NextArray != NULL)
						{
							result = (*m_NextArray).BeginSlot(index-LENGTH);
						}

						return (result);
					}
					uint64 EndSlot (const uint32 index) const
					{
						uint64 result = NULL;

						if (index < LENGTH)
						{
							uint32 baseOffset = 0;

							result = m_MinTime + m_SlottedArray[index].Slot ();
						}
						else if (m_NextArray != NULL)
						{
							result = (*m_NextArray).EndSlot(index-LENGTH);
						}

						return (result);
					}

					SLOTTEDOBJECT* operator[] (const uint32 index)
					{
						SLOTTEDOBJECT* result = NULL;

						if (index < LENGTH)
						{
							result = &m_SlottedArray[index].Data();
						}
						else if (m_NextArray != NULL)
						{
							result = (*m_NextArray)[index-LENGTH];
						}

						return (result);
					}
					const SLOTTEDOBJECT* operator[] (const uint32 index) const
					{
						const SLOTTEDOBJECT* result = NULL;

						if (index < LENGTH)
						{
							result = &m_SlottedArray[index].Data();
						}
						else if (m_NextArray != NULL)
						{
							result = (*m_NextArray)[index-LENGTH];
						}

						return (result);
					}
					bool Index (const uint32 index, SlotInfoType<SLOTTEDOBJECT>& info)
					{
						bool result = true;

						if (index < LENGTH)
						{
							uint32 baseOffset = 0;

							if (index > 0)
							{
								baseOffset = m_SlottedArray[index-1].Slot();
							}

							uint32 duration   = m_SlottedArray[index].Slot () - baseOffset; 
							info = SlotInfoType<SLOTTEDOBJECT>((m_MinTime + baseOffset), duration, m_SlottedArray[index].Data());
						}
						else if (m_NextArray != NULL)
						{
							result = (*m_NextArray).Index(index-LENGTH, info);
						}
						else
						{
							result = false;
						}

						return (result);
					}
					SlotInfoArray<SLOTTEDOBJECT,LENGTH>* FindArray (const uint64 slotTime)
					{
						SlotInfoArray<SLOTTEDOBJECT,LENGTH>* result = NULL;

						if ((m_NextSlot == 0) || (slotTime < (m_MinTime + (m_SlottedArray[m_NextSlot-1].Slot()))))
						{
							if (m_NextSlot != 0)
							{
								// Yes it is within these slots, do a bubble search here
								result = this;
							}
						}
						else if (m_NextArray != NULL)
						{
							result = m_NextArray->FindArray(slotTime);
						}

						return (result);
					}

					SlotInfoArray<SLOTTEDOBJECT,LENGTH>* NextSlot ()
					{
						return (m_NextArray);
					}
				
					uint32 BubleSearch (const uint64 slotTime) const
					{
						ASSERT (slotTime >= m_MinValue);

						uint32 highBoundary = LENGTH;
						uint32 lowBoundary = 0;
						uint32 index = (m_NextSlot / 2);
						uint32 searchSlot = static_cast<uint32>(slotTime - m_MinTime);
						bool found = false;

						while (found == false)
						{
							uint32 beginTime = 0;

							if (index > 0)
							{
								beginTime = m_SlottedArray[index-1].Slot();
							}

							// Did we hit it ?
							if ((beginTime <= searchSlot) && (searchSlot < m_SlottedArray[index].Slot ()))
							{
								found = true;
							}
							else
							{
								// No change the index..
								if (searchSlot <= beginTime)
								{
									highBoundary = index;

									// Move down..
									index = index - ((index - lowBoundary) / 2);
								}
								else
								{
									lowBoundary = index;

									// Move up..
									index = index + ((highBoundary - index) / 2);
								}	
							}
						}

						return (found == true ? index : ~0);
					}

				private:
					uint64									m_MinTime;
					unsigned int							m_NextSlot;
					InternalSlotInfoType<SLOTTEDOBJECT>		m_SlottedArray[LENGTH];
					SlotInfoArray<SLOTTEDOBJECT,LENGTH>*	m_NextArray;
			};

		public:
			SlottedStorage      () :
				m_Root          (new SlotInfoArray<SLOTTEDOBJECT,LENGTH>()),
				m_CurrentBlock  (m_Root)
			{
			}
			~SlottedStorage()
			{
				// Clear all allocated space !!!
				delete m_Root;
			}

		public:
			inline bool HasData () const
			{
				return (m_Root->HasData());
			}
			inline void AddSlot (const SlotInfoType<SLOTTEDOBJECT>& info)
			{
				m_CurrentBlock = m_CurrentBlock->InsertSlot(info);
			}
			inline bool FindObject (const uint64 slotTime, SlotInfoType<SLOTTEDOBJECT>& info) const
			{
				SlotInfoType<SLOTTEDOBJECT>* result = NULL;
				SlotInfoArray<SLOTTEDOBJECT,LENGTH>* foundArray = m_Root->FindArray(slotTime);

				if (foundArray != NULL) 
				{
					uint32 index = foundArray->BubleSearch (slotTime);

					if (index != ~0)
					{
						result = &(foundArray->Index(index));
					}
				}

				return (result);
			}

			SLOTTEDOBJECT& operator[] (const uint32 index)
			{
				SLOTTEDOBJECT* result = m_Root[index];

				ASSERT (result == NULL);
				
				return (*result);
			}

			const SLOTTEDOBJECT& operator[] (const uint32 index) const
			{
				SLOTTEDOBJECT* result = m_Root[index];

				ASSERT (result == NULL);
				
				return (*result);
			}

			inline void Lock ()
			{
				return (m_AdminLock.Lock());
			}

			inline void Unlock()
			{
				return (m_AdminLock.Unlock());
			}

			uint32 FindIndex (const uint64 slotTime, SlotInfoArray<SLOTTEDOBJECT,LENGTH>*& foundArray) const
			{
				uint32 result = ~0;
				foundArray = m_Root->FindArray(slotTime);

				if (foundArray != NULL) 
				{
					result = foundArray->BubleSearch (slotTime);
				}

				return (result);
			}

			inline SlotInfoArray<SLOTTEDOBJECT,LENGTH>& Root ()
			{
				return (*m_Root);
			}

		private:
			SlotInfoArray<SLOTTEDOBJECT,LENGTH>*	m_Root;
			SlotInfoArray<SLOTTEDOBJECT,LENGTH>*	m_CurrentBlock;
			CriticalSection							m_AdminLock;
	};

} } // namespace Solutions::Generics

#endif // __SLOTTEDSTORAGE_H
