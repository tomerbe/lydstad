/*
 *  SHSoundfile.cpp
 *  shapers-vst-2007
 *
 *  Created by Tom Erbe on 9/9/10.
 *  Copyright 2010 SoundHack. All rights reserved.
 *
 */
#include <stdint.h> // this won't work on windows...
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

/*------------------------------------------------------------------*/
/*	SHSoundFile.cpp												    */
/*	Copyright 1999 Tom Erbe, All Rights Reserved.		            */
/*------------------------------------------------------------------*/

// all includes here
#include "SHSoundfile.h"
#include "IEEE80.h"

//---------------
// constructor
// creates and empty soundfile objects
// you need to do New or Open after this
//---------------

SHSoundFile::SHSoundFile()
{
	union
	{
		char Array[4];
		int32_t Chars; 
	} TestUnion;
	char c;
	int32_t x;
	
	fileType = 'NULL';
	oneOver2147483648 = 1.0/2147483648.0;	// 32-bit scaling
	oneOver8388608 = 1.0/8388608.0;			// 24-bit scaling
	oneOver32768 = 1.0/32768.0;				// 16-bit scaling
	oneOver128 = 1.0/128.0;					// 8-bit scaling
	charBlock = 0;
	shortBlock = 0;
	longBlock = 0;
	compBlock = 0;
	compressedBlockSize = dataSize = frameCount = framePosition = 0;
	framesPerBlock = 0;
	c = 'a';
	/* Test platform Endianness */ 
	for(x = 0; x < 4; x++)
		TestUnion.Array[x] = c++;
	if (TestUnion.Chars == 0x61626364)
		nativeBig = true;
	else 
		nativeBig = false;
	isOpen = false;
}

//---------------
// destructor
//---------------

SHSoundFile::~SHSoundFile()
{
	Close();
}

//	returns 0 if it isn't a valid soundfile
int32_t SHSoundFile::Open(char *fileName)
{
	unsigned long lengthName, periodPlace;
	char tmpMIME[256];
    long i;
    int32_t error;
	
	openFile = fopen(fileName, "a+");
	if(openFile == NULL)
		return(-1);	
	fileType = 'NULL';
	// look at file extensions
	// and mac file IDs as well
	lengthName = strlen(fileName);
	periodPlace = 0;
	for(i = lengthName; i >= 0; i--)
	{
		if(fileName[i] == '.')
			periodPlace = i;
	}
	if(periodPlace != 0)
	{
		for(i = periodPlace+1; i <= lengthName; i++)
			tmpMIME[i - (periodPlace + 1)] = fileName[i];
	}
	
	if(strcmp(tmpMIME, "wav") == 0)
		fileType = 'WAVE';
	else if(strcmp(tmpMIME, "WAV") == 0)
		fileType = 'WAVE';
	else if(strcmp(tmpMIME, "wave") == 0)
		fileType = 'WAVE';
	else if(strcmp(tmpMIME, "wave") == 0)
		fileType = 'WAVE';
	else if(strcmp(tmpMIME, "sd2") == 0)
		fileType = 'SDII';
	else if(strcmp(tmpMIME, "sd2f") == 0)
		fileType = 'SDII';
	else if(strcmp(tmpMIME, "SD2") == 0)
		fileType = 'SDII';
	else if(strcmp(tmpMIME, "SD2F") == 0)
		fileType = 'SDII';
	else if(strcmp(tmpMIME, "aiff") == 0)
		fileType = 'AIFF';
	else if(strcmp(tmpMIME, "AIFF") == 0)
		fileType = 'AIFF';
	else if(strcmp(tmpMIME, "aif") == 0)
		fileType = 'AIFF';
	else if(strcmp(tmpMIME, "AIF") == 0)
		fileType = 'AIFF';
	else if(strcmp(tmpMIME, "aifc") == 0)
		fileType = 'AIFC';
	else if(strcmp(tmpMIME, "AIFC") == 0)
		fileType = 'AIFC';
	
	error = ReadHeader();
    display = (float *)malloc(sizeof(float) * channelCount * 1000);
	if(error == 0)
    {
		isOpen = true;
        strcpy(fullname, fileName);
        FullToFileName();
        framePosition = dataStart;
        ReadFloatBlock(1000, display);
    }
	return(error);
}

void SHSoundFile::Close()
{
    free(display);
	FreeDiskMemory();
	if(isOpen == true)
		fclose(openFile);
	isOpen = false;
}

int32_t SHSoundFile::CheckChunkID(char test[4], char *ID)
{
	if(test[0] == ID[0] && test[1] == ID[1] && test[2] == ID[2] && test[3] == ID[3])
		return(0);
	else
		return(-1);
}

// converts a pascal string to a c string in place
int32_t SHSoundFile::PVertC(char *inPlaceStr)
{
	int32_t characters, i;
	
	characters = inPlaceStr[0];
	for(i = 0; i<characters; i++)
		inPlaceStr[i] = inPlaceStr[i+1];
	inPlaceStr[characters] = 0;
	return(0);
}

// converts a c string to a pascal string in place
int32_t SHSoundFile::CVertP(char *inPlaceStr)
{
	int32_t i;
	char c;
	
	for(i = 0; i<256; i++)
	{
		if(inPlaceStr[i] == 0)
			break;
	}
	c = inPlaceStr[0];
	inPlaceStr[0] = i;
	for(; i>0; i--)
		inPlaceStr[i+1] = inPlaceStr[i];
	inPlaceStr[1] = c;
	return(0);
}

int32_t SHSoundFile::FullToFileName()
{
    size_t i, j;
    char c;
    
    for(i = strlen(fullname); i>0; i--)
    {
        c = fullname[i];
        if(c == '/')
            break;
    }
    i++;
    for(j = 0; j < 256; j++, i++)
    {
        c = filename[j] = fullname[i];
        if(c == 0)
            break;
    }
}

// this will create the file and write a preliminary header
// before this gets called, the following should be filled out
// file, fileType, sampleRate, dataFormat, channelCount
int32_t SHSoundFile::CreateHeader()
{
	int32_t error = 0;
	
	switch(dataFormat)
	{
		case 'ibad':
		case 'msad':
		case 'imad':
			sampleSize = 4;
			break;
		case 'al08':
		case 'mu08':
		case 'us08':
			sampleSize = 8;
			break;
		case 'si16':
			sampleSize = 16;
			break;
		case 'si24':
			sampleSize = 24;
			break;
		case 'ti32':
		case 'si32':
			sampleSize = 32;
			break;
	}
	// initalize some dummy values
	dataStart = 0;
	dataEnd = 0;
	dataSize = 0;
	switch(fileType)
	{
		case 'WAVE':
			error = CreateWaveHeader();
			break;
		case 'SDII':
			error = CreateSDIIHeader();
			break;
		case 'AIFF':
			error = CreateAIFFHeader();
			break;
		case 'AIFC':
			error = CreateAIFCHeader();
			break;
		case 'SUN ':
			error = CreateSunHeader();
			break;
		default:
			error = -1;
	}
	if(error != 0)
		return(error);
	error = WriteHeader();
	return(error);
}


// this routine assumes that the file and fileType members
// are filled out - everything else is derived from the
// soundfile.
int32_t SHSoundFile::ReadHeader()
{
	int32_t error;
	
	switch(fileType)
	{
		case 'WAVE':
			error = ReadWaveHeader();
			break;
		case 'SDII':
//			error = ReadSDIIHeader();
			error = -1;
			break;
		case 'AIFF':
			error = ReadAIFFHeader();
			if(error == 0)
				break;
			error = ReadAIFCHeader();
			break;
		case 'AIFC':
			error = ReadAIFCHeader();
			if(error == 0)
				break;
			error = ReadAIFFHeader();
			break;
		case 'SUN ':
			error = ReadSunHeader();
			break;
		default:
			error = ReadWaveHeader();
			if(error == 0)
				break;
			error = ReadAIFFHeader();
			if(error == 0)
				break;
			error = ReadAIFCHeader();
			if(error == 0)
				break;
			error = ReadSunHeader();
			break;
	}
	return(error);
}

