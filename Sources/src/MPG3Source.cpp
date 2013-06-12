#include "Source.h"
#include "MPG3Source.h"

namespace Solutions { namespace Source
{

// Ripped from id3ed V1.10.2
static const TCHAR* genre[] = {
	"Blues",	"Classic Rock",	"Country",		"Dance",		"Disco",		"Funk",
	"Grunge", 	"Hip-Hop", 	"Jazz", 		"Metal", 		"New Age", 		"Oldies",
	"Other", 	"Pop", 		"R&B", 			"Rap", 			"Reggae", 		"Rock",
	"Techno", 	"Industrial", 	"Alternative",		"Ska", 			"Death Metal",		"Pranks",
	"Soundtrack",	"Euro-Techno",	"Ambient",		"Trip-Hop", 		"Vocal", 		"Jazz+Funk",
	"Fusion", 	"Trance", 	"Classical",		"Instrumental",		"Acid", 		"House",
	"Game", 	"Sound Clip", 	"Gospel", 		"Noise", 		"AlternRock",		"Bass",
	"Soul", 	"Punk", 	"Space", 		"Meditative",		"Instrumental Pop",	"Instrumental Rock",
	"Ethnic", 	"Gothic", 	"Darkwave", 		"Techno-Industrial",	"Electronic", 		"Pop-Folk",
	"Eurodance",	"Dream", 	"Southern Rock",	"Comedy",		"Cult", 		"Gangsta",
	"Top 40", 	"Christian Rap","Pop/Funk", 		"Jungle", 		"Native American",	"Cabaret",
	"New Wave", 	"Psychadelic", 	"Rave", 		"Showtunes", 		"Trailer", 		"Lo-Fi",
	"Tribal", 	"Acid Punk", 	"Acid Jazz", 		"Polka", 		"Retro", 		"Musical",
	"Rock & Roll",	"Hard Rock", 	"Folk", 		"Folk/Rock",		"National Folk",	"Swing",
	"Fast-Fusion",	"Bebob", 	"Latin", 		"Revival",		"Celtic",		"Bluegrass",
	"Avantgarde",	"Gothic Rock", 	"Progressive Rock",	"Psychedelic Rock", 	"Symphonic Rock",	"Slow Rock",
	"Big Band",	"Chorus", 	"Easy Listening",	"Acoustic", 		"Humour", 		"Speech",
	"Chanson",	"Opera", 	"Chamber Music",	"Sonata", 		"Symphony", 		"Booty Bass",
	"Primus",	"Porn Groove", 	"Satire", 		"Slow Jam", 		"Club", 		"Tango",
	"Samba",	"Folklore", 	"Ballad", 		"Power Ballad",		"Rhythmic Soul", 	"Freestyle",
	"Duet",		"Punk Rock", 	"Drum Solo", 		"A capella", 		"Euro-House", 		"Dance Hall",
	"Goa", 		"Drum & Bass", 	"Club House", 		"Hardcore", 		"Terror",		"Indie",
	"BritPop", 	"NegerPunk", 	"Polsk Punk", 		"Beat",			"Christian Gangsta",	"Heavy Metal",
	"Black Metal",	"Crossover", 	"Contemporary C",	"Christian Rock",	"Merengue", 		"Salsa",
	"Thrash Metal",	"Anime", 	"JPop",			"SynthPop"
};

MPG3Source::MPG3Source (const Generics::DataElement& dataObject) : 
	m_Data(),
	m_Duration(0)
{
	if (dataObject.Size() > 4)
	{
		uint64 offset = SkipID3Tag(dataObject, 0);

		offset = MPEG::AudioHeader::FindTag (dataObject, offset);

		if (offset < dataObject.Size())
		{
			m_Data = MPEG::MPEGAudio(MPEG::MPEGStream(dataObject, offset, static_cast<uint64>(0), static_cast<uint64>(0)));
			MPEG::AudioHeader info = MPEG::AudioHeader (dataObject, offset);
			m_Duration = (((dataObject.Size() - offset) / info.Data().Size()) * info.FrameDuration()); // In milliSeconds
		}
	}
}

/* virtual */ MPG3Source::~MPG3Source()
{
}

uint64 MPG3Source::SkipID3Tag(const Generics::DataElement& data, const uint64 offset) const
{
	uint64 result = offset;
	const uint8* base = &(data.Buffer()[offset]);

	// Yip this is an ID3 Tag, skip it..
	if ( (base[0] == 'I') && (base[1] == 'D') && (base[2] == '3') )
	{
		result += (10 + ((base[6]&0x7F)<<21) + ((base[7]&0x7F)<<14) + ((base[8]&0x7F)<<7) + (base[9]&0x7F));
	}

	return (result);
}

} } // namespace Solutions::Source

