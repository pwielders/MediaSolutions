#include "DataElementFile.h"

namespace Solutions { namespace Generics
{

DataElementFile::DataElementFile (const TextFragment& fileName)  :
	DataElement (),
	m_MemoryMappedFile(INVALID_HANDLE_VALUE),
	m_FileName(fileName.Text())
{
	// Start by opening the file..
	m_FileHandle = ::CreateFile(m_FileName.Data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_FileHandle != INVALID_HANDLE_VALUE)
	{
		// Check if the file is longer than 0 bytes..
		// Currently we only allow files of up to 4Gb (32 bits) if bigger we stop !!
		if ( (::GetFileInformationByHandle (m_FileHandle, &m_FileInfo) == FALSE) ||
			 ((m_FileInfo.nFileSizeHigh != 0) || (m_FileInfo.nFileSizeLow == 0)) )
		{
			::CloseHandle (m_FileHandle);

			m_FileHandle = INVALID_HANDLE_VALUE;
		}
		else
		{
			// Open the file in MM mode as one element.
			m_MemoryMappedFile = ::CreateFileMapping(m_FileHandle, NULL, PAGE_READONLY, 0, 0, NULL);

			if (m_MemoryMappedFile == INVALID_HANDLE_VALUE)
			{
				::CloseHandle (m_FileHandle);

				m_FileHandle = INVALID_HANDLE_VALUE;
			}
			else
			{
				// Seems like everything succeeded. Lets map it.
				UpdateCache (0, m_FileInfo.nFileSizeLow, static_cast<uint8*>(::MapViewOfFile (m_MemoryMappedFile, FILE_MAP_READ, 0, 0, 0)));
			}
		}
	}
}

/* virtual */ DataElementFile::~DataElementFile ()
{
	if (m_MemoryMappedFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle (m_MemoryMappedFile);
	}
	if (m_FileHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle (m_FileHandle);
	}
}

} } // namespace Solutions::Generics