int32_t SHSoundFile::WriteHeader()
{
	int32_t error;
	
	switch(fileType)
	{
		case 'WAVE':
			error = WriteWaveHeader();
			break;
		case 'SDII':
			error = WriteSDIIHeader();
			break;
		case 'AIFF':
			error = WriteAIFFHeader();
			break;
		case 'AIFC':
			error = WriteAIFCHeader();
			break;
		case 'SUN ':
			error = WriteSunHeader();
			break;
		default:
			error = -1;
	}
	return(error);
}

int32_t SHSoundFile::ReadWaveHeader()
{
	int32_t	error;
	int16_t				soundFound = false, commonFound = false;
	int32_t				filePos;
	chunkHeader 		myHeader;
	RIFFFormChunk		myFormChunk;
	WAVEFormatChunk		myFormatChunk;
	int16_t				mySampleSize;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);

	if((CheckChunkID(myFormChunk.ckID, WAV_ID_RIFF) != 0) || (CheckChunkID(myFormChunk.formType, WAV_ID_WAVE)))
	{
		return(-1);
	}
	filePos += SIZEOF_WAVFORMCHUNK;
	/* Loop through the Chunks until finding the Format Chunk and Sound Chunk */
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(CheckChunkID(myHeader.ckID, WAV_ID_FORMAT) == 0)
		{
			fseek(openFile, filePos, SEEK_SET);
			fread(&myFormatChunk, SIZEOF_WAVFORMATCHUNK, 1, openFile);
			// swap if neccessary - WAVE is little endian - Intel
			// these macros should only swap on PPC
			if(nativeBig == true)
			{
				myFormatChunk.ckSize = SwapFourBytes(myFormatChunk.ckSize);
				myFormatChunk.wFormatTag = SwapTwoBytes(myFormatChunk.wFormatTag);
				myFormatChunk.wChannels = SwapTwoBytes(myFormatChunk.wChannels);
				myFormatChunk.dwSamplePerSec = SwapFourBytes(myFormatChunk.dwSamplePerSec);
				myFormatChunk.dwAvgBytesPerSec = SwapFourBytes(myFormatChunk.dwAvgBytesPerSec);
				myFormatChunk.wBlockAlign = SwapTwoBytes(myFormatChunk.wBlockAlign);
			}
			
			sampleRate = myFormatChunk.dwSamplePerSec;
			channelCount = myFormatChunk.wChannels;
			switch(myFormatChunk.wFormatTag)
			{
				case WAV_FORMAT_PCM:
					// 22 is a little magic to get around alignment problems
					// its the number of bytes myFormatChunk would be if it
					// was packed. so we get to the next data
					fseek(openFile, filePos + 22, SEEK_SET);
					fread(&mySampleSize, 2, 1, openFile);
					if(nativeBig == true)
						sampleSize = SwapTwoBytes(mySampleSize);
					else
						sampleSize = mySampleSize;

					if(sampleSize == 8)
						dataFormat = 'us08';
					else if(sampleSize == 16)
						dataFormat = 'si16';
					else if(sampleSize == 24)
						dataFormat = 'si24';
					else if(sampleSize == 32)
						dataFormat = 'si32';
					break;
				case IBM_FORMAT_MULAW:
					sampleSize = 8;
					dataFormat = 'mu08';
					return(-1);
					break;
				case IBM_FORMAT_ALAW:
					sampleSize = 8;
					dataFormat = 'al08';
					return(-1);
					break;
				case IBM_FORMAT_ADDVI:
					sampleSize = 4;
					dataFormat = 'ibad';
					return(-1);
					break;
				case WAV_FORMAT_ADDVI:
					sampleSize = 4;
					dataFormat = 'msad';
					return(-1);
					break;
				case WAV_FORMAT_ADIMA:
					sampleSize = 4;
					dataFormat = 'imad';
					return(-1);
					break;
				default:
					return(-1);
					break;
			}
			commonFound = true;
			filePos += (myFormatChunk.ckSize + SIZEOF_CHUNKHEADER);
		}
		else if(CheckChunkID(myHeader.ckID, WAV_ID_DATA) == 0)
		{
			/* We only want to get the offset for start of sound and the number of bytes */
			if(nativeBig == true)
				dataSize = myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			else
				dataSize = myHeader.ckSize;
			dataStart = filePos + SIZEOF_CHUNKHEADER;
			dataEnd = filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
			soundFound = true;
		}
		else
		{
			if(nativeBig == true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	fileType = 'WAVE';
	return(0);
}

int32_t SHSoundFile::CreateWaveHeader()
{
	int32_t				filePos;
	RIFFFormChunk		myFormChunk;
	WAVEFormatChunk		myFormatChunk;
	chunkHeader			myHeader;
		
	filePos = 0;
	/* Initialize chunks */
	
	// setup form and format chunks together
	strncpy(myFormChunk.ckID, WAV_ID_RIFF, 4);
	strncpy(myFormChunk.formType, WAV_ID_WAVE, 4);
	strncpy(myFormatChunk.ckID, WAV_ID_FORMAT, 4);
	switch(dataFormat)
	{
		case 'us08':
		case 'si16':
		case 'si24':
		case 'si32':
			myFormatChunk.ckSize = SIZEOF_WAVFORMATCHUNK - SIZEOF_CHUNKHEADER + sizeof(int16_t);
			myFormChunk.ckSize = SIZEOF_WAVFORMCHUNK + SIZEOF_WAVFORMATCHUNK;
			if(nativeBig == true)
			{
				myFormChunk.ckSize = SwapFourBytes(myFormChunk.ckSize);
				myFormatChunk.ckSize = SwapFourBytes(myFormatChunk.ckSize);
			}
			break;
		default:
			myFormatChunk.ckSize = SIZEOF_WAVFORMATCHUNK - SIZEOF_CHUNKHEADER;
			myFormChunk.ckSize = SIZEOF_WAVFORMCHUNK + SIZEOF_WAVFORMATCHUNK;
			if(nativeBig == true)
			{
				myFormChunk.ckSize = SwapFourBytes(myFormChunk.ckSize);
				myFormatChunk.ckSize = SwapFourBytes(myFormatChunk.ckSize);
			}
			break;
	}
	fseek(openFile, filePos, SEEK_SET);
	if(nativeBig == true)
		fwrite(&myFormChunk, SwapFourBytes(myFormChunk.ckSize) + SIZEOF_CHUNKHEADER, 1, openFile);
	else
		fwrite(&myFormChunk, myFormChunk.ckSize + SIZEOF_CHUNKHEADER, 1, openFile);
	filePos += 12;
	fseek(openFile, filePos, SEEK_SET);
	if(nativeBig == true)
	{
		fwrite(&myFormatChunk, SwapFourBytes(myFormatChunk.ckSize) + SIZEOF_CHUNKHEADER, 1, openFile);
		filePos += SwapFourBytes(myFormatChunk.ckSize)+ SIZEOF_CHUNKHEADER;
	}
	else
	{
		fwrite(&myFormatChunk, myFormatChunk.ckSize + SIZEOF_CHUNKHEADER, 1, openFile);
		filePos += myFormatChunk.ckSize + SIZEOF_CHUNKHEADER;
	}
	strncpy(myHeader.ckID, WAV_ID_DATA, 4);
	myHeader.ckSize = 0L;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
	return(0);
}

// this will write an existing WaveHeader
// updating it with new information
int32_t SHSoundFile::WriteWaveHeader()
{
	int32_t				error, pcmData;
	int16_t				soundFound = false, commonFound = false;
	off_t				filePos, fileSize;
	
	chunkHeader 		myHeader;
	RIFFFormChunk		myFormChunk;
	WAVEFormatChunk		myFormatChunk;
	int16_t				mySampleSize;
	unsigned char		zero;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);
	if((CheckChunkID(myFormChunk.ckID, WAV_ID_RIFF) != 0) || (CheckChunkID(myFormChunk.formType, WAV_ID_WAVE)))
	{
		return(error);
	}

	// update the chunkSize and rewrite the form chunk
	fseek(openFile, 0L, SEEK_END);
	fileSize = ftell(openFile);

	if(nativeBig == true)
		myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
	else
		myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;

	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);
	
	filePos += SIZEOF_WAVFORMCHUNK;
	/* Loop through the Chunks until finding the Format Chunk and Sound Chunk */
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(nativeBig == true)
			myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
		if(CheckChunkID(myHeader.ckID, WAV_ID_FORMAT) == 0)
		{
			strncpy(myFormatChunk.ckID, WAV_ID_FORMAT, 4);
			if(nativeBig == true)
				myFormatChunk.ckSize = SwapFourBytes(myHeader.ckSize);
			pcmData = false;
			switch(dataFormat)
			{
				case 'us08':
				case 'si16':
				case 'si24':
				case 'si32':
					myFormatChunk.wFormatTag = ((int16_t)WAV_FORMAT_PCM);
					pcmData = true;
					break;
				case 'mu08':
					myFormatChunk.wFormatTag = ((int16_t)IBM_FORMAT_MULAW);
					break;
				case 'al08':
					myFormatChunk.wFormatTag = ((int16_t)IBM_FORMAT_ALAW);
					break;
				case 'ibad':
					myFormatChunk.wFormatTag = ((int16_t)IBM_FORMAT_ADDVI);
					break;
				case 'msad':
					myFormatChunk.wFormatTag = ((int16_t)WAV_FORMAT_ADDVI);
					break;
				case 'imad':
					myFormatChunk.wFormatTag = ((int16_t)WAV_FORMAT_ADIMA);
					break;
			}
			if(nativeBig == true)
			{
				myFormatChunk.wFormatTag = SwapTwoBytes(myFormatChunk.wFormatTag);
				myFormatChunk.wChannels = SwapTwoBytes(channelCount);
				myFormatChunk.dwSamplePerSec = SwapFourBytes(sampleRate);
				myFormatChunk.dwAvgBytesPerSec = SwapFourBytes((sampleRate * sampleSize * channelCount)>>3);
				myFormatChunk.wBlockAlign = SwapTwoBytes((int16_t)((sampleSize * channelCount)>>3));
			}
			else			
			{
				myFormatChunk.wChannels = (channelCount);
				myFormatChunk.dwSamplePerSec = (sampleRate);
				myFormatChunk.dwAvgBytesPerSec = ((sampleRate * sampleSize * channelCount)>>3);
				myFormatChunk.wBlockAlign = ((int16_t)((sampleSize * channelCount)>>3));
			}
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myFormatChunk, SIZEOF_WAVFORMATCHUNK, 1, openFile);
			if(pcmData == true)
			{
				mySampleSize = (int16_t)sampleSize;
				if(nativeBig == true)
					mySampleSize = SwapTwoBytes(mySampleSize);
				fseek(openFile, filePos+22, SEEK_SET);
				fwrite(&mySampleSize, sizeof(int16_t), 1, openFile);
			}
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			commonFound = true;
		}
		else if(CheckChunkID(myHeader.ckID, WAV_ID_DATA) == 0)
		{
			/* We only need tp update the size of header */
			if(nativeBig == true)
				myHeader.ckSize = SwapFourBytes(dataSize);
			else
				myHeader.ckSize = dataSize;
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
			// pad the data and update the form chunk if odd sized data
			filePos += (dataSize + SIZEOF_CHUNKHEADER);
			if(dataSize & 1)
			{
				zero = 0;
				fseek(openFile, filePos, SEEK_SET);
				fwrite(&zero, 1, 1, openFile);
				EVENUP(filePos);
				/* Need to update FORM ckSize again */
				fseek(openFile, 0L, SEEK_END);
				fileSize = ftell(openFile);
				if(nativeBig == true)
					myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
				else
					myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;
				fseek(openFile, 0, SEEK_SET);
				fwrite(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);
			}
			soundFound = true;
		}
		else
		{
			if(nativeBig == true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	return(0);
}

/*int32_t SHSoundFile::ReadSDIIHeader()
{
	MacFile	*tmpFile;
	int32_t	error;
	char	*pSampleSize;
	char	*pSampleRate;
	char	*pChannelCount;
	float	fSampleRate;
	size_t	size;
	
	tmpFile = new MacFile(file, B_READ_WRITE);
	if((error = tmpFile->InitCheck()) != B_NO_ERROR)
	{
		delete tmpFile;
		return(error);
	}
	
	if((error = tmpFile->OpenResourceFork()) != B_OK)
	{
		delete tmpFile;
		return(error);
	}
	pSampleSize = (char *)tmpFile->FindResource('STR ', 1000L, &size);
	PVertC(pSampleSize);
	sscanf(pSampleSize,"%ld",&sampleSize);
	free(pSampleSize);
	sampleSize = 8 * sampleSize;
	
	pSampleRate = (char *)tmpFile->FindResource('STR ', 1001L, &size);
	PVertC(pSampleRate);
	sscanf(pSampleRate,"%f",&fSampleRate);
	sampleRate = (int32_t)fSampleRate;
	free(pSampleRate);
	
	pChannelCount = (char *)tmpFile->FindResource('STR ', 1002L, &size);
	PVertC(pChannelCount);
	sscanf(pChannelCount,"%ld",&channelCount);
	free(pChannelCount);
	
	tmpFile->CloseResourceFork();
	
	dataStart = 0;
	tmpFile->GetSize((&dataSize));
	dataEnd = dataSize;
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	switch(sampleSize)
	{
		case 8:
			dataFormat = 'si08';
			break;
		case 16:
			dataFormat = 'si16';
			break;
		case 24:
			dataFormat = 'si24';
			break;
		case 32:
			dataFormat = 'si32';
			break;
	}
	delete tmpFile;
	return(0);
}*/

int32_t SHSoundFile::CreateSDIIHeader()
{
	return(-1);
}

int32_t SHSoundFile::WriteSDIIHeader()
{
	return(-1);
}

int32_t SHSoundFile::ReadAIFFHeader()
{
	int32_t	error;
	int16_t				soundFound = false, commonFound = false;
	double				tmpDouble;
	int32_t				filePos;
	chunkHeader 		myHeader;
	SHAIFFFormChunk		myFormChunk;
	SHAIFFCommonChunk		myCommonChunk;
	SHAIFFSoundDataChunkHeader	mySoundDataChunk;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	if((CheckChunkID(myFormChunk.ckID, SHAIFFFORMID) != 0) || (CheckChunkID(myFormChunk.formType, SHAIFFTYPE)))
	{
		return(-1);
	}
	filePos += SIZEOF_AIFFFORMCHUNK;
	/* Loop through the Chunks until finding the Format Chunk and Sound Chunk */
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(CheckChunkID(myHeader.ckID, SHAIFFCOMMONID) == 0)
		{
			fseek(openFile, filePos, SEEK_SET);
			fread(&myCommonChunk, SIZEOF_AIFFCOMMONCHUNK, 1, openFile);
			// swap if neccessary - AIFF is big endian - Motorola
			// these macros should only swap on Intel
			if(nativeBig != true)
			{
				myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
				myCommonChunk.numChannels = SwapTwoBytes(myCommonChunk.numChannels);
				myCommonChunk.numSampleFrames1 = SwapTwoBytes(myCommonChunk.numSampleFrames1);
				myCommonChunk.numSampleFrames2 = SwapTwoBytes(myCommonChunk.numSampleFrames2);
				myCommonChunk.sampleSize = SwapTwoBytes(myCommonChunk.sampleSize);
			}
			
			tmpDouble = ieee_80_to_double(myCommonChunk.sampleRate);
			sampleRate = (int32_t)tmpDouble;
			sampleSize = myCommonChunk.sampleSize;
			channelCount = myCommonChunk.numChannels;
			frameCount = myCommonChunk.numSampleFrames2 + (uint32_t)myCommonChunk.numSampleFrames1 * 65536UL;
			switch(sampleSize)
			{
				case 8:
					dataFormat = 'si08';
					break;
				case 16:
					dataFormat = 'si16';
					break;
				case 24:
					dataFormat = 'si24';
					break;
				case 32:
					dataFormat = 'si32';
					break;
				default:
					return(-1);
					break;
			}
			dataSize = (frameCount * sampleSize * channelCount)>>3;
			commonFound = true;
			filePos += SIZEOF_AIFFCOMMONCHUNK;
		}
		else if(CheckChunkID(myHeader.ckID, SHAIFFSOUNDID) == 0)
		{
			/* We only want to get the offset for start of sound and the number of bytes */
			fseek(openFile, filePos, SEEK_SET);
			fread(&mySoundDataChunk, SIZEOF_AIFFSOUNDDATACHUNK, 1, openFile);
			if(nativeBig != true)
			{
				mySoundDataChunk.offset = SwapFourBytes(mySoundDataChunk.offset);
				mySoundDataChunk.ckSize = SwapFourBytes(mySoundDataChunk.ckSize);
			}
			dataStart = filePos + mySoundDataChunk.offset + SIZEOF_AIFFSOUNDDATACHUNK;
			dataEnd = filePos += (mySoundDataChunk.ckSize + SIZEOF_CHUNKHEADER);
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			EVENUP(filePos);
			soundFound = true;
		}
		else
		{
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	fileType = 'AIFF';
	return(0);
}

int32_t SHSoundFile::CreateAIFFHeader()
{
	int32_t				filePos;
	SHAIFFFormChunk		myFormChunk;
	SHAIFFCommonChunk		myCommonChunk;
	SHAIFFSoundDataChunk	mySoundDataChunk;
	
	/*Create File*/
	
	filePos = 0;
	/* Initialize chunks */
	
	// setup form and format chunks together
	strncpy(myFormChunk.ckID, SHAIFFFORMID, 4);
	strncpy(myFormChunk.formType, SHAIFFTYPE, 4);
	myFormChunk.ckSize = SIZEOF_AIFFFORMCHUNK + SIZEOF_AIFFCOMMONCHUNK + SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
	
	strncpy(myCommonChunk.ckID, SHAIFFCOMMONID, 4);
	myCommonChunk.ckSize = SIZEOF_AIFFCOMMONCHUNK - SIZEOF_CHUNKHEADER;
	
	strncpy(mySoundDataChunk.ckID, SHAIFFSOUNDID, 4);
	mySoundDataChunk.ckSize = SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
	mySoundDataChunk.offset = 0L;
	mySoundDataChunk.blockSize = 0L;
	if(nativeBig != true)
	{
		myFormChunk.ckSize = SwapFourBytes(myFormChunk.ckSize);
		myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
		mySoundDataChunk.ckSize = SwapFourBytes(mySoundDataChunk.ckSize);
		mySoundDataChunk.offset = SwapFourBytes(mySoundDataChunk.offset);
		mySoundDataChunk.blockSize = SwapFourBytes(mySoundDataChunk.blockSize);
	}
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	filePos += SIZEOF_AIFFFORMCHUNK;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myCommonChunk, SIZEOF_AIFFCOMMONCHUNK, 1, openFile);
	filePos += SIZEOF_AIFFCOMMONCHUNK;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&mySoundDataChunk, SIZEOF_AIFFSOUNDDATACHUNK, 1, openFile);
	return(0);
}

