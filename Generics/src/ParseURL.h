// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef URL_PARSE_H__
#define URL_PARSE_H__

#include "Module.h"

// Component ------------------------------------------------------------------

// Represents a substring for URL parsing.
namespace Solutions { namespace Generics
{
	struct Component {
	  Component() : _begin(0), _len(-1) {}

	  // Normal constructor: takes an offset and a length.
	  Component(int b, int l) : _begin(b), _len(l) {}

	  int length() const {
		  return (_len);
	  }

	  int end() const {
		return _begin + _len;
	  }

	  int begin() const {
		return _begin;
	  }
	  // Helper that returns a component created with the given begin and ending
	  // points. The ending point is non-inclusive.
	  inline void SetRange(int start, int stop) {
		  _begin = start;
		  _len = stop - start;
	  }

	  // Returns true if this component is valid, meaning the length is given. Even
	  // valid components may be empty to record the fact that they exist.
	  bool is_valid() const {
		return (_len != -1);
	  }

	  // Returns true if the given component is specified on false, the component
	  // is either empty or invalid.
	  bool is_nonempty() const {
		return (_len > 0);
	  }

	  void reset() {
		_begin = 0;
		_len = -1;
	  }

	  bool operator==(const Component& other) const {
		return _begin == other._begin && _len == other._len;
	  }

	  int _begin;  // Byte offset in the string of this component.
	  int _len;    // Will be -1 if the component is unspecified.
	};

	// Parsed ---------------------------------------------------------------------

	// A structure that holds the identified parts of an input URL. This structure
	// does NOT store the URL itself. The caller will have to store the URL text
	// and its corresponding Parsed structure separately.
	//
	// Typical usage would be:
	//
	//    url_parse::Parsed parsed;
	//    url_parse::Component scheme;
	//    if (!url_parse::ExtractScheme(url, url_len, &scheme))
	//      return I_CAN_NOT_FIND_THE_SCHEME_DUDE;
	//
	//    if (IsStandardScheme(url, scheme))  // Not provided by this component
	//      url_parseParseStandardURL(url, url_len, &parsed);
	//    else if (IsFileURL(url, scheme))    // Not provided by this component
	//      url_parse::ParseFileURL(url, url_len, &parsed);
	//    else
	//      url_parse::ParsePathURL(url, url_len, &parsed);
	//
	struct Parsed {
	  // Identifies different components.
	  enum ComponentType {
		SCHEME,
		USERNAME,
		PASSWORD,
		HOST,
		PORT,
		PATH,
		QUERY,
		REF,
	  };

	  // The default constructor is sufficient for the components.
	  Parsed() {}

	  // Returns the length of the URL (the end of the last component).
	  //
	  // Note that for some invalid, non-canonical URLs, this may not be the length
	  // of the string. For example "http://": the parsed structure will only
	  // contain an entry for the four-character scheme, and it doesn't know about
	  // the "://". For all other last-components, it will return the real length.
	  int Length() const;

	  // Returns the number of characters before the given component if it exists,
	  // or where the component would be if it did exist. This will return the
	  // string length if the component would be appended to the end.
	  //
	  // Note that this can get a little funny for the port, query, and ref
	  // components which have a delimiter that is not counted as part of the
	  // component. The |include_delimiter| flag controls if you want this counted
	  // as part of the component or not when the component exists.
	  //
	  // This example shows the difference between the two flags for two of these
	  // delimited components that is present (the port and query) and one that
	  // isn't (the reference). The components that this flag affects are marked
	  // with a *.
	  //                 0         1         2
	  //                 012345678901234567890
	  // Example input:  http://foo:80/?query
	  //              include_delim=true,  ...=false  ("<-" indicates different)
	  //      SCHEME: 0                    0
	  //    USERNAME: 5                    5
	  //    PASSWORD: 5                    5
	  //        HOST: 7                    7
	  //       *PORT: 10                   11 <-
	  //        PATH: 13                   13
	  //      *QUERY: 14                   15 <-
	  //        *REF: 20                   20
	  //
	  int CountCharactersBefore(ComponentType type, bool include_delimiter) const;

	  // Scheme without the colon: "http://foo"/ would have a scheme of "http".
	  // The length will be -1 if no scheme is specified ("foo.com"), or 0 if there
	  // is a colon but no scheme (":foo"). Note that the scheme is not guaranteed
	  // to start at the beginning of the string if there are preceeding whitespace
	  // or control characters.
	  Component scheme;

	  // Username. Specified in URLs with an @ sign before the host. See |password|
	  Component username;

	  // Password. The length will be -1 if unspecified, 0 if specified but empty.
	  // Not all URLs with a username have a password, as in "http://me@host/".
	  // The password is separated form the username with a colon, as in
	  // "http://me:secret@host/"
	  Component password;

	  // Host name.
	  Component host;

	  // Port number.
	  Component port;

