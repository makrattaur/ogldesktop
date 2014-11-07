#ifndef H_TYPES
#define H_TYPES

#include <vector>
#include <inttypes.h>


class AudioMessage
{
public:
	enum MessageType
	{
		Undefined,
		NewFrame,
		StartPlayback,
		UpdateClock,
		Close
	};

	AudioMessage();
	AudioMessage(MessageType type);

	MessageType Type;
	//int NumSamples;
	//int BytesPerSample;
	//std::vector<char> ResampledSamples;
	//int ResampledSamplesCount;
};

class QueuedSample
{
public:
	QueuedSample();
	uint64_t PTS;
	int NumSamples;
	std::vector<char> ResampledSamples;
	int ResampledSamplesCount;
};

class QueuedVideoFrame
{
public:
	QueuedVideoFrame();
	uint64_t PTS;
	uint64_t Duration;
	int Type;
	bool DataAllocated;
	int PlaneYStride;
	int PlaneCbStride;
	int PlaneCrStride;
	std::vector<uint8_t> PlaneY;
	std::vector<uint8_t> PlaneCb;
	std::vector<uint8_t> PlaneCr;
	int RGBAStride;
	std::vector<uint8_t> RGBABytes;
};

class QueuedSubtitle
{
public:
	uint64_t PTS;
	uint32_t Delay; // In ms.
	uint32_t Duration; // In ms.
	std::string String;
};

#endif