int32_t SHSoundFile::WriteAIFFHeader()
{
	int32_t				error;
	unsigned char		zero;
	int16_t				soundFound = false, commonFound = false;
	off_t				filePos, fileSize;
	double				tmpDouble;
	chunkHeader 		myHeader;
	SHAIFFFormChunk		myFormChunk;
	SHAIFFCommonChunk		myCommonChunk;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	if((CheckChunkID(myFormChunk.ckID, SHAIFFFORMID) != 0) || (CheckChunkID(myFormChunk.formType, SHAIFFTYPE)))
	{
		error = -1;
		return(error);
	}
	fseek(openFile, 0L, SEEK_END);
	fileSize = ftell(openFile);
	if(nativeBig == true)
		myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;
	else
		myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	
	filePos += SIZEOF_AIFFFORMCHUNK;
	// Loop through the Chunks until finding the Format Chunk and Sound Chunk
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(nativeBig != true)
			myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
		if(CheckChunkID(myHeader.ckID, SHAIFFCOMMONID) == 0)
		{
			strncpy(myCommonChunk.ckID, SHAIFFCOMMONID, 4);
			myCommonChunk.ckSize = myHeader.ckSize;
			tmpDouble = (double)sampleRate;
			double_to_ieee_80(tmpDouble, myCommonChunk.sampleRate);
			myCommonChunk.numChannels = channelCount;
			myCommonChunk.sampleSize = sampleSize;
			myCommonChunk.numSampleFrames1 = (uint16_t)((frameCount >> 16) & 0x0000ffff);
			myCommonChunk.numSampleFrames2 = (uint16_t)(frameCount & 0x0000ffff);
			if(nativeBig != true)
			{
				myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
				myCommonChunk.numChannels = SwapTwoBytes(myCommonChunk.numChannels);
				myCommonChunk.sampleSize = SwapTwoBytes(myCommonChunk.sampleSize);
				myCommonChunk.numSampleFrames1 = SwapTwoBytes(myCommonChunk.numSampleFrames1);
				myCommonChunk.numSampleFrames2 = SwapTwoBytes(myCommonChunk.numSampleFrames2);
			}
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myCommonChunk, SIZEOF_AIFFCOMMONCHUNK, 1, openFile);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			commonFound = true;
		}
		else if(CheckChunkID(myHeader.ckID, SHAIFFSOUNDID) == 0)
		{
			// We only need tp update the size of header
			if(nativeBig == true)
				myHeader.ckSize = dataSize + SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
			else
				myHeader.ckSize = SwapFourBytes(dataSize + SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER);
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
			// pad the data and update the form chunk if odd sized data
			filePos += (dataSize + SIZEOF_AIFFSOUNDDATACHUNK);
			if(dataSize & 1)
			{
				zero = 0;
				fseek(openFile, filePos, SEEK_SET);
				fwrite(&zero, 1, 1, openFile);
				EVENUP(filePos);
				// Need to update FORM ckSize again
				fseek(openFile, 0L, SEEK_END);
				fileSize = ftell(openFile);
				if(nativeBig == true)
					myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;
				else
					myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
				fseek(openFile, 0, SEEK_SET);
				fwrite(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);
			}
			soundFound = true;
		}
		else
		{
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	return(0);
}

