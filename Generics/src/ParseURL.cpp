/* Based on nsURLParsers.cc from Mozilla
 * -------------------------------------
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <stdlib.h>

#include "UniformResourceLocator.h"
#include "ParseURL.h"

namespace Solutions { namespace Generics
{

// Returns true if the given character is a valid digit to use in a port.
inline bool IsPortDigit(TCHAR ch) {
  return ch >= '0' && ch <= '9';
}

// Returns the offset of the next authority terminator in the input starting
// from start_offset. If no terminator is found, the return value will be equal
// to spec_len.
template<typename CHAR>
int FindNextAuthorityTerminator(const CHAR* spec,
                                int start_offset,
                                int spec_len) {
  for (int i = start_offset; i < spec_len; i++) {
    if (IsAuthorityTerminator(spec[i]))
      return i;
  }
  return spec_len;  // Not found.
}

// Fills in all members of the Parsed structure except for the scheme.
//
// |spec| is the full spec being parsed, of length |spec_len|.
// |after_scheme| is the character immediately following the scheme (after the
//   colon) where we'll begin parsing.
//
// Compatability data points. I list "host", "path" extracted:
// Input                IE6             Firefox                Us
// -----                --------------  --------------         --------------
// http://foo.com/      "foo.com", "/"  "foo.com", "/"         "foo.com", "/"
// http:foo.com/        "foo.com", "/"  "foo.com", "/"         "foo.com", "/"
// http:/foo.com/       fail(*)         "foo.com", "/"         "foo.com", "/"
// http:\foo.com/       fail(*)         "\foo.com", "/"(fail)  "foo.com", "/"
// http:////foo.com/    "foo.com", "/"  "foo.com", "/"         "foo.com", "/"
//
// (*) Interestingly, although IE fails to load these URLs, its history
// canonicalizer handles them, meaning if you've been to the corresponding
// "http://foo.com/" link, it will be colored.
template <typename CHAR>
void DoParseAfterScheme(const CHAR spec[],
                        int spec_len,
                        int after_scheme,
                        Parsed& parsed) {
  int num_slashes = CountConsecutiveSlashes(spec, after_scheme, spec_len);
  int after_slashes = after_scheme + num_slashes;

  // First split into two main parts, the authority (username, password, host,
  // and port) and the full path (path, query, and reference).
  Component authority;
  Component full_path;

  // Found "//<some data>", looks like an authority section. Treat everything
  // from there to the next slash (or end of spec) to be the authority. Note
  // that we ignore the number of slashes and treat it as the authority.
  int end_auth = FindNextAuthorityTerminator(spec, after_slashes, spec_len);
  authority = Component(after_slashes, end_auth - after_slashes);

  if (end_auth == spec_len)  // No beginning of path found.
    full_path = Component();
  else  // Everything starting from the slash to the end is the path.
    full_path = Component(end_auth, spec_len - end_auth);

  // Now parse those two sub-parts.
  DoParseAuthority(spec, authority, parsed.username, parsed.password,
                   parsed.host, parsed.port);
  ParsePath(spec, full_path, parsed.path, parsed.query, parsed.ref);
}

template<typename CHAR>
void ParseUserInfo(const CHAR* spec,
                   const Component& user,
                   Component& username,
                   Component& password) {
  // Find the first colon in the user section, which separates the username and
  // password.
  int colon_offset = 0;
  while (colon_offset < user._len && spec[user._begin + colon_offset] != ':')
    colon_offset++;

  if (colon_offset < user._len) {
    // Found separator: <username>:<password>
    username = Component(user._begin, colon_offset);
    password.SetRange(user._begin + colon_offset + 1,
                          user._begin + user._len);
  } else {
    // No separator, treat everything as the username
    username = user;
    password = Component();
  }
}

template<typename CHAR>
void ParseServerInfo(const CHAR* spec,
                     const Component& serverinfo,
                     Component& hostname,
                     Component& port_num) {
  if (serverinfo._len == 0) {
    // No server info, host name is empty.
    hostname.reset();
    port_num.reset();
    return;
  }

  // If the host starts with a left-bracket, assume the entire host is an
  // IPv6 literal.  Otherwise, assume none of the host is an IPv6 literal.
  // This assumption will be overridden if we find a right-bracket.
  //
  // Our IPv6 address canonicalization code requires both brackets to exist,
  // but the ability to locate an incomplete address can still be useful.
  int ipv6_terminator = spec[serverinfo._begin] == '[' ? serverinfo.end() : -1;
  int colon = -1;

  // Find the last right-bracket, and the last colon.
  for (int i = serverinfo._begin; i < serverinfo.end(); i++) {
    switch (spec[i]) {
      case ']':
        ipv6_terminator = i;
        break;
      case ':':
        colon = i;
        break;
    }
  }

  if (colon > ipv6_terminator) {
    // Found a port number: <hostname>:<port>
    hostname.SetRange(serverinfo._begin, colon);
    if (hostname._len == 0)
      hostname.reset();
    port_num.SetRange(colon + 1, serverinfo.end());
  } else {
    // No port: <hostname>
    hostname = serverinfo;
    port_num.reset();
  }
}

// Given an already-identified auth section, breaks it into its consituent
// parts. The port number will be parsed and the resulting integer will be
// filled into the given *port variable, or -1 if there is no port number or it
// is invalid.
template<typename CHAR>
void DoParseAuthority(const CHAR* spec,
                      const Component& auth,
                      Component& username,
                      Component& password,
                      Component& hostname,
                      Component& port_num) {
  ASSERT(auth.is_valid());
  if (auth._len == 0) {
    username.reset();
    password.reset();
    hostname.reset();
    port_num.reset();
    return;
  }

  // Search backwards for @, which is the separator between the user info and
  // the server info.
  int i = auth._begin + auth._len - 1;
  while (i > auth._begin && spec[i] != '@')
    i--;

  if (spec[i] == '@') {
    // Found user info: <user-info>@<server-info>
    ParseUserInfo(spec, Component(auth._begin, i - auth._begin),
                  username, password);
	Component tempItem; tempItem.SetRange(i + 1, auth._begin + auth._len);
	ParseServerInfo(spec, tempItem, hostname, port_num);
  } else {
    // No user info, everything is server info.
    username.reset();
    password.reset();
    ParseServerInfo(spec, auth, hostname, port_num);
  }
}

template<typename CHAR>
void ParsePath(const CHAR* spec,
               const Component& path,
               Component& filepath,
               Component& query,
               Component& ref) {
  // path = [/]<segment1>/<segment2>/<...>/<segmentN>;<param>?<query>#<ref>

  // Special case when there is no path.
  if (path._len == -1) {
    filepath.reset();
    query.reset();
    ref.reset();
    return;
  }
  ASSERT(path._len > 0);

  // Search for first occurrence of either ? or #.
  int path_end = path._begin + path._len;

  int query_separator = -1;  // Index of the '?'
  int ref_separator = -1;    // Index of the '#'
  for (int i = path._begin; i < path_end; i++) {
    switch (spec[i]) {
      case '?':
        // Only match the query string if it precedes the reference fragment
        // and when we haven't found one already.
        if (ref_separator < 0 && query_separator < 0)
          query_separator = i;
        break;
      case '#':
        // Record the first # sign only.
        if (ref_separator < 0)
          ref_separator = i;
        break;
    }
  }

  // Markers pointing to the character after each of these corresponding
  // components. The code below words from the end back to the beginning,
  // and will update these indices as it finds components that exist.
  int file_end, query_end;

  // Ref fragment: from the # to the end of the path.
  if (ref_separator >= 0) {
    file_end = query_end = ref_separator;
    ref.SetRange(ref_separator + 1, path_end);
  } else {
    file_end = query_end = path_end;
    ref.reset();
  }

  // Query fragment: everything from the ? to the next boundary (either the end
  // of the path or the ref fragment).
  if (query_separator >= 0) {
    file_end = query_separator;
    query.SetRange(query_separator + 1, query_end);
  } else {
    query.reset();
  }

  // File path: treat an empty file path as no file path.
  if (file_end != path._begin)
    filepath.SetRange(path._begin, file_end);
  else
    filepath.reset();
}

template<typename CHAR>
bool DoExtractScheme(const CHAR* url,
                     int url_len,
                     Component& scheme) {
  // Skip leading whitespace and control characters.
  int begin = 0;
  while (begin < url_len && ShouldTrimFromURL(url[begin]))
    begin++;
  if (begin == url_len)
    return false;  // Input is empty or all whitespace.

  // Find the first colon character.
  for (int i = begin; i < url_len; i++) {
    if (url[i] == ':') {
      scheme.SetRange(begin, i);
      return true;
    }
  }
  return false;  // No colon found: no scheme
}

// The main parsing function for standard URLs. Standard URLs have a scheme,
// host, path, etc.
template<typename CHAR>
void DoParseStandardURL(const CHAR* spec, int spec_len, Parsed& parsed) {
  ASSERT(spec_len >= 0);

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);

  int after_scheme;
  if (DoExtractScheme(spec, spec_len, parsed.scheme)) {
    after_scheme = parsed.scheme.end() + 1;  // Skip past the colon.
  } else {
    // Say there's no scheme when there is a colon. We could also say that
    // everything is the scheme. Both would produce an invalid URL, but this way
    // seems less wrong in more cases.
    parsed.scheme.reset();
    after_scheme = begin;
  }
  DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

// Initializes a path URL which is merely a scheme followed by a path. Examples
// include "about:foo" and "javascript:alert('bar');"
template<typename CHAR>
void DoParsePathURL(const CHAR* spec, int spec_len, Parsed& parsed) {
  // Get the non-path and non-scheme parts of the URL out of the way, we never
  // use them.
  parsed.username.reset();
  parsed.password.reset();
  parsed.host.reset();
  parsed.port.reset();
  parsed.query.reset();
  parsed.ref.reset();

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);

  // Handle empty specs or ones that contain only whitespace or control chars.
  if (begin == spec_len) {
    parsed.scheme.reset();
    parsed.path.reset();
    return;
  }

  // Extract the scheme, with the path being everything following. We also
  // handle the case where there is no scheme.
  if (ExtractScheme(&spec[begin], spec_len - begin, parsed.scheme)) {
    // Offset the results since we gave ExtractScheme a substring.
    parsed.scheme._begin += begin;

    // For compatability with the standard URL parser, we treat no path as
    // -1, rather than having a length of 0 (we normally wouldn't care so
    // much for these non-standard URLs).
    if (parsed.scheme.end() == spec_len - 1)
      parsed.path.reset();
    else
      parsed.path.SetRange(parsed.scheme.end() + 1, spec_len);
  } else {
    // No scheme found, just path.
    parsed.scheme.reset();
    parsed.path.SetRange(begin, spec_len);
  }
}

template<typename CHAR>
void DoParseMailtoURL(const CHAR spec[], int spec_len, Parsed& parsed) {
  ASSERT(spec_len >= 0);

  // Get the non-path and non-scheme parts of the URL out of the way, we never
  // use them.
  parsed.username.reset();
  parsed.password.reset();
  parsed.host.reset();
  parsed.port.reset();
  parsed.ref.reset();
  parsed.query.reset();  // May use this; reset for convenience.

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);

  // Handle empty specs or ones that contain only whitespace or control chars.
  if (begin == spec_len) {
    parsed.scheme.reset();
    parsed.path.reset();
    return;
  }

  int path_begin = -1;
  int path_end = -1;

  // Extract the scheme, with the path being everything following. We also
  // handle the case where there is no scheme.
  if (ExtractScheme(&spec[begin], spec_len - begin, parsed.scheme)) {
    // Offset the results since we gave ExtractScheme a substring.
    parsed.scheme._begin += begin;

    if (parsed.scheme.end() != spec_len - 1) {
      path_begin = parsed.scheme.end() + 1;
      path_end = spec_len;
    }
  } else {
    // No scheme found, just path.
    parsed.scheme.reset();
    path_begin = begin;
    path_end = spec_len;
  }

  // Split [path_begin, path_end) into a path + query.
  for (int i = path_begin; i < path_end; ++i) {
    if (spec[i] == '?') {
      parsed.query.SetRange(i + 1, path_end);
      path_end = i;
      break;
    }
  }

  // For compatability with the standard URL parser, treat no path as
  // -1, rather than having a length of 0
  if (path_begin == path_end) {
    parsed.path.reset();
  } else {
    parsed.path.SetRange(path_begin, path_end);
  }
}

// Converts a port number in a string to an integer. We'd like to just call
// sscanf but our input is not NULL-terminated, which sscanf requires. Instead,
// we copy the digits to a small stack buffer (since we know the maximum number
// of digits in a valid port number) that we can NULL terminate.
template<typename CHAR>
int DoParsePort(const CHAR spec[], const Component& component) {
  // Easy success case when there is no port.
  const int kMaxDigits = 5;
  if (!component.is_nonempty())
    return PORT_UNSPECIFIED;

  // Skip over any leading 0s.
  Component digits_comp(component.end(), 0);
  for (int i = 0; i < component._len; i++) {
    if (spec[component._begin + i] != '0') {
      digits_comp.SetRange(component._begin + i, component.end());
      break;
    }
  }
  if (digits_comp._len == 0)
    return 0;  // All digits were 0.

  // Verify we don't have too many digits (we'll be copying to our buffer so
  // we need to double-check).
  if (digits_comp._len > kMaxDigits)
    return PORT_INVALID;

  // Copy valid digits to the buffer.
  char digits[kMaxDigits + 1];  // +1 for null terminator
  for (int i = 0; i < digits_comp._len; i++) {
    CHAR ch = spec[digits_comp._begin + i];
    if (!IsPortDigit(ch)) {
      // Invalid port digit, fail.
      return PORT_INVALID;
    }
    digits[i] = static_cast<char>(ch);
  }

  // Null-terminate the string and convert to integer. Since we guarantee
  // only digits, atoi's lack of error handling is OK.
  digits[digits_comp._len] = 0;
  int port = atoi(digits);
  if (port > 65535)
    return PORT_INVALID;  // Out of range.
  return port;
}

template<typename CHAR>
void DoExtractFileName(const CHAR spec[],
                       const Component& path,
                       Component& file_name) {
  // Handle empty paths: they have no file names.
  if (!path.is_nonempty()) {
    file_name.reset();
    return;
  }

  // Search backwards for a parameter, which is a normally unused field in a
  // URL delimited by a semicolon. We parse the parameter as part of the
  // path, but here, we don't want to count it. The last semicolon is the
  // parameter. The path should start with a slash, so we don't need to check
  // the first one.
  int file_end = path.end();
  for (int i = path.end() - 1; i > path.begin(); i--) {
    if (spec[i] == ';') {
      file_end = i;
      break;
    }
  }

  // Now search backwards from the filename end to the previous slash
  // to find the beginning of the filename.
  for (int i = file_end - 1; i >= path.begin(); i--) {
    if (IsURLSlash(spec[i])) {
      // File name is everything following this character to the end
      file_name.SetRange(i + 1, file_end);
      return;
    }
  }

  // No slash found, this means the input was degenerate (generally paths
  // will start with a slash). Let's call everything the file name.
  file_name.SetRange(path.begin(), file_end);
  return;
}

template<typename CHAR>
bool DoExtractQueryKeyValue(const CHAR spec[],
                            Component& query,
                            Component& key,
                            Component& value) 
{
  if (!query.is_nonempty())
    return false;

  int start = query.begin();
  int cur = start;
  int end = query.end();

  // We assume the beginning of the input is the beginning of the "key" and we
  // skip to the end of it.
  key._begin = cur;
  while (cur < end && spec[cur] != '&' && spec[cur] != '=')
    cur++;
  key._len = cur - key.begin();

  // Skip the separator after the key (if any).
  if (cur < end && spec[cur] == '=')
    cur++;

  // Find the value part.
  value._begin = cur;
  while (cur < end && spec[cur] != '&')
    cur++;
  value._len = cur - value.begin();

  // Finally skip the next separator if any
  if (cur < end && spec[cur] == '&')
    cur++;

  // Save the new query
  query.SetRange(cur, end);
  return true;
}

int Parsed::Length() const 
{
  if (ref.is_valid())
    return ref.end();
  return CountCharactersBefore(REF, false);
}

int Parsed::CountCharactersBefore(ComponentType type,
                                  bool include_delimiter) const 
{
  if (type == SCHEME)
    return scheme.begin();

  // There will be some characters after the scheme like "://" and we don't
  // know how many. Search forwards for the next thing until we find one.
  int cur = 0;
  if (scheme.is_valid())
    cur = scheme.end() + 1;  // Advance over the ':' at the end of the scheme.

  if (username.is_valid()) {
    if (type <= USERNAME)
      return username.begin();
    cur = username.end() + 1;  // Advance over the '@' or ':' at the end.
  }

  if (password.is_valid()) {
    if (type <= PASSWORD)
      return password.begin();
    cur = password.end() + 1;  // Advance over the '@' at the end.
  }

  if (host.is_valid()) {
    if (type <= HOST)
      return host.begin();
    cur = host.end();
  }

  if (port.is_valid()) {
    if (type < PORT || (type == PORT && include_delimiter))
      return port.begin() - 1;  // Back over delimiter.
    if (type == PORT)
      return port.begin();  // Don't want delimiter counted.
    cur = port.end();
  }

  if (path.is_valid()) {
    if (type <= PATH)
      return path.begin();
    cur = path.end();
  }

  if (query.is_valid()) {
    if (type < QUERY || (type == QUERY && include_delimiter))
      return query.begin() - 1;  // Back over delimiter.
    if (type == QUERY)
      return query.begin();  // Don't want delimiter counted.
    cur = query.end();
  }

  if (ref.is_valid()) {
    if (type == REF && !include_delimiter)
      return ref.begin();  // Back over delimiter.

    // When there is a ref and we get here, the component we wanted was before
    // this and not found, so we always know the beginning of the ref is right.
    return ref.begin() - 1;  // Don't want delimiter counted.
  }

  return cur;
}

bool ExtractScheme(const TCHAR url[], int url_len, Component& scheme) 
{
  return DoExtractScheme(url, url_len, scheme);
}

// This handles everything that may be an authority terminator, including
// backslash. For special backslash handling see DoParseAfterScheme.
bool IsAuthorityTerminator(TCHAR ch) 
{
  return IsURLSlash(ch) || ch == '?' || ch == '#' || ch == ';';
}

void ExtractFileName(const TCHAR url[],
                     const Component& path,
                     Component& file_name) 
{
  DoExtractFileName(url, path, file_name);
}

bool ExtractQueryKeyValue(const TCHAR url[],
                          Component& query,
                          Component& key,
                          Component& value) 
{
  return DoExtractQueryKeyValue(url, query, key, value);
}

void ParseAuthority(const TCHAR spec[],
                    const Component& auth,
                    Component& username,
                    Component& password,
                    Component& hostname,
                    Component& port_num) 
{
  DoParseAuthority(spec, auth, username, password, hostname, port_num);
}

int ParsePort(const TCHAR url[], const Component& port) 
{
  return DoParsePort(url, port);
}

void ParsePathInternal(const TCHAR spec[],
                       const Component& path,
                       Component& filepath,
                       Component& query,
                       Component& ref) 
{
  ParsePath(spec, path, filepath, query, ref);
}

void ParseAfterScheme(const TCHAR spec[],
                      int spec_len,
                      int after_scheme,
                      Parsed& parsed) 
{
  DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

// Interesting IE file:isms...
//
//  INPUT                      OUTPUT
//  =========================  ==============================
//  file:/foo/bar              file:///foo/bar
//      The result here seems totally invalid!?!? This isn't UNC.
//
//  file:/
//  file:// or any other number of slashes
//      IE6 doesn't do anything at all if you click on this link. No error:
//      nothing. IE6's history system seems to always color this link, so I'm
//      guessing that it maps internally to the empty URL.
//
//  C:\                        file:///C:/
//      When on a file: URL source page, this link will work. When over HTTP,
//      the file: URL will appear in the status bar but the link will not work
//      (security restriction for all file URLs).
//
//  file:foo/                  file:foo/     (invalid?!?!?)
//  file:/foo/                 file:///foo/  (invalid?!?!?)
//  file://foo/                file://foo/   (UNC to server "foo")
//  file:///foo/               file:///foo/  (invalid, seems to be a file)
//  file:////foo/              file://foo/   (UNC to server "foo")
//      Any more than four slashes is also treated as UNC.
//
//  file:C:/                   file://C:/
//  file:/C:/                  file://C:/
//      The number of slashes after "file:" don't matter if the thing following
//      it looks like an absolute drive path. Also, slashes and backslashes are
//      equally valid here.

// A subcomponent of DoInitFileURL, the input of this function should be a UNC
// path name, with the index of the first character after the slashes following
// the scheme given in |after_slashes|. This will initialize the host, path,
// query, and ref, and leave the other output components untouched
// (DoInitFileURL handles these for us).
template<typename CHAR>
void DoParseUNC(const CHAR* spec,
                int after_slashes,
                int spec_len,
               Parsed& parsed) {
  int next_slash = FindNextSlash(spec, after_slashes, spec_len);
  if (next_slash == spec_len) {
    // No additional slash found, as in "file://foo", treat the text as the
    // host with no path (this will end up being UNC to server "foo").
    int host_len = spec_len - after_slashes;
    if (host_len)
      parsed.host = Component(after_slashes, host_len);
    else
      parsed.host.reset();
    parsed.path.reset();
    return;
  }

#ifdef WIN32
  // See if we have something that looks like a path following the first
  // component. As in "file://localhost/c:/", we get "c:/" out. We want to
  // treat this as a having no host but the path given. Works on Windows only.
  if (DoesBeginWindowsDriveSpec(spec, next_slash + 1, spec_len)) {
    parsed.host.reset();
	Component tempItem; tempItem.SetRange(next_slash, spec_len);
    ParsePathInternal(spec, tempItem, parsed.path, parsed.query, parsed.ref);
    return;
  }
#endif

  // Otherwise, everything up until that first slash we found is the host name,
  // which will end up being the UNC host. For example "file://foo/bar.txt"
  // will get a server name of "foo" and a path of "/bar". Later, on Windows,
  // this should be treated as the filename "\\foo\bar.txt" in proper UNC
  // notation.
  int host_len = next_slash - after_slashes;
  if (host_len)
    parsed.host.SetRange(after_slashes, next_slash);
  else
    parsed.host.reset();
  if (next_slash < spec_len) {
	Component tempItem; tempItem.SetRange(next_slash, spec_len);
    ParsePathInternal(spec, tempItem,
                      parsed.path, parsed.query, parsed.ref);
  } else {
    parsed.path.reset();
  }
}

// A subcomponent of DoParseFileURL, the input should be a local file, with the
// beginning of the path indicated by the index in |path_begin|. This will
// initialize the host, path, query, and ref, and leave the other output
// components untouched (DoInitFileURL handles these for us).
template<typename CHAR>
void DoParseLocalFile(const CHAR* spec,
                      int path_begin,
                      int spec_len,
                      Parsed& parsed) {
  parsed.host.reset();
  Component tempItem; tempItem.SetRange(path_begin, spec_len);
  ParsePathInternal(spec, tempItem,
                    parsed.path, parsed.query, parsed.ref);
}

// Backend for the external functions that operates on either char type.
// We are handed the character after the "file:" at the beginning of the spec.
// Usually this is a slash, but needn't be; we allow paths like "file:c:\foo".
template<typename CHAR>
void DoParseFileURL(const CHAR* spec, int spec_len, Parsed& parsed) {
  ASSERT(spec_len >= 0);

  // Get the parts we never use for file URLs out of the way.
  parsed.username.reset();
  parsed.password.reset();
  parsed.port.reset();

  // Many of the code paths don't set these, so it's convenient to just clear
  // them. We'll write them in those cases we need them.
  parsed.query.reset();
  parsed.ref.reset();

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);

  // Find the scheme.
  int num_slashes;
  int after_scheme;
  int after_slashes;
#ifdef WIN32
  // See how many slashes there are. We want to handle cases like UNC but also
  // "/c:/foo". This is when there is no scheme, so we can allow pages to do
  // links like "c:/foo/bar" or "//foo/bar". This is also called by the
  // relative URL resolver when it determines there is an absolute URL, which
  // may give us input like "/c:/foo".
  num_slashes = CountConsecutiveSlashes(spec, begin, spec_len);
  after_slashes = begin + num_slashes;
  if (DoesBeginWindowsDriveSpec(spec, after_slashes, spec_len)) {
    // Windows path, don't try to extract the scheme (for example, "c:\foo").
    parsed.scheme.reset();
    after_scheme = after_slashes;
  } else if (DoesBeginUNCPath(spec, begin, spec_len, false)) {
    // Windows UNC path: don't try to extract the scheme, but keep the slashes.
    parsed.scheme.reset();
    after_scheme = begin;
  } else
#endif
  {
    if (ExtractScheme(&spec[begin], spec_len - begin, parsed.scheme)) {
      // Offset the results since we gave ExtractScheme a substring.
      parsed.scheme._begin += begin;
      after_scheme = parsed.scheme.end() + 1;
    } else {
      // No scheme found, remember that.
      parsed.scheme.reset();
      after_scheme = begin;
    }
  }

  // Handle empty specs ones that contain only whitespace or control chars,
  // or that are just the scheme (for example "file:").
  if (after_scheme == spec_len) {
    parsed.host.reset();
    parsed.path.reset();
    return;
  }

  num_slashes = CountConsecutiveSlashes(spec, after_scheme, spec_len);

  after_slashes = after_scheme + num_slashes;
#ifdef WIN32
  // Check whether the input is a drive again. We checked above for windows
  // drive specs, but that's only at the very beginning to see if we have a
  // scheme at all. This test will be duplicated in that case, but will
  // additionally handle all cases with a real scheme such as "file:///C:/".
  if (!DoesBeginWindowsDriveSpec(spec, after_slashes, spec_len) &&
      num_slashes != 3) {
    // Anything not beginning with a drive spec ("c:\") on Windows is treated
    // as UNC, with the exception of three slashes which always means a file.
    // Even IE7 treats file:///foo/bar as "/foo/bar", which then fails.
    DoParseUNC(spec, after_slashes, spec_len, parsed);
    return;
  }
#else
  // file: URL with exactly 2 slashes is considered to have a host component.
  if (num_slashes == 2) {
    DoParseUNC(spec, after_slashes, spec_len, parsed);
    return;
  }
#endif  // WIN32

  // Easy and common case, the full path immediately follows the scheme
  // (modulo slashes), as in "file://c:/foo". Just treat everything from
  // there to the end as the path. Empty hosts have 0 length instead of -1.
  // We include the last slash as part of the path if there is one.
  DoParseLocalFile(spec,
      num_slashes > 0 ? after_scheme + num_slashes - 1 : after_scheme,
      spec_len, parsed);
}

void ParseStandardURL(const TCHAR url[], int url_len, Parsed& parsed) 
{
  DoParseStandardURL(url, url_len, parsed);
}

void ParsePathURL(const TCHAR url[], int url_len, Parsed& parsed) 
{
  DoParsePathURL(url, url_len, parsed);
}

void ParseMailtoURL(const TCHAR url[], int url_len, Parsed& parsed) 
{
  DoParseMailtoURL(url, url_len, parsed);
}

void ParseFileURL(const TCHAR url[], int url_len, Parsed& parsed) 
{
  DoParseFileURL(url, url_len, parsed);
}

} } // namespace Solutions::Generics