	  // Path, this is everything following the host name. Length will be -1 if
	  // unspecified. This includes the preceeding slash, so the path on
	  // http://www.google.com/asdf" is "/asdf". As a result, it is impossible to
	  // have a 0 length path, it will be -1 in cases like "http://host?foo".
	  // Note that we treat backslashes the same as slashes.
	  Component path;

	  // Stuff between the ? and the # after the path. This does not include the
	  // preceeding ? character. Length will be -1 if unspecified, 0 if there is
	  // a question mark but no query string.
	  Component query;

	  // Indicated by a #, this is everything following the hash sign (not
	  // including it). If there are multiple hash signs, we'll use the last one.
	  // Length will be -1 if there is no hash sign, or 0 if there is one but
	  // nothing follows it.
	  Component ref;
	};

	// Initialization functions ---------------------------------------------------
	//
	// These functions parse the given URL, filling in all of the structure's
	// components. These functions can not fail, they will always do their best
	// at interpreting the input given.
	//
	// The string length of the URL MUST be specified, we do not check for NULLs
	// at any point in the process, and will actually handle embedded NULLs.
	//
	// IMPORTANT: These functions do NOT hang on to the given pointer or copy it
	// in any way. See the comment above the struct.
	//
	// The 8-bit versions require UTF-8 encoding.

	// StandardURL is for when the scheme is known to be one that has an
	// authority (host) like "http". This function will not handle weird ones
	// like "about:" and "javascript:", or do the right thing for "file:" URLs.
	void ParseStandardURL(const TCHAR url[], int url_len, Parsed& parsed);

	// PathURL is for when the scheme is known not to have an authority (host)
	// section but that aren't file URLs either. The scheme is parsed, and
	// everything after the scheme is considered as the path. This is used for
	// things like "about:" and "javascript:"
	void ParsePathURL(const TCHAR url[], int url_len, Parsed& parsed);

	// FileURL is for file URLs. There are some special rules for interpreting
	// these.
	void ParseFileURL(const TCHAR url[], int url_len, Parsed& parsed);

	// MailtoURL is for mailto: urls. They are made up scheme,path,query
	void ParseMailtoURL(const TCHAR url[], int url_len, Parsed& parsed);

	// Helper functions -----------------------------------------------------------

	// Locates the scheme according to the URL  parser's rules. This function is
	// designed so the caller can find the scheme and call the correct Init*
	// function according to their known scheme types.
	//
	// It also does not perform any validation on the scheme.
	//
	// This function will return true if the scheme is found and will put the
	// scheme's range into *scheme. False means no scheme could be found. Note
	// that a URL beginning with a colon has a scheme, but it is empty, so this
	// function will return true but *scheme will = (0,0).
	//
	// The scheme is found by skipping spaces and control characters at the
	// beginning, and taking everything from there to the first colon to be the
	// scheme. The character at scheme.end() will be the colon (we may enhance
	// this to handle full width colons or something, so don't count on the
	// actual character value). The character at scheme.end()+1 will be the
	// beginning of the rest of the URL, be it the authority or the path (or the
	// end of the string).
	//
	// The 8-bit version requires UTF-8 encoding.
	bool ExtractScheme(const TCHAR url[], int url_len, Component& scheme);

	// Returns true if ch is a character that terminates the authority segment
	// of a URL.
	bool IsAuthorityTerminator(TCHAR ch);

	// Does a best effort parse of input |spec|, in range |auth|. If a particular
	// component is not found, it will be set to invalid.
	void ParseAuthority(const TCHAR spec[],
						const Component& auth,
						Component& username,
						Component& password,
						Component& hostname,
						Component& port_num);

	// Computes the integer port value from the given port component. The port
	// component should have been identified by one of the init functions on
	// |Parsed| for the given input url.
	//
	// The return value will be a positive integer between 0 and 64K, or one of
	// the two special values below.
	enum SpecialPort { PORT_UNSPECIFIED = -1, PORT_INVALID = -2 };
	int ParsePort(const TCHAR url[], const Component& port);

	// Extracts the range of the file name in the given url. The path must
	// already have been computed by the parse function, and the matching URL
	// and extracted path are provided to this function. The filename is
	// defined as being everything from the last slash/backslash of the path
	// to the end of the path.
	//
	// The file name will be empty if the path is empty or there is nothing
	// following the last slash.
	//
	// The 8-bit version requires UTF-8 encoding.
	void ExtractFileName(const TCHAR url[],
						 const Component& path,
						 Component& file_name);

	// Extract the first key/value from the range defined by |*query|. Updates
	// |*query| to start at the end of the extracted key/value pair. This is
	// designed for use in a loop: you can keep calling it with the same query
	// object and it will iterate over all items in the query.
	//
	// Some key/value pairs may have the key, the value, or both be empty (for
	// example, the query string "?&"). These will be returned. Note that an empty
	// last parameter "foo.com?" or foo.com?a&" will not be returned, this case
	// is the same as "done."
	//
	// The initial query component should not include the '?' (this is the default
	// for parsed URLs).
	//
	// If no key/value are found |*key| and |*value| will be unchanged and it will
	// return false.
	bool ExtractQueryKeyValue(const TCHAR url[],
							  Component& query,
							  Component& key,
							  Component& value);

