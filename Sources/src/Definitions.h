#ifndef __STREAMERDEFINITIONS_H
#define __STREAMERDEFINITIONS_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "Source.h"
#include "CachedSource.h"
#include "SourceFile.h"
#include "SourceCollator.h"
#include "MPG12Source.h"
#include "AVIMediaSource.h" 
#include "MPG3Source.h"

// ---- Referenced classes and types ----

// ---- Helper functions ----
// -------------------------------------------------------------------------------
// Here we create a typedef for all possible source streamers
// -------------------------------------------------------------------------------
namespace Solutions { namespace Source
{
	EXTERNAL extern const TCHAR FILETYPE_MP3[];
	EXTERNAL extern const TCHAR FILETYPE_MPG[];
	EXTERNAL extern const TCHAR FILETYPE_MPEG[];
	EXTERNAL extern const TCHAR FILETYPE_TS[];
	EXTERNAL extern const TCHAR FILETYPE_AVI[];

	typedef SingleSourceType<FileType<FILETYPE_MP3>,    MPG3Source,  CollatorType<MPGAudioIterator,    MULTIPLE_FRAMES|REJECT_OVERSIZED_FRAMES>>	FileMP3Audio;
	typedef MultipleSourceType<FileType<FILETYPE_MPG>,  MPG12Source, CollatorType<MPGIterator,         MULTIPLE_FRAMES|REJECT_OVERSIZED_FRAMES>>	FileMPGMedia;
	typedef MultipleSourceType<FileType<FILETYPE_MPEG>, MPG12Source, CollatorType<MPGIterator,         MULTIPLE_FRAMES|REJECT_OVERSIZED_FRAMES>>	FileMPEGMedia;
	typedef MultipleSourceType<FileType<FILETYPE_TS>,   MPG12Source, CollatorType<MPGIterator,         MULTIPLE_FRAMES|REJECT_OVERSIZED_FRAMES>>	FileTSMedia;
	typedef MultipleSourceType<FileType<FILETYPE_AVI>,  AVIMedia,    CollatorType<AVIMedia::AVIStream, MULTIPLE_FRAMES|REJECT_OVERSIZED_FRAMES>>	FileAVIMedia;

	// Just to check compile timeif this also works
	// typedef CachedStreamSourceType<SourceFileType<FILETYPE_MP3>, MPEGAudioStreamer, 300>														FileMP3Audio;
	typedef SingleSourceType<FileType<FILETYPE_MP3>, MPG12Source, SourceType<MPG12Source>>														_FileMP3Audio;
	typedef MultipleSourceType<FileType<FILETYPE_AVI>, AVIMedia, SourceType<AVIMedia>>															_FileAVIMedia;

} } // namespace Solutions::Source

#endif // __STREAMERDEFINITIONS_H