int32_t SHSoundFile::ReadAIFCHeader()
{
	int32_t	error;
	int16_t				soundFound = false, commonFound = false;
	double				tmpDouble;
	int32_t				filePos;
	chunkHeader 		myHeader;
	SHAIFFFormChunk		myFormChunk;
	AIFCCommonChunk		myCommonChunk;
	SHAIFFSoundDataChunkHeader	mySoundDataChunk;
	AIFCFormatVersionChunk		myVersionChunk;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	if((CheckChunkID(myFormChunk.ckID, SHAIFFFORMID) != 0) || (CheckChunkID(myFormChunk.formType, AIFCTYPE)))
	{
		error = -1;
		return(error);
	}
	filePos += SIZEOF_AIFFFORMCHUNK;
	/* Loop through the Chunks until finding the Format Chunk and Sound Chunk */
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(CheckChunkID(myHeader.ckID, SHAIFFCOMMONID) == 0)
		{
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			fseek(openFile, filePos, SEEK_SET);
			fread(&myCommonChunk, myHeader.ckSize, 1, openFile);
			// swap if neccessary - WAVE is little endian - Intel
			// these macros should only swap on Intel
			if(nativeBig != true)
			{
				myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
				myCommonChunk.numChannels = SwapTwoBytes(myCommonChunk.numChannels);
				myCommonChunk.numSampleFrames1 = SwapTwoBytes(myCommonChunk.numSampleFrames1);
				myCommonChunk.numSampleFrames2 = SwapTwoBytes(myCommonChunk.numSampleFrames2);
				myCommonChunk.sampleSize = SwapTwoBytes(myCommonChunk.sampleSize);
			}
			tmpDouble = ieee_80_to_double(myCommonChunk.sampleRate);
			sampleRate = (int32_t)tmpDouble;
			channelCount = myCommonChunk.numChannels;
			frameCount = myCommonChunk.numSampleFrames2 + (uint32_t)myCommonChunk.numSampleFrames1 * 65536UL;
			// return -1 for any format not supported
			if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_NONE) == 0)
			{
				sampleSize = myCommonChunk.sampleSize;
				switch(sampleSize)
				{
					case 8:
						dataFormat = 'si08';
						break;
					case 16:
						dataFormat = 'si16';
						break;
					case 24:
						dataFormat = 'si24';
						break;
					case 32:
						dataFormat = 'si32';
						break;
					default:
						return(-1);
						break;
				}
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_ADIMA) == 0)
			{
				sampleSize = 4;
				dataFormat = 'imad';
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_MACE3) == 0)
			{
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_MACE6) == 0)
			{
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_ADDVI) == 0)
			{
				sampleSize = 4;
				dataFormat = 'msad';
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_ULAW) == 0)
			{
				sampleSize = 8;
				dataFormat = 'mu08';
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_ALAW) == 0)
			{
				sampleSize = 8;
				dataFormat = 'al08';
				return(-1);
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_FLT32) == 0)
			{
				sampleSize = 32;
				dataFormat = 'fl32';
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_FLT32L) == 0)
			{
				sampleSize = 32;
				dataFormat = 'fl32';
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_IN08) == 0)
			{
				sampleSize = 8;
				dataFormat = 'si08';
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_IN16) == 0)
			{
				sampleSize = 16;
				dataFormat = 'si16';
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_IN24) == 0)
			{
				sampleSize = 24;
				dataFormat = 'si24';
			}
			else if(CheckChunkID(myCommonChunk.compressionType, AIFC_ID_IN32) == 0)
			{
				sampleSize = 32;
				dataFormat = 'si32';
			}			
			else
			{
				return(-1);
			}			
			dataSize = (frameCount * sampleSize * channelCount)>>3;
			commonFound = true;
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
		}
		else if(CheckChunkID(myHeader.ckID, SHAIFFSOUNDID) == 0)
		{
			/* We only want to get the offset for start of sound and the number of bytes */
			fseek(openFile, filePos, SEEK_SET);
			fread(&mySoundDataChunk, SIZEOF_AIFFSOUNDDATACHUNK, 1, openFile);
			if(nativeBig != true)
			{
				mySoundDataChunk.offset = SwapFourBytes(mySoundDataChunk.offset);
				mySoundDataChunk.ckSize = SwapFourBytes(mySoundDataChunk.ckSize);
			}
			dataStart = filePos + mySoundDataChunk.offset + SIZEOF_AIFFSOUNDDATACHUNK;
			dataEnd = filePos += (mySoundDataChunk.ckSize + SIZEOF_CHUNKHEADER);
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			EVENUP(filePos);
			soundFound = true;
		}
		else if(CheckChunkID(myHeader.ckID, SHAIFFFORMVERID) == 0)
		{
			/* We only want to get the offset for start of sound and the number of bytes */
			fseek(openFile, filePos, SEEK_SET);
			fread(&myVersionChunk, sizeof(AIFCFormatVersionChunk), 1, openFile);
			if(nativeBig != true)
			{
				myVersionChunk.ckSize = SwapFourBytes(myHeader.ckSize);
				myVersionChunk.timestamp = SwapFourBytes(myVersionChunk.timestamp);
			}
			else
				myVersionChunk.ckSize = myHeader.ckSize;
			if(myVersionChunk.timestamp != AIFCVERSION1)
			{
				return(-1);
			}
			filePos += (myVersionChunk.ckSize + SIZEOF_CHUNKHEADER);
		}
		else
		{
			if(nativeBig != true)
				myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	fileType = 'AIFC';
	return(0);
}

