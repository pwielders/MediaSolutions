#ifndef __DATAELEMENTSTREAMER_H
#define __DATAELEMENTSTREAMER_H

#include "IMedia.h"
#include "DataElementIterator.h"

namespace Solutions { namespace Generics
{
	class EXTERNAL DataElementStreamer
	{	
		private:
			DataElementStreamer(const DataElementStreamer&);
			DataElementStreamer& operator= (const DataElementStreamer&);

		public:
			DataElementStreamer(IMedia* media);
			virtual ~DataElementStreamer();

		public:
			bool Receive ();
			bool Receive (DataElementIterator& newBuffer);
			bool Send ();
			bool Send (const DataElementIterator& newBuffer);

		private:
			void MoveSendIterator ();
			void MoveReceiveIterator ();

		private:
			IMedia*				m_Media;

			DataElementIterator	m_ReceiveIterator;
			uint32				m_ReceiveOffset;

			DataElementIterator	m_SendIterator;
			uint32				m_SendOffset;
	};

} } // namespace Solutions::Generics

#endif // __DATAELEMENTSTREAMER_H