	// We treat slashes and backslashes the same for IE compatability.
	inline bool IsURLSlash(TCHAR ch) {
	  return ch == '/' || ch == '\\';
	}

	// Returns true if we should trim this character from the URL because it is a
	// space or a control character.
	inline bool ShouldTrimFromURL(TCHAR ch) {
	  return ch <= ' ';
	}

	// Given an already-initialized begin index and length, this shrinks the range
	// to eliminate "should-be-trimmed" characters. Note that the length does *not*
	// indicate the length of untrimmed data from |*begin|, but rather the position
	// in the input string (so the string starts at character |*begin| in the spec,
	// and goes until |*len|).
	template<typename CHAR>
	inline void TrimURL(const CHAR* spec, int* begin, int* len) {
	  // Strip leading whitespace and control characters.
	  while (*begin < *len && ShouldTrimFromURL(spec[*begin]))
		(*begin)++;

	  // Strip trailing whitespace and control characters. We need the >i test for
	  // when the input string is all blanks; we don't want to back past the input.
	  while (*len > *begin && ShouldTrimFromURL(spec[*len - 1]))
		(*len)--;
	}

	// Counts the number of consecutive slashes starting at the given offset
	// in the given string of the given length.
	template<typename CHAR>
	inline int CountConsecutiveSlashes(const CHAR *str,
									   int begin_offset, int str_len) {
	  int count = 0;
	  while (begin_offset + count < str_len &&
			 IsURLSlash(str[begin_offset + count]))
		++count;
	  return count;
	}

	// Internal functions in url_parse.cc that parse the path, that is, everything
	// following the authority section. The input is the range of everything
	// following the authority section, and the output is the identified ranges.
	//
	// This is designed for the file URL parser or other consumers who may do
	// special stuff at the beginning, but want regular path parsing, it just
	// maps to the internal parsing function for paths.
	void ParsePathInternal(const TCHAR spec[],
						   const Component& path,
						   Component& filepath,
						   Component& query,
						   Component& ref);

	// Given a spec and a pointer to the character after the colon following the
	// scheme, this parses it and fills in the structure, Every item in the parsed
	// structure is filled EXCEPT for the scheme, which is untouched.
	void ParseAfterScheme(const TCHAR spec[],
						  int spec_len,
						  int after_scheme,
						  Parsed& parsed);


	#ifdef WIN32

	// We allow both "c:" and "c|" as drive identifiers.
	inline bool IsWindowsDriveSeparator(TCHAR ch) {
	  return ch == ':' || ch == '|';
	}
	inline bool IsWindowsDriveLetter(TCHAR ch) {
	  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
	}

	#endif  // WIN32

	// Returns the index of the next slash in the input after the given index, or
	// spec_len if the end of the input is reached.
	template<typename CHAR>
	inline int FindNextSlash(const CHAR* spec, int begin_index, int spec_len) {
	  int idx = begin_index;
	  while (idx < spec_len && !IsURLSlash(spec[idx]))
		idx++;
	  return idx;
	}

	#ifdef WIN32

	// Returns true if the start_offset in the given spec looks like it begins a
	// drive spec, for example "c:". This function explicitly handles start_offset
	// values that are equal to or larger than the spec_len to simplify callers.
	//
	// If this returns true, the spec is guaranteed to have a valid drive letter
	// plus a colon starting at |start_offset|.
	template<typename CHAR>
	inline bool DoesBeginWindowsDriveSpec(const CHAR* spec, int start_offset,
										  int spec_len) {
	  int remaining_len = spec_len - start_offset;
	  if (remaining_len < 2)
		return false;  // Not enough room.
	  if (!IsWindowsDriveLetter(spec[start_offset]))
		return false;  // Doesn't start with a valid drive letter.
	  if (!IsWindowsDriveSeparator(spec[start_offset + 1]))
		return false;  // Isn't followed with a drive separator.
	  return true;
	}

	// Returns true if the start_offset in the given text looks like it begins a
	// UNC path, for example "\\". This function explicitly handles start_offset
	// values that are equal to or larger than the spec_len to simplify callers.
	//
	// When strict_slashes is set, this function will only accept backslashes as is
	// standard for Windows. Otherwise, it will accept forward slashes as well
	// which we use for a lot of URL handling.
	template<typename CHAR>
	inline bool DoesBeginUNCPath(const CHAR* text,
								 int start_offset,
								 int len,
								 bool strict_slashes) {
	  int remaining_len = len - start_offset;
	  if (remaining_len < 2)
		return false;

	  if (strict_slashes)
		return text[start_offset] == '\\' && text[start_offset + 1] == '\\';
	  return IsURLSlash(text[start_offset]) && IsURLSlash(text[start_offset + 1]);
	}

	#endif  // WIN32

} } // namespace Solutions::Generics

#endif // URL_PARSE_H__
