#ifndef __MEDIASTREAMER_H
#define __MEDIASTREAMER_H

#include "IMedia.h"
#include "DataElement.h"
#include "DataElementContainer.h"

namespace Solutions { namespace Generics
{
	class EXTERNAL MediaSender
	{	
		private:
			MediaSender(const MediaSender&);
			MediaSender& operator= (const MediaSender&);

		public:
			MediaSender(IMedia* media);
			virtual ~MediaSender();

		public:
			IMedia* Media();

			// Description: This method will send any left information. After this call is made and there
			//              is still some data left to send, the method returns false to indicate that not
			//              all information was send.
			//              Always call this method before inserting new data, just to make sure that the 
			//              streamer is available to send data out.
			bool Send ();

			// Description: This method starts a request to send supplied data. Only call this if there is no other 
			//              send pending. This can be checked by calling the Send method without parameters, 
			//              that method will return true if there is no more data left to send.
			bool Send (const DataElement& buffer);

			// Description: This method starts a request to send supplied data. Only call this if there is no other 
			//              send pending. This can be checked by calling the Send method without parameters, 
			//              that method will return true if there is no more data left to send.
			bool Send (const DataElementContainer& buffer);

		private:
			IMedia*				m_Media;
			DataElement			m_SendBuffer;
			uint32				m_SendOffset;
			CriticalSection		m_Admin;
	};

	class EXTERNAL MediaReceiver
	{	
		private:
			MediaReceiver(const MediaReceiver&);
			MediaReceiver& operator= (const MediaReceiver&);

		public:
			MediaReceiver(IMedia* media);
			virtual ~MediaReceiver();

		public:
			IMedia* Media();

			// Description: This method will send any left information. After this call is made and there
			//              is still some data left to send, the method returns false to indicate that not
			//              all information was send.
			//              Always call this method before inserting new data, just to make sure that the 
			//              streamer is available to send data out.
			uint32 Receive ();

			// Description: This method starts a request to send supplied data. Only call this if there is no other 
			//              send pending. This can be checked by calling the Send method without parameters, 
			//              that method will return true if there is no more data left to send.
			uint32 Receive (const DataElement& buffer);

			// Description: Flush all currently present stuff, we do not need it. Restart receiving. The method
			//              returns the number of bytes flushed.
			uint32 Flush ();

		private:
			IMedia*				m_Media;
			DataElement			m_ReceiveBuffer;
			uint32				m_ReceiveOffset;
			CriticalSection		m_Admin;
	};

} } // namespace Solutions::Generics

#endif // __MEDIASTREAMER_H
