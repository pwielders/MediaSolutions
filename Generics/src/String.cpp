#include "Portability.h"
#include "String.h"
#include "Number.h"

namespace Solutions { namespace Generics
{

void ToString (const TCHAR realString[], std::string& result)
{
#ifndef _UNICODE

	result = std::string(realString);

#else

#ifdef __WIN32__

#pragma warning(disable : 4996)

	int requiredSize = ::wcstombs (NULL, realString, 0); 
	char* convertedText = static_cast<char*> (::_alloca((requiredSize + 1) * sizeof(char)));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::wcstombs( convertedText, realString, requiredSize + 1); 

#pragma warning(default : 4996)

    ASSERT (size != (size_t) (-1));

	result = std::string(convertedText);
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#endif // _UNICODE
}

void ToString (const TCHAR realString[], std::wstring& result)
{
#ifdef _UNICODE

	result = std::string(realString);

#else

#ifdef __WIN32__

#pragma warning(disable : 4996)

	int requiredSize = ::mbstowcs (NULL, realString, 0); 
	wchar_t* convertedText = static_cast<wchar_t*> (::_alloca((requiredSize + 1) * sizeof(wchar_t)));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::mbstowcs( convertedText, realString, requiredSize + 1); 

#pragma warning(default : 4996)

#if _TRACE_LEVEL > 0
    ASSERT (size != (size_t) (-1));
#endif

	result = std::wstring(convertedText);
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#endif // _UNICODE
}

const String ToString (const char realString[])
{
#ifdef _UNICODE

#ifdef __WIN32__

#pragma warning(disable : 4996)

	int requiredSize = ::mbstowcs (NULL, realString, 0); 
	wchar_t* convertedText = static_cast<wchar_t*> (::_alloca((requiredSize + 1) * 2));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::mbstowcs( convertedText, realString, requiredSize + 1); 

#pragma warning(default : 4996)

    ASSERT (size != (size_t) (-1));

	return (std::wstring(convertedText));
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#else

	return (String(realString));

#endif // _UNICODE
}

const String ToString (const wchar_t realString[])
{
#ifdef _UNICODE

	return (String(realString));

#else

#ifdef __WIN32__

	// First calculate the number of elements needed.
	int requiredSize = ::wcstombs_s (NULL, NULL, 0, realString, NUMBER_MAX_UNSIGNED(size_t)); 

	// Allocate the just calculated amount of elements.
	char* convertedText = static_cast<char*> (::_alloca(requiredSize + 1));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::wcstombs_s ( NULL, convertedText, requiredSize + 1, realString, NUMBER_MAX_UNSIGNED(size_t)); 

#if _TRACE_LEVEL > 0
    ASSERT (size != (size_t) (-1));
#endif

	return (String(convertedText));
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#endif // _UNICODE
}

const String ToString (const char realString[], unsigned int length)
{
#ifdef _UNICODE

#ifdef __WIN32__

#pragma warning(disable : 4996)

	int requiredSize = ::mbstowcs (NULL, realString, length); 
	wchar_t* convertedText = static_cast<wchar_t*> (::_alloca((requiredSize + 1) * 2));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::mbstowcs( convertedText, realString, requiredSize + 1); 

#pragma warning(default : 4996)

    ASSERT (size != (size_t) (-1));

	return (std::wstring(convertedText));
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#else

	return (String(realString, length));

#endif // _UNICODE
}

const String ToString (const wchar_t realString[], unsigned int length)
{
#ifdef _UNICODE

	return (String(realString), length);

#else

#ifdef __WIN32__

#pragma warning(disable : 4996)

	int requiredSize = ::wcstombs (NULL, realString, length); 
	char* convertedText = static_cast<char*> (::_alloca(requiredSize + 1));

#if _TRACE_LEVEL > 0
	size_t size = 
#endif
	::wcstombs( convertedText, realString, requiredSize + 1); 

#pragma warning(default : 4996)

#if _TRACE_LEVEL > 0
   ASSERT (size != (size_t) (-1));
#endif

	return (String(convertedText));
#endif

#ifdef __LINUX__
NOT IMPLEMENTED YET !!!!
#endif

#ifdef __POSIX__
NOT IMPLEMENTED YET !!!!
#endif

#endif // _UNICODE
}

template<>
uint32	
	TextFragment::Convert<Fractional> (
		const uint32			offset,
		Fractional&				Value,
		const NumberBase		Type,
		const TemplateIntToType<false>&	/* For compile time diffrentiation */) const
{
	uint32 integerPart;
	uint32 fractionalPart;
	uint32 index = Number<uint32,false> (integerPart, offset, Type);

	// See if we found something
	if ( (index != 0) && (Data()[offset+index] == '.') )
	{
		index += (1 + Number<uint32,false> (fractionalPart, offset+index+1, Type));
	}

	Value = Fractional(integerPart,fractionalPart);

	return (index);
}

template<>
uint32
	TextFragment::Convert<Fractional> (
		const uint32			offset,
		Fractional&				Value,
		const NumberBase		Type,
		const TemplateIntToType<true>&	/* For compile time diffrentiation */) const
{
	sint32 integerPart;
	uint32 fractionalPart;
	uint32 index = Number<sint32,true> (integerPart, offset, Type);

	// See if we found something
	if ( (index != 0) && (Data()[offset+index] == '.') )
	{
		index += (1 + Number<uint32,false> (fractionalPart, offset+index+1, Type));
	}

	Value = Fractional(integerPart,fractionalPart);

	return (index);
}

} } // namespace Solutions::Generics
