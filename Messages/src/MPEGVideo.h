#ifndef __MPEGVIDEO_H
#define __MPEGVIDEO_H

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
	class EXTERNAL MPEGVideo
	{
		private:
			class EXTERNAL InfoLoader
			{
				public:
					typedef enum
					{
						NO_LOADING,
						GROUP_LOADING,
						PICTURE_LOADING,
						SLICE_LOADING

					}	CollectorState;

				public:
					inline InfoLoader() :
						m_CurrentState(NO_LOADING),
						m_SliceHeader(),
						m_PictureHeader(),
						m_GroupHeader(),
						m_SliceBuffer(Generics::DataStorage::Create(2048)),
						m_PictureBuffer(Generics::DataStorage::Create(128)),
						m_GroupBuffer(Generics::DataStorage::Create(128))
					{
					}
					inline InfoLoader(const InfoLoader& copy) :
						m_CurrentState(copy.m_CurrentState),
						m_SliceHeader(copy.m_SliceHeader),
						m_PictureHeader(copy.m_PictureHeader),
						m_GroupHeader(copy.m_GroupHeader),
						m_SliceBuffer(copy.m_SliceBuffer),
						m_PictureBuffer(copy.m_PictureBuffer),
						m_GroupBuffer(copy.m_GroupBuffer)
					{
					}

					inline ~InfoLoader ()
					{
					}

					inline InfoLoader& operator= (const InfoLoader& RHS)
					{
						m_CurrentState = RHS.m_CurrentState;
						m_SliceHeader = RHS.m_SliceHeader;
						m_PictureHeader = RHS.m_PictureHeader;
						m_GroupHeader = RHS.m_GroupHeader;
						m_SliceBuffer = RHS.m_SliceBuffer;
						m_PictureBuffer = RHS.m_PictureBuffer;
						m_GroupBuffer = RHS.m_GroupBuffer;

						return (*this);
					}

				public:
					inline bool IsValid() const
					{
						return ((m_CurrentState == NO_LOADING) && (m_GroupHeader.IsValid()) && (m_PictureHeader.IsValid()));
					}
					inline CollectorState State () const
					{
						return (m_CurrentState);
					}
					inline const MPEG::GroupOfPicturesHeader& GroupOfPicturesHeader() const
					{
						return (m_GroupHeader);
					}
					inline const MPEG::PictureHeader& PictureHeader() const
					{
						return (m_PictureHeader);
					}
					inline const MPEG::SliceHeader& SliceHeader() const
					{
						return (m_SliceHeader);
					}
					inline void Loaded ()
					{
						if (m_CurrentState != NO_LOADING)
						{
							m_CurrentState = NO_LOADING;

							if (m_CurrentState == SLICE_LOADING)
							{
								m_SliceHeader = MPEG::SliceHeader(m_GroupBuffer, 0);
							}
							else if (m_CurrentState == PICTURE_LOADING)
							{
								m_PictureHeader = MPEG::PictureHeader(m_GroupBuffer, 0);
							}
							else if (m_CurrentState == GROUP_LOADING)
							{
								m_GroupHeader = MPEG::GroupOfPicturesHeader(m_GroupBuffer, 0);
							}
							else
							{
								ASSERT (false);
							}
						}
					}
					void GroupOfPicturesLoading (const MPEG::GroupOfPicturesHeader& data)
					{
						ASSERT (m_CurrentState == NO_LOADING);

						m_GroupHeader = data;
					}
					void GroupOfPicturesLoading (const Generics::DataElement& data)
					{
						ASSERT ((m_CurrentState == NO_LOADING) || (m_CurrentState == GROUP_LOADING));

						m_CurrentState = GROUP_LOADING;

						// Make sure we can add a new part...
						m_GroupBuffer.Copy(data, m_GroupBuffer.Size());
					}
					void PictureLoading (const MPEG::PictureHeader& data)
					{
						ASSERT (m_CurrentState == NO_LOADING);

						m_PictureHeader = data;
					}
					void PictureLoading (const Generics::DataElement& data)
					{
						ASSERT ((m_CurrentState == NO_LOADING) || (m_CurrentState == PICTURE_LOADING));

						m_CurrentState = PICTURE_LOADING;

						// Make sure we can add a new part...
						m_PictureBuffer.Copy(data, m_PictureBuffer.Size());
					}
					void SliceLoading (const MPEG::SliceHeader& data)
					{
						ASSERT (m_CurrentState == NO_LOADING);

						m_SliceHeader = data;
					}
					void SliceLoading (const Generics::DataElement& data)
					{
						ASSERT ((m_CurrentState == NO_LOADING) || (m_CurrentState == SLICE_LOADING));

						m_CurrentState = SLICE_LOADING;

						// Make sure we can add a new part...
						m_SliceBuffer.Copy(data, m_SliceBuffer.Size());
					}

				private:
					CollectorState				m_CurrentState;
					MPEG::SliceHeader			m_SliceHeader;
					MPEG::PictureHeader			m_PictureHeader;
					MPEG::GroupOfPicturesHeader	m_GroupHeader;
					Generics::DataElement		m_SliceBuffer;
					Generics::DataElement		m_PictureBuffer;
					Generics::DataElement		m_GroupBuffer;
			};

			class EXTERNAL MPEGElement
			{
				public:
					MPEGElement(InfoLoader& loadedInfo);
					MPEGElement (const MPEGElement& copy);
					MPEGElement (const Generics::DataElement& source, const uint64 offset, InfoLoader& loadedInfo);
					inline ~MPEGElement () {}

					MPEGElement& operator=  (const MPEGElement& RHS);

				public:
					inline void Reset ()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint64);
					}
					inline bool IsValid() const
					{
						return ((m_Info.IsValid()) && (m_Index < m_Buffer.Size()));
					}
					inline bool PassedGroupOfPicturesHeader () const
					{
						return (m_NewGroupOfPicturesHeader);
					}
					inline bool PassedPictureHeader () const
					{
						return (m_NewPictureHeader);
					}
					inline uint64 Size () const
					{
						return (m_Buffer.Size());
					}
					inline uint64 PlayTime () const
					{
						// PlayTime in milliseconds
						return (m_PlayingTime);
					}
					bool NextGOP();
					bool NextPicture();
					bool NextSlice();

				private:
					uint32 PictureFrameTime() const;
					bool MetaDataSection(const Generics::DataElement&, const uint64);

				private:
					Generics::DataElement	m_Buffer;
					uint64					m_Index;
					bool					m_NewGroupOfPicturesHeader;
					bool					m_NewPictureHeader;
					uint64					m_PlayingTime;
					InfoLoader&				m_Info;
			};

		public:
			MPEGVideo ();
			MPEGVideo (const MPEGVideo& copy);
			MPEGVideo (const MPEGStream& stream);
			inline ~MPEGVideo() {}

			MPEGVideo& operator= (const MPEGVideo& RHS);

		public:
			inline void Reset ()
			{
				m_Source.Reset();
				m_Index = MPEGElement(m_LoadedInfo);
				m_BaseTime = 0;
			}
			inline bool IsValid() const
			{
				return (m_LoadedInfo.IsValid() && m_Index.IsValid());
			}
			inline const GroupOfPicturesHeader& CurrentGroupOfPicturesHeader () const
			{
				return (m_LoadedInfo.GroupOfPicturesHeader());
			}
			inline const PictureHeader& CurrentPictureHeader () const
			{
				return (m_LoadedInfo.PictureHeader());
			}
			inline SliceHeader CurrentSlice() const
			{
				return (m_LoadedInfo.SliceHeader());
			}
			inline bool PassedGroupOfPicturesHeader () const
			{
				return (m_Index.PassedGroupOfPicturesHeader());
			}
			inline bool PassedPictureHeader () const
			{
				return (m_Index.PassedPictureHeader());
			}
			inline uint64 PlayTime () const
			{
				// PlayTime in milliseconds
				return (m_BaseTime + m_Index.PlayTime());
			}

			bool NextGOP();
			bool NextPicture();
			bool NextSlice();

	private:
		InfoLoader				m_LoadedInfo;
		MPEGElement				m_Index;
		MPEGStream				m_Source;
		uint64					m_BaseTime;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGVIDEO_H