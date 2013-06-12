#ifndef __DATAELEMENTFILE_H
#define __DATAELEMENTFILE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "String.h"
#include "DataElement.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	// The datapackage is the abstract of a package that needs to be send over the line.
	class EXTERNAL DataElementFile : public DataElement
	{
		private:
			DataElementFile ();
			DataElementFile(const DataElementFile&);
			DataElementFile& operator= (const DataElementFile&);

		public:
			DataElementFile (const TextFragment& fileName);
			virtual ~DataElementFile ();

		public:
			inline bool IsValid () const
			{
				return (m_FileHandle != INVALID_HANDLE_VALUE);
			}
			inline const TextFragment& FileName () const
			{
				return (m_FileName);
			}

		private:
			HANDLE						m_FileHandle;
			HANDLE						m_MemoryMappedFile;
			BY_HANDLE_FILE_INFORMATION	m_FileInfo;
			TextFragment				m_FileName;
	};

} } // namespace Solutions::Generics

#endif // __DATAELEMENTFILE_H