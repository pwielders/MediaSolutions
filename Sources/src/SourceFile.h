#ifndef __SOURCEFILE_H
#define __SOURCEFILE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	template<const TCHAR FILETYPE[]>
	class FileType : public Generics::DataElementFile
	{
		private:
			FileType ();
			FileType (const FileType<FILETYPE>& RHS);
			FileType<FILETYPE>& operator= (const FileType<FILETYPE>& RHS);

		public:
			inline FileType(const Generics::URL& identifier) : Generics::DataElementFile(FileNameFromURL(identifier))
			{
			}
			virtual ~FileType ()
			{
			}

		public:
			static Generics::TextFragment FileNameFromURL(const Generics::URL& identifier)
			{	
				return (identifier.Path().IsSet() ? identifier.Path().Value() : Generics::TextFragment());
			}

			inline const Generics::OptionalType<Generics::URL> SourceId () const
			{
				if (Generics::DataElementFile::IsValid())
				{
					Generics::URL result (Generics::URL::SCHEME_FILE);

					result.Path(Generics::DataElementFile::FileName());

					return (Generics::OptionalType<Generics::URL>(result));
				}

				return (Generics::OptionalType<Generics::URL>());
			}

			inline static bool IsValidSource (const Generics::URL& identifier)
			{
				if ((identifier.Type() == Generics::URL::SCHEME_FILE) && (identifier.Path().IsSet() == true))
				{
					Generics::PathParser parser (identifier.Path().Value());

					// It's better to also check if they are referrinf to file as type and from 
					// the local host, for now we make it simple, just the extenstion arth the end of the URL ;-)
					//TODO: Make the check more contraining, see above...

					return ((parser.Extension().IsSet() == true) && (Generics::TextFragment(FILETYPE).EqualText(parser.Extension().Value()) == true));
				}

				return (false);
			}
	};

} } // namespace Solutions::Source

#endif // __SOURCEFILE_H