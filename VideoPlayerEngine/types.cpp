#include "types.h"


AudioMessage::AudioMessage()
	: Type(AudioMessage::MessageType::Undefined)
{
}


AudioMessage::AudioMessage(MessageType type)
	: Type(type)
{
}

QueuedSample::QueuedSample()
	: PTS(0),
	NumSamples(0),
	ResampledSamplesCount(0)
{
}

QueuedVideoFrame::QueuedVideoFrame()
	: PTS(0),
	Duration(0),
	Type(0),
	DataAllocated(false),
	PlaneYStride(0),
	PlaneCbStride(0),
	PlaneCrStride(0),
	RGBAStride(0)
{
}