int32_t SHSoundFile::CreateAIFCHeader()
{
	int32_t				filePos;
	SHAIFFFormChunk		myFormChunk;
	AIFCCommonChunk		myCommonChunk;
	SHAIFFSoundDataChunk	mySoundDataChunk;
	AIFCFormatVersionChunk		myVersionChunk;
	
	filePos = 0;
	/* Initialize chunks */
	// setup form and format chunks together
	strncpy(myFormChunk.ckID, SHAIFFFORMID, 4);
	strncpy(myFormChunk.formType, AIFCTYPE, 4);
	myFormChunk.ckSize = SIZEOF_AIFFFORMCHUNK + SIZEOF_AIFCCOMMONCHUNK + SIZEOF_AIFCVERSIONCHUNK 
	+ SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
	
	strncpy(myVersionChunk.ckID, SHAIFFFORMVERID, 4);
	myVersionChunk.ckSize = SIZEOF_AIFCVERSIONCHUNK - SIZEOF_CHUNKHEADER;
	myVersionChunk.timestamp = AIFCVERSION1;
	
	
	strncpy(myCommonChunk.ckID, SHAIFFCOMMONID, 4);
	myCommonChunk.ckSize = SIZEOF_AIFCCOMMONCHUNK - SIZEOF_CHUNKHEADER;
	
	strncpy(mySoundDataChunk.ckID, SHAIFFSOUNDID, 4);
	mySoundDataChunk.ckSize = SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
	mySoundDataChunk.offset = 0L;
	mySoundDataChunk.blockSize = 0L;

	if(nativeBig != true)
	{
		myFormChunk.ckSize = SwapFourBytes(myFormChunk.ckSize);
		myVersionChunk.ckSize = SwapFourBytes(myVersionChunk.ckSize);
		myVersionChunk.timestamp = SwapFourBytes(myVersionChunk.timestamp);
		myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
		mySoundDataChunk.ckSize = SwapFourBytes(mySoundDataChunk.ckSize);
		mySoundDataChunk.offset = SwapFourBytes(mySoundDataChunk.offset);
		mySoundDataChunk.blockSize = SwapFourBytes(mySoundDataChunk.blockSize);
	}
	
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	filePos += SIZEOF_AIFFFORMCHUNK;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myVersionChunk, SIZEOF_AIFCVERSIONCHUNK, 1, openFile);
	filePos += SIZEOF_AIFCVERSIONCHUNK;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myCommonChunk, SIZEOF_AIFCCOMMONCHUNK, 1, openFile);
	filePos += SIZEOF_AIFCCOMMONCHUNK;
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&mySoundDataChunk, SIZEOF_AIFFSOUNDDATACHUNK, 1, openFile);
	return(0);
}

