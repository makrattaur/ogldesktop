#ifndef H_AUDIOTHREAD
#define H_AUDIOTHREAD

extern "C"
{
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}
#define NOMINMAX
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <memory>
#include <atomic>
#include <SFML/System/Thread.hpp>

#include <autohandle_win32.h>
#include <threadsafequeue.h>
#include "objectpool.h"
#include "types.h"


class AudioThread
{
public:
	AudioThread();
	bool InitAudio(AVFormatContext *formatCtx, unsigned int streamIndex);
	void DeInitAudio();
	void PostAudioMessage(AudioMessage::MessageType type);
	void ResampleAndQueueSample(AVFrame *audioFrame, uint64_t pts);
	size_t GetSampleQueueSize();
	int64_t GetCurrentPTS();
	double GetCurrentPosition();
	bool GetUnderflowState();
	int GetSampleCountInQueue();
	int GetResampledCountInQueue();
	double GetDevicePosition();
	void SetObjectPool(ObjectPool<QueuedSample> &pool);
private:
	IAudioClient *InitAudioClient();
	void AudioThreadProc();
	void DequeueNewSample();

	AVFormatContext *m_formatCtx;

	unsigned int m_audioStreamIndex;
	AVCodecContext *m_audioCodecCtx;
	AVStream *m_audioStream;

	GenericAutoHandle m_audioInitialisedEvent;
	std::atomic<bool> m_audioInitialised;

	int m_deviceChannelCount;
	uint64_t m_deviceChannelLayout;
	int m_deviceSamplingRate;
	AVSampleFormat m_deviceSampleFormat;
	int m_deviceBytesPerSample;

	int m_audioChannelCount;
	uint64_t m_audioChannelLayout;
	int m_audioSamplingRate;
	AVSampleFormat m_audioSampleFormat;
	int m_audioBytesPerSample;

	SwrContext *m_audioResampler;
	std::vector<char> m_resampleBuffer;
	int64_t m_resampleBufferSampleCount;

	GenericAutoHandle m_audioEvent;
	GenericAutoHandle m_audioMessageEvent;
	std::vector<HANDLE> m_waitHandles;
	ThreadSafeQueue<AudioMessage> m_audioMessages;
	ThreadSafeQueue<QueuedSample *> m_sampleQueue;
	std::unique_ptr<sf::Thread> m_audioThread;
	bool m_quitAudio;
	std::atomic<bool> m_canUnderflow;
	std::atomic<bool> m_underflowing;

	std::atomic<uint64_t> m_currentAudioPTS;
	std::atomic<double> m_currentAudioPos;

	std::atomic<int> m_queuedSampleCount;
	std::atomic<int> m_queuedResampledCount;

	std::atomic<double> m_devicePos;

	//std::unique_ptr<QueuedSample> m_currentSample;

	QueuedSample *m_currentSample;
	int m_currentSamplePos;
	int m_currentSampleChunkSize;

	ObjectPool<QueuedSample> *m_objectPool;
};


#endif

