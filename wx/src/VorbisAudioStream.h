// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#ifndef VorbisAudioStream_H
#define VorbisAudioStream_H

#include "AudioStream.h"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

class VorbisFile;

class VorbisAudioStream : public AudioStream
{
	std::string mFileName;
	bool mShouldLoop;
	boost::shared_ptr<VorbisFile> mFile;
	boost::shared_array<int16> mBuffer;
	size_t mBufferSize;
	volatile size_t mDataBegin;
	volatile size_t mDataEnd;
	volatile bool mDoneWithFile;
	size_t mUnderrunCount;

	enum {
		CHANNELS = 2
	};

	void InitializeFile();

	//////////
	// If we're supposed to be looping, and we're out of data, re-open
	// our file and read it from the beginning.
	//
	void RestartFileIfLoopingAndDone();

	size_t ReadIntoBlock(int16 *inSpace, size_t inLength);
	bool DoneReadingData();

	//////////
	// Return true if the circular buffer is full.
	//
	bool IsBufferFull();

	//////////
	// Get pointers to the free space in the buffer.  Because the buffer is
	// circular, the free space may be discontiguous.  If the space is
	// contiguous, *outSpace2 will be NULL.  If no space is available, both
	// *outSpace1 and *outSpace2 will be NULL.
	//
	// This API is inspired by some interfaces in DirectSound.
	//
	void GetFreeBlocks(int16 **outSpace1, size_t *outLength1,
					   int16 **outSpace2, size_t *outLength2);

	//////////
	// Mark the specified number of samples as written.
	//
	void MarkAsWritten(size_t inSize);

public:
	VorbisAudioStream(const char *inFileName, size_t inBufferSize,
					  bool inShouldLoop);
	~VorbisAudioStream();

	virtual void Idle();
	
protected:
	bool FillBuffer(void *outBuffer, unsigned long inFrames,
					PaTimestamp inTime);	
};


#endif // VorbisAudioStream_H