int32_t SHSoundFile::WriteAIFCHeader()
{
	int32_t				error;
	unsigned char		zero;
	int16_t				soundFound = false, commonFound = false;
	off_t				filePos, fileSize;
	double				tmpDouble;
	chunkHeader 		myHeader;
	SHAIFFFormChunk		myFormChunk;
	AIFCCommonChunk		myCommonChunk;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	if((CheckChunkID(myFormChunk.ckID, SHAIFFFORMID) != 0) || (CheckChunkID(myFormChunk.formType, AIFCTYPE)))
	{
		error = -1;
		return(error);
	}
	fseek(openFile, 0L, SEEK_END);
	fileSize = ftell(openFile);
	if(nativeBig == true)
		myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;
	else
		myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
	fseek(openFile, filePos, SEEK_SET);
	fwrite(&myFormChunk, SIZEOF_AIFFFORMCHUNK, 1, openFile);
	
	filePos += SIZEOF_AIFFFORMCHUNK;
	// Loop through the Chunks until finding the Format Chunk and Sound Chunk
	while(commonFound == false || soundFound == false)
	{
		fseek(openFile, filePos, SEEK_SET);
		fread(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
		if(nativeBig != true)
			myHeader.ckSize = SwapFourBytes(myHeader.ckSize);
		if(CheckChunkID(myHeader.ckID, SHAIFFCOMMONID) == 0)
		{
			strncpy(myCommonChunk.ckID, SHAIFFCOMMONID, 4);
			myCommonChunk.ckSize = myHeader.ckSize;
			tmpDouble = (double)sampleRate;
			double_to_ieee_80(tmpDouble, myCommonChunk.sampleRate);
			myCommonChunk.numChannels = channelCount;
			myCommonChunk.numSampleFrames1 = (uint16_t)((frameCount >> 16) & 0x0000ffff);
			myCommonChunk.numSampleFrames2 = (uint16_t)(frameCount & 0x0000ffff);
			switch(dataFormat)
			{
				case 'imad':
					strncpy(myCommonChunk.compressionType, AIFC_ID_ADIMA, 4);
					myCommonChunk.sampleSize = 16L;
					sprintf(myCommonChunk.compressionName, "IMA 4:1 ADPCM Compression");
					break;
				case 'msad':
					strncpy(myCommonChunk.compressionType, AIFC_ID_ADDVI, 4);
					myCommonChunk.sampleSize = 16L;
					sprintf(myCommonChunk.compressionName, "Intel/DVI 4:1 ADPCM Compression");
					break;
				case 'mu08':
					strncpy(myCommonChunk.compressionType, AIFC_ID_ULAW, 4);
					myCommonChunk.sampleSize = 16L;
					sprintf(myCommonChunk.compressionName, "µLaw 2:1 Compression");
					break;
				case 'al08':
					strncpy(myCommonChunk.compressionType, AIFC_ID_ALAW, 4);
					myCommonChunk.sampleSize = 16L;
					sprintf(myCommonChunk.compressionName, "aLaw 2:1 Compression");
					break;
				case 'fl32':
					strncpy(myCommonChunk.compressionType, AIFC_ID_FLT32, 4);
					myCommonChunk.sampleSize = 16L;
					sprintf(myCommonChunk.compressionName, "32-bit Floating Point");
					break;
				default:
					strncpy(myCommonChunk.compressionType, AIFC_ID_NONE, 4);
					myCommonChunk.sampleSize = sampleSize;
					sprintf(myCommonChunk.compressionName, "No Compression");
					break;
			}
			CVertP(myCommonChunk.compressionName);
			if(nativeBig != true)
			{
				myCommonChunk.ckSize = SwapFourBytes(myCommonChunk.ckSize);
				myCommonChunk.numChannels = SwapTwoBytes(myCommonChunk.numChannels);
				myCommonChunk.numSampleFrames1 = SwapTwoBytes(myCommonChunk.numSampleFrames1);
				myCommonChunk.numSampleFrames2 = SwapTwoBytes(myCommonChunk.numSampleFrames2);
				myCommonChunk.sampleSize = SwapTwoBytes(myCommonChunk.sampleSize);
			}
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myCommonChunk, SIZEOF_AIFCCOMMONCHUNK, 1, openFile);
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			commonFound = true;
		}
		else if(CheckChunkID(myHeader.ckID, SHAIFFSOUNDID) == 0)
		{
			// We only need tp update the size of header
			if(nativeBig == true)
				myHeader.ckSize = dataSize + SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER;
			else
				myHeader.ckSize = SwapFourBytes(dataSize + SIZEOF_AIFFSOUNDDATACHUNK - SIZEOF_CHUNKHEADER);
			fseek(openFile, filePos, SEEK_SET);
			fwrite(&myHeader, SIZEOF_CHUNKHEADER, 1, openFile);
			// pad the data and update the form chunk if odd sized data
			filePos += (dataSize + SIZEOF_AIFFSOUNDDATACHUNK);
			if(dataSize & 1)
			{
				zero = 0;
				fseek(openFile, filePos, SEEK_SET);
				fwrite(&zero, 1, 1, openFile);
				EVENUP(filePos);
				// Need to update FORM ckSize again
				fseek(openFile, 0L, SEEK_END);
				fileSize = ftell(openFile);
				if(nativeBig == true)
					myFormChunk.ckSize = fileSize - SIZEOF_CHUNKHEADER;
				else
					myFormChunk.ckSize = SwapFourBytes(fileSize - SIZEOF_CHUNKHEADER);
				fseek(openFile, 0, SEEK_SET);
				fwrite(&myFormChunk, SIZEOF_WAVFORMCHUNK, 1, openFile);
			}
			soundFound = true;
		}
		else
		{
			filePos += (myHeader.ckSize + SIZEOF_CHUNKHEADER);
			EVENUP(filePos);
		}
	}
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	return(0);
}

int32_t SHSoundFile::ReadSunHeader()
{
	uint32_t		filePos, error;
	SunSoundInfo	mySunSI;
	
	filePos = 0;
	fseek(openFile, filePos, SEEK_SET);
	fread(&mySunSI, sizeof(SunSoundInfo), 1, openFile);
	if(mySunSI.magic != SUNMAGIC)
	{
		return(-1);
	}
	switch(mySunSI.dataFormat)
	{
		case SUN_FORMAT_LINEAR_8:
			dataFormat = 'si08';
			sampleSize = 8;
			break;
		case SUN_FORMAT_LINEAR_16:
			dataFormat = 'si16';
			sampleSize = 16;
			break;
		case SUN_FORMAT_LINEAR_24:
			sampleSize = 24;
			dataFormat = 'si24';
			break;
		case SUN_FORMAT_LINEAR_32:
			sampleSize = 32;
			dataFormat = 'si32';
			break;
		case SUN_FORMAT_FLOAT:
			dataFormat = 'fl32';
			sampleSize = 32;
			break;
		case SUN_FORMAT_MULAW_8:
			dataFormat = 'mu08';
			sampleSize = 8;
			return(-1);
			break;
		case SUN_FORMAT_ALAW_8:
			dataFormat = 'al08';
			sampleSize = 8;
			return(-1);
			break;
		default:
			return(-1);
	}
	sampleRate = mySunSI.samplingRate;
	channelCount = mySunSI.channelCount;
	dataStart = mySunSI.dataLocation;
	fseek(openFile, 0L, SEEK_END);
	dataEnd = ftell(openFile);
	dataSize = dataEnd - dataStart;
	frameCount = (dataSize * 8) / (channelCount * sampleSize);
	fileType = 'SUN ';
	return(0);
}

int32_t SHSoundFile::CreateSunHeader()
{
	SunSoundInfo	mySunSI;
	uint32_t	error;
	
	/*Create File*/
	
	mySunSI.magic = SUNMAGIC;
	switch(dataFormat)
	{
		case 'si08':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_8;
			break;
		case 'si16':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_16;
			break;
		case 'si24':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_24;
			break;
		case 'si32':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_32;
			break;
		case 'mu08':
			mySunSI.dataFormat = SUN_FORMAT_MULAW_8;
			return(-1);
			break;
		case 'al08':
			mySunSI.dataFormat = SUN_FORMAT_ALAW_8;
			return(-1);
			break;
		case 'fl32':
			mySunSI.dataFormat = SUN_FORMAT_FLOAT;
			break;
		default:
			return(-1);
			break;
	}
	mySunSI.dataLocation = dataStart = sizeof(SunSoundInfo);
	fseek(openFile, 0, SEEK_SET);
	fwrite(&mySunSI, sizeof(SunSoundInfo), 1, openFile);
	return(0);
}

int32_t SHSoundFile::WriteSunHeader()
{
	SunSoundInfo	mySunSI;
	uint32_t			error;
	
	mySunSI.dataLocation = dataStart;
	mySunSI.dataSize = dataSize;
	mySunSI.magic = SUNMAGIC;
	switch(dataFormat)
	{
		case 'si08':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_8;
			break;
		case 'si16':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_16;
			break;
		case 'si24':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_24;
			break;
		case 'si32':
			mySunSI.dataFormat = SUN_FORMAT_LINEAR_32;
			break;
		case 'mu08':
			mySunSI.dataFormat = SUN_FORMAT_MULAW_8;
			return(-1);
			break;
		case 'al08':
			mySunSI.dataFormat = SUN_FORMAT_ALAW_8;
			return(-1);
			break;
		case 'fl32':
			mySunSI.dataFormat = SUN_FORMAT_FLOAT;
			break;
		default:
			return(-1);
			break;
	}
	mySunSI.samplingRate = sampleRate;
	mySunSI.channelCount = channelCount;
	fseek(openFile, 0, SEEK_SET);
	fwrite(&mySunSI, sizeof(SunSoundInfo), 1, openFile);
	return(0);
}

