#ifndef __RTCPMESSAGE_H
#define __RTCPMESSAGE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----
#define MAX_REPORTBLOCK_SIZE	(6 * 4)  
#define MAX_RTCPMESSAGE_SIZE	(7 * 4)  

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace RTCP
{
	class EXTERNAL Message
	{
		public:
			class EXTERNAL ReportBlock
			{
				public:
					ReportBlock();
					ReportBlock(const Generics::DataElement& RHS);
					ReportBlock(const ReportBlock& copy);
					~ReportBlock();

					ReportBlock& operator= (const ReportBlock& RHS);
					ReportBlock& operator= (const Generics::DataElement& RHS);

				public:
					uint32		SynchronisationSource () const;
					void		SynchronisationSource (const uint32 value);
					uint8		FractionLost () const;
					void		FractionLost (const uint8 value);
					uint32		TotalNumberOfPackagesLost () const;
					void		TotalNumberOfPackagesLost (const uint32 value);
					uint32		HighestSequenceNumber () const;
					void		HighestSequenceNumber (const uint32 value);
					uint32		Jitter () const;
					void		Jitter (const uint32 value);
					uint32		LastTimeStamp () const;
					void		LastTimeStamp (const uint32 value);
					uint32		DeltaDelay () const;
					void		DeltaDelay (const uint32 value);

					const Generics::DataElement Data() const;

				private:
					Generics::DataElement		m_Buffer;

				public:
					class EXTERNAL Iterator
					{
						public:
							Iterator();
							Iterator(const Generics::DataElement& buffer);
							Iterator(const Iterator& copy);
							~Iterator();

							Iterator& operator= (const Iterator& copy);

						public:
							uint8 Count () const;
							bool IsValid() const;
							void Reset();
							bool Next();
							const ReportBlock Value() const;

						private:
							bool						m_PreStart;
							uint32						m_Current;
							Generics::DataElement					m_Buffer;
					};

			};

			class EXTERNAL ChunkItem
			{
				public:
					// Due to the dynamic length of this item, it is not possible to have setters on it, 
					// Creating an Item must be accomplished by ussing the Add Method on the ChunkItem 
					// class.
					class EXTERNAL Item
					{
						private:
							Item();
							Item& operator= (const Item&);

						public:
							Item(const uint8 type, const Generics::TextFragment& text);
							Item(const Generics::DataElement& buffer);
							Item(const Item& copy);
							~Item();
					
						public:
							uint8							Type () const;
							const Generics::TextFragment	Text () const;

						private:
							uint8					m_Type;
							Generics::TextFragment	m_Buffer;

						public:
							class Iterator
							{
								public:
									Iterator();
									Iterator(const Generics::DataElement& data);
									Iterator(const Iterator& copy);
									~Iterator();

									Iterator& operator= (const Iterator& copy);

								public:
									bool IsValid() const;
									uint16 Length() const;
									void Reset();
									bool Next();
									const Item Value() const;

								private:
									bool			m_PreStart;
									uint32			m_Current;
									Generics::DataElement		m_Buffer;
							};
					};

					class EXTERNAL Iterator
					{
						public:
							Iterator();
							Iterator(const Generics::DataElement& buffer);
							Iterator(const Iterator& copy);
							~Iterator();

							Iterator& operator= (const Iterator& copy);

						public:
							uint8 Count () const;
							bool IsValid() const;
							uint16 Length() const;
							void Reset();
							bool Next();
							const ChunkItem Value() const;

						private:
							uint32					m_Index;
							uint32					m_Current;
							mutable Generics::DataElement		m_Buffer;
					};

				public:
					ChunkItem(const uint32 syncSource	);
					ChunkItem(const Generics::DataElement& buffer);
					ChunkItem(const ChunkItem& copy);
					~ChunkItem();

					ChunkItem& operator= (const ChunkItem& RHS);

				public:
					uint32				SyncSourceId() const;
					void				SyncSourceId(uint32 syncSource);
					Item::Iterator		Items () const;
					const Generics::DataElement	Data() const;

					// If you add an item, you need to add the ItemChunk back to the parent !!!
					void				AddItem (const Item& item);

				private:
					Generics::DataElement	m_Buffer;
			};

		private:
			Message(const Message&);
			Message& operator= (const Message&);

		public:
			Message();
			Message(const Generics::DataElement& input);

		public:
			inline uint32				Size					() const
			{
				return (static_cast<uint32>(m_Buffer.Size()));
			}
			bool						IsBye					() const;
			bool						IsSDES					() const;
			bool						IsSenderReport			() const;
			bool						IsReceiverReport		() const;
			uint32						SynchronisationSource	() const;
			uint64						NTPTimeStamp			() const;
			uint32						RTPTimeStamp			() const;
			uint32						PackageCount			() const;
			uint32						OctetCount				() const;
			ReportBlock::Iterator		Reports					() const;
			ChunkItem::Iterator			Chunks					() const;

			// If the type of RTCP message is set, the package is reduced to 
			// te minimum. All chunks and reports are cleared. So always
			// set the type of Message before filling in Chunks or reports.
			void IsBye(const bool value);
			void IsSDES(const bool value);
			void IsSenderReport (const bool value);
			void IsReceiverReport (const bool value);

			void SynchronisationSource (const uint32 value);
			void NTPTimeStamp(const uint64 value);
			void RTPTimeStamp(const uint32 value);
			void PackageCount(const uint32 value);
			void OctetCount(const uint32 value);

			void AddReport(const ReportBlock& report);
			void AddChunk(const ChunkItem& chunk);

			const Generics::DataElement	Data() const;

		private:
			mutable Generics::DataElement	m_Buffer;
	};

} } // namespace Solutions::RTCP

#endif // __RTCPMESSAGE_H
