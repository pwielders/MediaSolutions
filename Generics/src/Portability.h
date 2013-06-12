// W3 -- Make sure the "clas 'xxxx' needs to have dll-interface to be used by clients of class 'xxxx'"
#pragma warning (disable: 4251)

// W3 -- Make sure the "C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc"
#pragma warning (disable: 4530)

// W4 -- Make sure the "conditional expression is constant"
#pragma warning (disable: 4127)

#ifndef __PORTABILITY_H
#define __PORTABILITY_H

#ifdef WIN32

#define __WIN32__
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
//#define WIN32_LEAN_AND_MEAN
#define NOWINRES
//#define NOGDI
#define NOSERVICE
#define NOMCX
#define NOIME
#include <Windows.h>
#include <TCHAR.h>
#include <string>
#include <memory.h>

inline void SleepS (unsigned int a_Time) { ::Sleep (a_Time*1000); }
inline void SleepMs (unsigned int a_Time) { ::Sleep (a_Time); }
#ifdef  _UNICODE
typedef std::wstring	String;
#endif

#ifndef _UNICODE
typedef std::string		String;
#endif

#define EXTERNAL_EXPORT		__declspec(dllexport)
#define EXTERNAL_IMPORT		__declspec(dllimport) 

#ifdef _DEBUG
#define __DEBUG__
#endif

#endif

#ifdef __LINUX__

#include <assert.h>
#include <string>

#define ESUCCESS             0
#define _Geterrno()          errno
#define BYTE                 unsigned char
#define __POSIX__            1
#define __UNIX__             1

#ifdef  _UNICODE
#define _T(x)      L ## x
#define TCHAR	   wchar_t
#endif

#ifndef _UNICODE
#define _T(x)      x
#define TCHAR	   char
#endif

#endif

#ifdef __DEC__

#include <assert.h>
#include <string>

#define socklen_t            int 
#define BYTE                 unsigned char
#define __POSIX__            1
#define __UNIX__             1

#ifdef  _UNICODE
#define _T(x)      L ## x
#define TCHAR	   wchar_t
#endif

#ifndef _UNICODE
#define _T(x)      x
#define TCHAR	   char
#endif

#endif

#include "Trace.h"

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned long      uint32;
typedef unsigned __int64   uint64;

typedef signed char        sint8;
typedef signed short       sint16;
typedef signed long        sint32;
typedef signed __int64     sint64;

#ifndef FALSE
#define FALSE                           (0)
#endif
                                                                                                           
#ifndef TRUE
#define TRUE                            (!FALSE)
#endif

#ifndef OK
#define OK								(0)
#endif

#ifndef INFINITE
#define INFINITE       (unsigned int) (-1)
#endif

#ifdef __WIN32__
#define SYSTEM_SYNC_HANDLE		HANDLE
#endif

#ifdef __POSIX__
#define SYSTEM_SYNC_HANDLE		HANDLE
#endif

template <unsigned int TYPE>
struct TemplateIntToType
{
	enum { value = TYPE };
};

void *__cdecl memrcpy(void * _Dst, const void * _Src, size_t _MaxCount);

// ---- Helper types and constants ----
#define _TXT(THETEXT) _T(THETEXT), (sizeof(THETEXT)/sizeof(TCHAR))-1

#define NUMBER_MAX_BITS(TYPE)		(sizeof (TYPE) << 3)
#define NUMBER_MIN_UNSIGNED(TYPE)	(static_cast <TYPE> (0))
#define NUMBER_MAX_UNSIGNED(TYPE)	(static_cast <TYPE> (~0))
#define NUMBER_MAX_SIGNED(TYPE)		(static_cast <TYPE> ((((static_cast<TYPE>(1) << (NUMBER_MAX_BITS(TYPE)-2)) -1) << 1 ) + 1 ))
#define NUMBER_MIN_SIGNED(TYPE)		(static_cast <TYPE> (-1-NUMBER_MAX_SIGNED(TYPE)))

typedef enum
{
	BASE_UNKNOWN		= 0,
	BASE_OCTAL			= 8,
	BASE_DECIMAL		= 10,
	BASE_HEXADECIMAL	= 16

}	NumberBase;

#endif // __PORTABILITY_H