void SHSoundFile::FreeDiskMemory(void)
{
	if(charBlock != 0)
		free(charBlock);
	if(shortBlock != 0)
		free(shortBlock);
	if(longBlock != 0)
		free(longBlock);
	if(compBlock != 0)
		free(compBlock);
	compBlock = 0;
	charBlock = 0;
	shortBlock = 0;
	longBlock = 0;
}

size_t SHSoundFile::InitDiskBlock(size_t framesRequested)
{
	if(framesRequested != framesPerBlock)
		FreeDiskMemory();
	framesPerBlock = framesRequested;
	switch(sampleSize)
	{
		case 4:
		case 8:
			if(charBlock == 0)
				charBlock = (char *)malloc(framesPerBlock * channelCount);
			break;
		case 16:
			if(shortBlock == 0)
				shortBlock = (int16_t *)malloc(framesPerBlock * channelCount * 2);
			break;
		case 24:
			if(longBlock == 0)
				longBlock = (int32_t *)malloc(framesPerBlock * channelCount * 3);
			break;
		case 32:
			if(longBlock == 0)
				longBlock = (int32_t *)malloc(framesPerBlock * channelCount * 4);
			break;
	}
	if(compressedBlockSize != 0 && compBlock == 0)
		compBlock = (char *)malloc(compressedBlockSize);
	return(0);	
}

// this is called by ReadFloatBlock to get the data off of disk
size_t SHSoundFile::ReadDiskBlock(size_t framesRequested)
{
	int32_t	bytePosition;
	size_t framesRead;
	
	bytePosition = dataStart + ((framePosition * channelCount * sampleSize)>>3);
	fseek(openFile, bytePosition, SEEK_SET);
	switch(sampleSize)
	{
		case 4:
		case 8:
			framesRead = fread(charBlock, 1, (framesRequested * channelCount), openFile);
			framesRead = framesRead / channelCount;
			break;
		case 16:
			framesRead = fread(shortBlock, 2, (framesRequested * channelCount), openFile);
			framesRead = framesRead / channelCount;
			break;
		case 24:
			framesRead = fread(longBlock, 3, (framesRequested * channelCount), openFile);
			framesRead = framesRead / channelCount;
			break;
		case 32:
			framesRead = fread(longBlock, 4, (framesRequested * channelCount), openFile);
			framesRead = framesRead / channelCount;
			break;
	}	
	return(framesRead);
}

// this is called by WriteFloatBlock to write the data on to disk

size_t SHSoundFile::WriteDiskBlock(size_t framesRequested)
{
	int32_t	error, bytePosition;
	size_t framesWritten;
	
	bytePosition = dataStart + ((framePosition * channelCount * sampleSize)>>3);
	fseek(openFile, bytePosition, SEEK_SET);
	switch(sampleSize)
	{
		case 4:
		case 8:
			framesWritten = fwrite(charBlock, 1, (framesRequested * channelCount), openFile);
			framesWritten = framesWritten / channelCount;
			break;
		case 16:
			framesWritten = fwrite(shortBlock, 2, (framesRequested * channelCount), openFile);
			framesWritten = framesWritten / channelCount;
			break;
		case 24:
			framesWritten = fwrite(longBlock, 3, (framesRequested * channelCount), openFile);
			framesWritten = framesWritten / channelCount;
			break;
		case 32:
			framesWritten = fwrite(longBlock, 4, (framesRequested * channelCount), openFile);
			framesWritten = framesWritten / channelCount;
			break;
	}	
	frameCount += framesWritten;
	dataSize = (frameCount * channelCount * sampleSize) >> 3;
	dataEnd = dataSize + dataStart;
	WriteHeader();
	return(framesWritten);
}

// this returns an array of samples, read from the soundfile, with
// channels interleaved
// in order. the amplitude is normalized to the range of -1.0 to
// 1.0. the buffer "floatSams" needs to be allocated before this
// function is called - the SHSoundFileFilter Class does this 
// properly.
// the number of frames read is returned - or -1 if failed
size_t SHSoundFile::ReadFloatBlock(size_t framesRequested, float *floatSams)
{
	int32_t i, j, tmpLong, tmpLongOne, tmpLongTwo, tmpLongThree;
	size_t framesRead;
	int16_t tmpShort;
	union 
	{
		float f;
		uint32_t l;
	} tmpLongFloat;
	uint32_t channel, tmpULong;
	
	memset(floatSams, 0, (framesRequested * channelCount * sizeof(float)));
	if(framePosition > frameCount)
		framePosition = frameCount;
	if(InitDiskBlock(framesRequested) != 0)
		return(-1);
	if(framesRequested > (frameCount - framePosition))
		framesRequested = frameCount - framePosition;
	framesRead = ReadDiskBlock(framesRequested);
	if(framesRead > framesRequested)
		return(0);
	switch(dataFormat)
	{
		case 'us08':
			for(i = 0; i < (framesRead * channelCount); i++)
				floatSams[i] = (char)(*(charBlock+i) ^ 0x80) * oneOver128;
			break;	
		case 'si08':
			for(i = 0; i < (framesRead * channelCount); i++)
				floatSams[i] = *(charBlock+i) * oneOver128;
			break;	
		case 'si16':
			if(fileType == 'WAVE')
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpShort = SwapTwoBytes(*(shortBlock+i));
					else
						tmpShort = *(shortBlock+i);
					floatSams[i] = (float)tmpShort * oneOver32768;
				}
			else
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpShort = *(shortBlock+i);
					else
						tmpShort = SwapTwoBytes(*(shortBlock+i));
					floatSams[i] = (float)tmpShort * oneOver32768;
				}
			break;
		case 'si32':
			if(fileType == 'WAVE')
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpLong = SwapFourBytes(*(longBlock+i));
					else
						tmpLong = *(longBlock+i);
					floatSams[i] = (float)tmpLong * oneOver2147483648;
				}
			else
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpLong = *(longBlock+i);
					else
						tmpLong = SwapFourBytes(*(longBlock+i));
					floatSams[i] = (float)tmpLong * oneOver2147483648;
				}
			break;
		case 'si24':
			// byte swapping is not done
			if(fileType == 'WAVE' && nativeBig == true)
				// l1h0m0l0 m2l2h1m1 h3m3l3h2 untested
				for(i = j = 0; i < (framesRead * channelCount); i += 4, j += 3)
				{
					tmpLongOne = SwapFourBytes(*(longBlock+j));
					tmpLongTwo = SwapFourBytes(*(longBlock+j+1));
					tmpLongThree = SwapFourBytes(*(longBlock+j+2));
					tmpLong = ((tmpLongOne & 0x00ffffffL)<<8);
					floatSams[i] = tmpLong * oneOver2147483648;
					if(i+1 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongOne & 0xff000000L) >> 16) + ((tmpLongTwo & 0x0000ffffL) << 16);
					floatSams[i+1] = tmpLong * oneOver2147483648;
					if(i+2 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongTwo & 0xffff0000L) >> 8) + ((tmpLongThree & 0x000000ffL) << 24);
					floatSams[i+2] = tmpLong * oneOver2147483648;
					if(i+3 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongThree & 0xffffff00L));
					floatSams[i+3] = tmpLong * oneOver2147483648;
				}
			else if(fileType == 'WAVE' && nativeBig != true)
				// l1h0m0l0 m2l2h1m1 h3m3l3h2
				for(i = j = 0; i < (framesRead * channelCount); i += 4, j += 3)
				{
					tmpLong = ((*(longBlock+j) & 0x00ffffffL)<<8);
					floatSams[i] = tmpLong * oneOver2147483648;
					if(i+1 == (framesRead * channelCount))
						break;
					tmpLong = ((*(longBlock+j) & 0xff000000L) >> 16) + ((*(longBlock+j+1) & 0x0000ffffL) << 16);
					floatSams[i+1] = tmpLong * oneOver2147483648;
					if(i+2 == (framesRead * channelCount))
						break;
					tmpLong = ((*(longBlock+j+1) & 0xffff0000L) >> 8) + ((*(longBlock+j+2) & 0x000000ffL) << 24);
					floatSams[i+2] = tmpLong * oneOver2147483648;
					if(i+3 == (framesRead * channelCount))
						break;
					tmpLong = ((*(longBlock+j+2) & 0xffffff00L));
					floatSams[i+3] = tmpLong * oneOver2147483648;
				}
			else if(fileType != 'WAVE' && nativeBig == true)
				for(i = j = 0; i < (framesRead * channelCount); i += 4, j += 3)
				{
					// h0m0l0h1 m1l1h2m2 l2h3m3l3 swapped
					tmpLongOne = (*(longBlock+j));
					tmpLongTwo = (*(longBlock+j+1));
					tmpLongThree = (*(longBlock+j+2));
					tmpLong = ((tmpLongOne & 0xffffff00L));
					floatSams[i] = tmpLong * oneOver2147483648;
					if(i+1 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongOne & 0x000000ffL) << 24) + ((tmpLongTwo & 0xffff0000L) >> 8);
					floatSams[i+1] = tmpLong * oneOver2147483648;
					if(i+2 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongTwo & 0x0000ffffL) << 16) + ((tmpLongThree & 0xff000000L) >> 16);
					floatSams[i+2] = tmpLong * oneOver2147483648;
					if(i+3 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongThree & 0x00ffffffL)<<8);
					floatSams[i+3] = tmpLong * oneOver2147483648;
				}
			else if(fileType != 'WAVE' && nativeBig != true)
				for(i = j = 0; i < (framesRead * channelCount); i += 4, j += 3)
				{
					// h1l0m0h0 m2h2l1m1 l3m3h3l2
					// h0m0l0h1 m1l1h2m2 l2h3m3l3 swapped
					tmpLongOne = SwapFourBytes(*(longBlock+j));
					tmpLongTwo = SwapFourBytes(*(longBlock+j+1));
					tmpLongThree = SwapFourBytes(*(longBlock+j+2));
					tmpLong = ((tmpLongOne & 0xffffff00L));
					floatSams[i] = tmpLong * oneOver2147483648;
					if(i+1 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongOne & 0x000000ffL) << 24) + ((tmpLongTwo & 0xffff0000L) >> 8);
					floatSams[i+1] = tmpLong * oneOver2147483648;
					if(i+2 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongTwo & 0x0000ffffL) << 16) + ((tmpLongThree & 0xff000000L) >> 16);
					floatSams[i+2] = tmpLong * oneOver2147483648;
					if(i+3 == (framesRead * channelCount))
						break;
					tmpLong = ((tmpLongThree & 0x00ffffffL)<<8);
					floatSams[i+3] = tmpLong * oneOver2147483648;
				}
			break;
		case 'fl32':
			if(fileType == 'WAVE')
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpLongFloat.l = SwapFourBytes(*(longBlock+i));
					else
						tmpLongFloat.l = (*(longBlock+i));
					floatSams[i] = tmpLongFloat.f;
				}
			else
				for(i = 0; i < (framesRead * channelCount); i++)
				{
					if(nativeBig == true)
						tmpLongFloat.l = (*(longBlock+i));
					else
						tmpLongFloat.l = SwapFourBytes(*(longBlock+i));
					floatSams[i] = tmpLongFloat.f;
				}
			break;
			// compressesd formats are not supported yet
		case 'mu08':
		case 'al08':
		case 'ibad':
		case 'msad':
		case 'imad':
			return(-1);
			break;
	}
	framePosition += framesRead;
	return(framesRead);
}

