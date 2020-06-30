/*
 *  SHSoundfile.h
 *  shapers-vst-2007
 *
 *  Created by Tom Erbe on 9/9/10.
 *  Copyright 2010 SoundHack. All rights reserved.
 *
 */
#include <stdint.h> // this won't work on windows...
#include <sys/types.h>

/*------------------------------------------------------------------*/
/*	TRESoundfile.h												*/
/*	Copyright 1999 Tom Erbe, All Rights Reserved.		*/
/*------------------------------------------------------------------*/
#define EVENUP(n) if (n & 1) n++
#define E_OK 0
#define SwapTwoBytes(data) ( (((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00) )
#define SwapFourBytes(data)	( (((data) >> 24) & 0x000000FF) | (((data) >> 8) & 0x0000FF00) | (((data) << 8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) )
#define SwapEightBytes(data)	\ ( (((data) >> 56) & 0x00000000000000FF) | (((data) >> 40) & 0x000000000000FF00) | \
(((data) >> 24) & 0x0000000000FF0000) | (((data) >> 8) & 0x00000000FF000000) | \
(((data) << 8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
(((data) << 40) & 0x00FF000000000000) | (((data) << 56) & 0xFF00000000000000) )

typedef struct
{
	char	ckID[4];
	int32_t	ckSize;
}	chunkHeader;
	
	// BEGIN (class declaration)
class SHSoundFile
{
public:
	// insert public methods and data here
		SHSoundFile();
		~SHSoundFile();
		int32_t	Open(char *fileName);
		void	Close();
		
		int32_t	ReadHeader();
		int32_t	WriteHeader();
		int32_t	CreateHeader();
		
		int32_t	ReadWaveHeader();
		int32_t	CreateWaveHeader();
		int32_t	WriteWaveHeader();
		
//		int32_t 	ReadSDIIHeader();
		int32_t 	CreateSDIIHeader();
		int32_t 	WriteSDIIHeader();
		
		int32_t	ReadAIFFHeader();
		int32_t	CreateAIFFHeader();
		int32_t	WriteAIFFHeader();
		
		int32_t	ReadAIFCHeader();
		int32_t	CreateAIFCHeader();
		int32_t	WriteAIFCHeader();
		
		int32_t	ReadSunHeader();
		int32_t	CreateSunHeader();
		int32_t	WriteSunHeader();
		
		void	FreeDiskMemory(void);
        void    SetPosition(float fraction);
		size_t	InitDiskBlock(size_t frameCount);
		size_t	ReadDiskBlock(size_t framesRequested);
		size_t	WriteDiskBlock(size_t framesRequested);
		size_t	ReadFloatBlock(size_t frameCount, float *floatSams);
		size_t	WriteFloatBlock(size_t frameCount, float *floatSams);
		
        int32_t FullToFileName();
		int32_t	CVertP(char *inPlaceStr);
		int32_t	PVertC(char *inPlaceStr);
		int32_t 	CheckChunkID(char test[4], char *ID);
		
		
		//properties
		bool		isOpen;
        char fullname[256];
        char filename[256];
		FILE * openFile;
		int32_t		nativeBig;
		uint32_t	fileType;
		uint32_t	dataFormat;
		uint32_t	sampleRate;
		uint32_t	sampleSize;
		off_t	dataStart;
		off_t	dataEnd;
		off_t	dataSize;
		off_t	frameCount;
		uint32_t	channelCount;
		off_t	framePosition;
		uint32_t	compressedBlockSize;
        float   *display;
	private:
		float	oneOver128;
		float	oneOver32768;
		float	oneOver8388608;
		float	oneOver2147483648;
		char	*charBlock;
		char	*compBlock;
		int16_t	*shortBlock;
		int32_t	*longBlock;
		off_t	framesPerBlock;
};
// END (class declaration)

// WAVE stuff
#define	WAV_ID_RIFF				"RIFF"
#define	WAV_ID_WAVE				"WAVE"
#define	WAV_ID_LIST				"LIST"
#define	WAV_ID_FORMAT			"fmt "
#define	WAV_ID_DATA				"data"
#define	WAV_ID_INFO				"INFO"
#define	WAV_ID_INAM				"INAM"
#define	WAV_ID_WAVELIST			"wavl"
#define	WAV_ID_SILENCE			"sInt"
#define	WAV_ID_INST				"inst"
#define	WAV_ID_SAMPLE			"smpl"
#define	WAV_ID_CUE				"cue "

#define	WAV_FORMAT_PCM			0x0001
#define	WAV_FORMAT_ADDVI		0x0002
#define	WAV_FORMAT_ADIMA		0x0011
#define	IBM_FORMAT_MULAW		0x0101
#define	IBM_FORMAT_ALAW			0x0102
#define	IBM_FORMAT_ADDVI		0x0103

#define SIZEOF_CHUNKHEADER		 8

#define SIZEOF_WAVFORMCHUNK		12
typedef	struct
	{
		char	ckID[4];
		int32_t	ckSize;
		char	formType[4];
	}	RIFFFormChunk;

#define	SIZEOF_WAVFORMATCHUNK	22
typedef struct
	{
		char	ckID[4];			//4
		int32_t	ckSize;				//4
		int16_t	wFormatTag;			//2
		int16_t	wChannels;			//2
		int32_t	dwSamplePerSec;		//4
		int32_t	dwAvgBytesPerSec;	//4
		int16_t	wBlockAlign;		//2 (22 total)
	}	WAVEFormatChunk;

typedef struct
	{
		char			ckID[4];
		int32_t			ckSize;
		unsigned char	bUnshiftedNote;
		char			chFineTune;
		char			chGain;
		unsigned char	bLowNote;
		unsigned char	bHighNote;
		unsigned char	bLowVelocity;
		unsigned char	bHighVelocity;
	}	WAVEInstChunk;

typedef struct
	{
		char	ckID[4];
		int32_t	ckSize;
		int32_t	dwManufacturer;
		int32_t	dwProduct;
		int32_t	dwSamplePeriod;
		int32_t	dwMIDIUnityNote;
		int32_t	dwMIDIPitchFraction;
		int32_t	dwSMPTEFormat;
		int32_t	dwSMPTEOffset;
		int32_t	cSampleLoops;
		int32_t	cbSamplerData;
	}	WAVESampleHeader;

typedef struct
	{
		int32_t	dwIdentifier;
		int32_t	dwType;
		int32_t	dwStart;
		int32_t	dwEnd;
		int32_t	dwFraction;
		int32_t	dwPlayCount;
	}	WAVESampleLoop;

typedef struct
	{
		char	ckID[4];
		int32_t	ckSize;
		int32_t	dwCuePoints;
	}	WAVECueHeader;

typedef struct 
	{
		int32_t	dwName;	
		int32_t	dwPosition;
		char	fccChunk[4];
		int32_t	dwChunkStart;
		int32_t	dwBlockStart;	
		int32_t	dwSampleOffset;
	}	WAVECuePoint;

/* NeXT - Sun DEFINES AND STRUCTURES */
#define	SUNMAGIC	0x2e736e64L

#define	SUN_FORMAT_UNSPECIFIED		(0)
#define	SUN_FORMAT_MULAW_8			(1)
#define	SUN_FORMAT_LINEAR_8		(2)
#define	SUN_FORMAT_LINEAR_16		(3)
#define	SUN_FORMAT_LINEAR_24		(4)
#define	SUN_FORMAT_LINEAR_32		(5)
#define	SUN_FORMAT_FLOAT			(6)
#define	SUN_FORMAT_DOUBLE			(7)
#define	SUN_FORMAT_INDIRECT		(8)
#define	SUN_FORMAT_NESTED			(9)
#define	SUN_FORMAT_DSP_CORE		(10)
#define	SUN_FORMAT_DSP_DATA_8		(11)
#define	SUN_FORMAT_DSP_DATA_16		(12)
#define	SUN_FORMAT_DSP_DATA_24		(13)
#define	SUN_FORMAT_DSP_DATA_32		(14)
#define	SUN_FORMAT_DISPLAY			(16)
#define	SUN_FORMAT_MULAW_SQUELCH	(17)
#define	SUN_FORMAT_EMPHASIZED		(18)
#define	SUN_FORMAT_COMPRESSED		(19)
#define	SUN_FORMAT_COMPRESSED_EMPHASIZED	(20)
#define	SUN_FORMAT_DSP_COMMANDS	(21)
#define	SUN_FORMAT_DSP_COMMANDS_SAMPLES	(22)
#define	SUN_FORMAT_ALAW_8			(27)

/* typedef for NeXT or Sun header structure */

typedef struct
	{
		int32_t 	magic;
		int32_t	dataLocation;
		int32_t	dataSize;
		int32_t	dataFormat;
		int32_t	samplingRate;
		int32_t	channelCount;
	}	SunSoundInfo;

/* AIFF - AIFC DEFINES AND STRUCTURES */

/* defines for AIFF files */
#define	SHAIFFFORMID					"FORM"
#define SHAIFFFORMVERID				"FVER"
#define	SHAIFFTYPE				"AIFF"
#define	AIFCTYPE				"AIFC"
#define	SHAIFFCOMMONID				"COMM"
#define	SHAIFFSOUNDID					"SSND"
#define SHAIFFMARKERID				"MARK"
#define SHAIFFINSTRUMENTID			"INST"
#define SHAIFFMIDIDATAID				"MIDI"
#define SHAIFFJUNKID					"JUNK"
#define SHAIFFAUDIORECORDINGID		"AESD"
#define SHAIFFAPPLICATIONSPECIFICID	"APPL"
#define SHAIFFCOMMENTID				"COMT"
#define SHAIFFNAMEID					"NAME"
#define AUTHORID				"AUTH"
#define SHAIFFCOPYRIGHTID				"(c) "
#define SHAIFFANNOTATIONID			"ANNO"
#define	AIFCVERSION1			0xA2805140

/* defines for AIFC compression types */
#define AIFC_ID_ADDVI	"ADP4"
#define AIFC_ID_ADIMA	"ima4"
#define AIFC_ID_MACE3	"MAC3"
#define AIFC_ID_MACE6	"MAC6"
#define AIFC_ID_NONE	"NONE"
#define AIFC_ID_ULAW	"ulaw"
#define AIFC_ID_ALAW	"alaw"
#define AIFC_ID_FLT32	"FL32"
#define AIFC_ID_FLT32L	"fl32"
#define AIFC_ID_IN08	"in08"
#define AIFC_ID_IN16	"in16"
#define AIFC_ID_IN24	"in24"
#define AIFC_ID_IN32	"in32"

/* defines for uncompressed AIFF - AIFC */
#define	SHAIFF_FORMAT_LINEAR_8	8
#define	SHAIFF_FORMAT_LINEAR_16	16
#define	SHAIFF_FORMAT_LINEAR_24	24
#define	SHAIFF_FORMAT_LINEAR_32	32

/* structs for Audio IFF file format */
typedef	int32_t SHAIFFID;

#define SIZEOF_AIFFFORMCHUNK	12
typedef	struct
	{
		char	ckID[4];
		int32_t	ckSize;
		char	formType[4];
	}	SHAIFFFormChunk;

#define	SIZEOF_AIFFCOMMONCHUNK	26
typedef	struct
	{
		char			ckID[4];			// 4
		int32_t			ckSize;				// 4
		int16_t			numChannels;		// 2
		uint16_t	numSampleFrames1;	// 4
		uint16_t	numSampleFrames2;	// 4
		int16_t			sampleSize;			// 2
		unsigned char	sampleRate[10];		//10
	}	SHAIFFCommonChunk;					//26

#define	SIZEOF_AIFFSOUNDDATACHUNK 16
typedef	struct
	{
		char			ckID[4];
		int32_t			ckSize;
		uint32_t	offset;
		uint32_t	blockSize;
		unsigned char	*soundData;
	}	SHAIFFSoundDataChunk;

typedef	struct
	{
		char			ckID[4];
		int32_t			ckSize;
		uint32_t	offset;
		uint32_t	blockSize;
	}	SHAIFFSoundDataChunkHeader;

typedef	struct
	{
		char			ckID[4];
		int32_t			ckSize;
		uint16_t	numComments;
	}	SHAIFFCommentsChunkHeader;

typedef	struct
	{
		uint32_t	timeStamp;
		uint16_t	MarkerID;
		uint16_t	count;
	}	SHAIFFComment;

typedef struct
	{
		char			ckID[4];
		int32_t			ckSize;
		uint16_t	numMarkers;
	}	SHAIFFMarkerChunkHeader;

typedef struct
	{
		uint16_t	MarkerID;
		uint32_t	position;
		char			markerName[256];
	}	SHAIFFMarker;

/* Just so I don't have to deal with the variable sized text */
typedef struct
	{
		uint16_t	MarkerID;
		uint32_t	position;
	}	SHAIFFShortMarker;

typedef struct
	{
		int16_t			playMode;	/* 0 = No Loop, 1 = Forward Loop, 2 = For/Back Loop */
		uint16_t	beginLoop;
		uint16_t	endLoop;
	}	SHAIFFLoop;

typedef struct
	{
		char			ckID[4];			// 4
		int32_t			ckSize;			// 4
		char			baseFrequency;	// 1
		char			detune;			// 1
		char			lowFrequency;	// 1
		char			highFrequency;	// 1
		char			lowVelocity;	// 1
		char			highVelocity;	// 1
		int16_t			gain;			// 2
		SHAIFFLoop		sustainLoop;	// 6
		SHAIFFLoop		releaseLoop;	// 6
	}	SHAIFFInstrumentChunk;


/* Additional structs for AIFC support. AIFC uses SoundDataChunk and FormChunk from
 above declarations */

#define	SIZEOF_AIFCCOMMONCHUNK	286
typedef struct
	{
		char			ckID[4];				//4
		int32_t			ckSize;					//4
		int16_t			numChannels;			//2
		uint16_t	numSampleFrames1;		//2
		uint16_t	numSampleFrames2;		//2
		int16_t			sampleSize;				//2
		unsigned char	sampleRate[10];			//10
		char			compressionType[4];		//4
		char			compressionName[256];	//256
	}	AIFCCommonChunk;

#define	SIZEOF_AIFCVERSIONCHUNK	12
typedef struct
	{
		char			ckID[4];
		int32_t			ckSize;
		uint32_t	timestamp;
	}	AIFCFormatVersionChunk;