size_t SHSoundFile::WriteFloatBlock(size_t framesRequested, float *floatSams)
{
	size_t framesWritten;
	int32_t i, j, tmpLongA, tmpLongB, tmpShort;
	uint32_t tmpULong, channel;
	float tmpFloat;
	
	if(InitDiskBlock(framesRequested) != 0)
		return(-1);
	
	switch(dataFormat)
	{
		case 'us08':
			for(i = 0; i < (framesRequested * channelCount); i++)
				*(charBlock+i) = (char)(floatSams[i] * 128.0) ^ (char)0x80;
			break;
		case 'si08':
			for(i = 0; i < (framesRequested * channelCount); i++)
				*(charBlock+i)  = (char)(floatSams[i] * 128.0);
			break;
		case 'si16':
			if(fileType == 'WAVE')
				for(i = 0; i < (framesRequested * channelCount); i++)
				{
					tmpShort = (int16_t)(floatSams[i] * 32768.0);
					if(nativeBig == true)
						*(shortBlock+i) = SwapTwoBytes(tmpShort);
					else
						*(shortBlock+i) = (tmpShort);
				}
			else
				for(i = 0; i < (framesRequested * channelCount); i++)
				{
					tmpShort = (int16_t)(floatSams[i] * 32768.0);
					if(nativeBig == true)
						*(shortBlock+i) = (tmpShort);
					else
						*(shortBlock+i) = SwapTwoBytes(tmpShort);
				}
			break;
		case 'si32':
			if(fileType == 'WAVE')
				for(i = 0; i < (framesRequested * channelCount); i++)
				{
					tmpLongA = (int32_t)(floatSams[i] * 2147483648.0);
					if(nativeBig == true)
						*(longBlock+i) = SwapFourBytes(tmpLongA);
					else
						*(longBlock+i) = (tmpLongA);
				}
			else
				for(i = 0; i < (framesRequested * channelCount); i++)
				{
					tmpLongA = (int32_t)(floatSams[i] * 2147483648.0);
					if(nativeBig == true)
						*(longBlock+i) = (tmpLongA);
					else
						*(longBlock+i) = SwapFourBytes(tmpLongA);
				}
			break;
		case 'si24':
			// not sure of endian issues on 24 - bit numbers
			for(i = j = 0; i < (framesRequested * channelCount); i += 4, j += 3)
			{
				if((i+1) == (framesRequested * channelCount))
				{
					tmpLongA = (int32_t)(floatSams[i] * 2147483648.0);
					*(longBlock+j) = tmpLongA & 0xffffff00L;
				}
				else
				{
					tmpLongA = (int32_t)(floatSams[i] * 2147483648.0);
					tmpLongB = (int32_t)(floatSams[i+1] * 2147483648.0);
					*(longBlock+j) = tmpLongA & 0xffffff00L | (tmpLongB & 0xff000000L)>>24;
				}
				if((i+2) == (framesRequested * channelCount))
				{
					tmpLongA = (int32_t)(floatSams[i+1] * 2147483648.0);
					*(longBlock+j+1) = (tmpLongA & 0x00ffff00L)<<8;
				}
				else
				{
					tmpLongA = (int32_t)(floatSams[i+1] * 2147483648.0);
					tmpLongB = (int32_t)(floatSams[i+2] * 2147483648.0);
					*(longBlock+j+1) = (tmpLongA & 0x00ffff00L)<<8 | (tmpLongB & 0xffff0000L)>>16;
				}
				if((i+3) == (framesRequested * channelCount))
				{
					tmpLongA = (int32_t)(floatSams[i+2] * 2147483648.0);
					*(longBlock+j+2) = (tmpLongA & 0x0000ff00L)<<16;
				}
				else
				{
					tmpLongA = (int32_t)(floatSams[i+2] * 2147483648.0);
					tmpLongB = (int32_t)(floatSams[i+3] * 2147483648.0);
					*(longBlock+j+2) = (tmpLongA & 0x0000ff00L)<<16 | (tmpLongB & 0xffffff00L)>>8;
				}
			}
			break;
			// compressesd formats are not supported yet
		case 'mu08':
		case 'al08':
		case 'ibad':
		case 'msad':
		case 'imad':
			return(-1);
			break;
	}
	framesWritten = WriteDiskBlock(framesRequested);
	framePosition += framesWritten;
	if(framesWritten <= 0)
		return(-1);
	return(framesWritten);
}
