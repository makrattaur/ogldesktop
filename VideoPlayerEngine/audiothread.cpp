#include "audiothread.h"

#include <iostream>
#include "autocomptr.h"
#include "scopedcominitialisation.h"
#include "ffutils.h"


AudioThread::AudioThread()
	: m_currentAudioPTS(0), m_currentAudioPos(0), m_devicePos(0), m_currentSample(NULL)
{
}

bool AudioThread::InitAudio(AVFormatContext *formatCtx, unsigned int streamIndex)
{
	m_formatCtx = formatCtx;
	m_audioStreamIndex = streamIndex;

	m_audioStream = m_formatCtx->streams[m_audioStreamIndex];
	m_audioCodecCtx = m_audioStream->codec;

	m_audioInitialisedEvent.Reset(CreateEvent(NULL, FALSE, FALSE, NULL));
	m_audioInitialised = false;

	m_quitAudio = false;
	m_audioEvent.Reset(CreateEvent(NULL, FALSE, FALSE, NULL));
	m_audioMessageEvent.Reset(CreateEvent(NULL, FALSE, FALSE, NULL));
	m_waitHandles.push_back(m_audioEvent);
	m_waitHandles.push_back(m_audioMessageEvent);

	m_canUnderflow = false;

	m_audioThread.reset(new sf::Thread(&AudioThread::AudioThreadProc, this));
	m_audioThread->launch();

	WaitForSingleObject(m_audioInitialisedEvent, INFINITE);
	if(!m_audioInitialised)
	{
		std::cerr << "Falied to init audio." << std::endl;

		return false;
	}

	m_deviceChannelCount = 2;
	m_deviceChannelLayout = AV_CH_LAYOUT_STEREO;
	m_deviceSamplingRate = 48000;
	m_deviceSampleFormat = AV_SAMPLE_FMT_FLT;
	m_deviceBytesPerSample = av_get_bytes_per_sample(m_deviceSampleFormat);

	m_audioChannelCount = m_audioCodecCtx->channels;
	m_audioChannelLayout = m_audioCodecCtx->channel_layout;
	//m_audioChannelLayout = AV_CH_LAYOUT_STEREO;
	m_audioSamplingRate = m_audioCodecCtx->sample_rate;
	m_audioSampleFormat = m_audioCodecCtx->sample_fmt;
	m_audioBytesPerSample = av_get_bytes_per_sample(m_audioSampleFormat);

	SwrContext *swr = swr_alloc();
	int ret;
	ret = av_opt_set_channel_layout(swr, "in_channel_layout", m_audioChannelLayout, 0);
	ret = av_opt_set_int(swr, "in_channel_count", m_audioChannelCount, 0);
	ret = av_opt_set_int(swr, "in_sample_rate", m_audioSamplingRate, 0);
	ret = av_opt_set_sample_fmt(swr, "in_sample_fmt", m_audioSampleFormat, 0);

	ret = av_opt_set_channel_layout(swr, "out_channel_layout", m_deviceChannelLayout, 0);
	//ret = av_opt_set_channel_layout(swr, "out_channel_layout", AV_CH_LAYOUT_5POINT1, 0);
	ret = av_opt_set_int(swr, "out_channel_count", m_deviceChannelCount, 0);
	ret = av_opt_set_int(swr, "out_sample_rate", m_deviceSamplingRate, 0);
	ret = av_opt_set_sample_fmt(swr, "out_sample_fmt", m_deviceSampleFormat, 0);

	if(swr_init(swr) < 0)
	{
		std::cerr << "Cannot init reasmpler !" << std::endl;
	}

	m_resampleBufferSampleCount = av_rescale_rnd(m_audioCodecCtx->frame_size, m_deviceSamplingRate, m_audioSamplingRate, AV_ROUND_UP);
	m_resampleBuffer.resize(m_resampleBufferSampleCount * m_deviceBytesPerSample * m_deviceChannelCount);

	m_audioResampler = swr;

	return true;
}

void AudioThread::DeInitAudio()
{
	//m_quitAudio = true;
	//SetEvent(m_audioEvent);
	PostAudioMessage(AudioMessage::Close);
	m_audioThread->wait();

	swr_free(&m_audioResampler);
}

IAudioClient *AudioThread::InitAudioClient()
{
	HRESULT hr;

	IMMDeviceEnumerator *tempDeviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
		IID_PPV_ARGS(&tempDeviceEnumerator)
	);
	if(FAILED(hr))
	{
		return NULL;
	}
	AutoComPtr<IMMDeviceEnumerator> deviceEnumerator(tempDeviceEnumerator);

	IMMDevice *tempDevice = NULL;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &tempDevice);
	if(FAILED(hr))
	{
		return NULL;
	}
	AutoComPtr<IMMDevice> device(tempDevice);

	IAudioClient *tempAudioClient = NULL;
	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&tempAudioClient);
	if(FAILED(hr))
	{
		return NULL;
	}
	AutoComPtr<IAudioClient> audioClient(tempAudioClient);

#if 0
	WAVEFORMATEX *mixWaveFormat;
	audioClient->GetMixFormat(&mixWaveFormat);
	bool isExtendedStruct = mixWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE;
	WAVEFORMATEXTENSIBLE *mixWaveFormatExtended = (WAVEFORMATEXTENSIBLE *)mixWaveFormat;
	bool isPcm = mixWaveFormatExtended->SubFormat == KSDATAFORMAT_SUBTYPE_PCM;
	bool isFloat = mixWaveFormatExtended->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	CoTaskMemFree(mixWaveFormat);
#endif

#if 0
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.cbSize = 0;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
#endif
	// Identical to mix format.
	WAVEFORMATEXTENSIBLE waveFormat;
	waveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	waveFormat.Samples.wValidBitsPerSample = 32;
	waveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	waveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	waveFormat.Format.cbSize = 22;
	waveFormat.Format.nChannels = 2;
	waveFormat.Format.nSamplesPerSec = 48000;
	waveFormat.Format.wBitsPerSample = 32;
	waveFormat.Format.nBlockAlign = (waveFormat.Format.nChannels * waveFormat.Format.wBitsPerSample) / 8;
	waveFormat.Format.nAvgBytesPerSec = waveFormat.Format.nSamplesPerSec * waveFormat.Format.nBlockAlign;

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		35 * 10000, // 35 ms
		0,
		(WAVEFORMATEX *)&waveFormat,
		NULL
	);
	bool isUnsupported = hr == AUDCLNT_E_UNSUPPORTED_FORMAT;
	if(FAILED(hr))
	{
		return NULL;
	}

	hr = audioClient->SetEventHandle(m_audioEvent);
	bool clientInit = hr == AUDCLNT_E_NOT_INITIALIZED;
	if(FAILED(hr))
	{
		return NULL;
	}

	audioClient->AddRef();
	return audioClient;
}

void AudioThread::AudioThreadProc()
{
	ScopedComInitialisation comInit;
	if(!comInit.IsInitialised())
	{
		return;
	}

	AutoComPtr<IAudioClient> audioClient(InitAudioClient());
	if(audioClient.IsInvalid())
	{
		return;
	}

	HRESULT hr;

#if 0
	{
		ISimpleAudioVolume *tempVolumeCtl = NULL;
		hr = audioClient->GetService(IID_PPV_ARGS(&tempVolumeCtl));
		if(SUCCEEDED(hr))
		{
			AutoComPtr<ISimpleAudioVolume> volumeCtl(tempVolumeCtl);

			hr = volumeCtl->SetMute(TRUE, NULL);
		}
	}
#endif

	IAudioClock *tempAudioClock = NULL;
	hr = audioClient->GetService(IID_PPV_ARGS(&tempAudioClock));
	if(FAILED(hr))
	{
		return;
	}
	AutoComPtr<IAudioClock> audioClock(tempAudioClock);
	UINT64 clockFrequency;
	hr = audioClock->GetFrequency(&clockFrequency);

	IAudioRenderClient *tempRenderClient = NULL;
	hr = audioClient->GetService(IID_PPV_ARGS(&tempRenderClient));
	if(FAILED(hr))
	{
		return;
	}
	AutoComPtr<IAudioRenderClient> renderClient(tempRenderClient);

	UINT32 audioClientBufferSize;
	audioClient->GetBufferSize(&audioClientBufferSize);

	m_audioInitialised = true;
	SetEvent(m_audioInitialisedEvent);

	//audioClient->Start();

	UINT64 devicePos;

	while(!m_quitAudio)
	{
		DWORD handleIndex = WaitForMultipleObjects(m_waitHandles.size(), m_waitHandles.data(), FALSE, INFINITE);

		if(handleIndex == WAIT_OBJECT_0)
		{
			m_underflowing = false;

			if(m_sampleQueue.empty())
			{
				m_underflowing = true;

				BYTE *audioBuffer;
				renderClient->GetBuffer(0, &audioBuffer);
				renderClient->ReleaseBuffer(0, AUDCLNT_BUFFERFLAGS_SILENT);
			}
			else
			{
				//if(!m_currentSample)
				if(m_currentSample == NULL)
				{
					//m_sampleQueue.tryMoveDequeue(m_currentSample);
					//m_currentSample = m_sampleQueue.moveDequeueValue();
					//m_currentSamplePos = 0;
					DequeueNewSample();
				}

				//QueuedSample *sample = m_currentSample.get();
				QueuedSample *sample = m_currentSample;
				//int requestedBufferSize = 0;

				BYTE *audioBuffer;
				hr = renderClient->GetBuffer(sample->ResampledSamplesCount, &audioBuffer);
				//hr = renderClient->GetBuffer(m_currentSampleChunkSize, &audioBuffer);
				if(hr == AUDCLNT_E_BUFFER_TOO_LARGE)
				{
					//std::cout << "Buffer filled too fast !" << std::endl;
					continue;
				}

				m_currentAudioPTS = sample->PTS;
				//AVRational audioTimeBase;
				//audioTimeBase.num = 1;
				//audioTimeBase.den = m_audioSamplingRate;
				//double currentAudioPos = sample->PTS * av_q2d(audioTimeBase) + sample->NumSamples / (double)m_audioSamplingRate;
				double currentAudioPos = sample->PTS * av_q2d(m_audioStream->time_base);
				m_currentAudioPos = currentAudioPos;

				m_queuedSampleCount -= sample->NumSamples;
				m_queuedResampledCount -= sample->ResampledSamplesCount;

				if(FAILED(hr))
				{
					std::cerr << "Falied to get a audio buffer from the device !" << std::endl;

					audioClient->Stop();
					m_quitAudio = true;
					break;
				}

				memcpy(audioBuffer, sample->ResampledSamples.data(), sample->ResampledSamplesCount * m_deviceBytesPerSample * m_deviceChannelCount);
				//memcpy(audioBuffer, sample->ResampledSamples.data(), sample->ResampledSamplesCount * m_deviceBytesPerSample * m_deviceChannelCount);
#if 0
				for (size_t i = 0; i < sample->ResampledSamplesCount * m_deviceChannelCount; i++)
				{
					float *value = &((float *)audioBuffer)[i];
					*value = (*value) * 10.0f;
				}
#endif
				//renderClient->ReleaseBuffer(m_currentSampleChunkSize, 0);
				renderClient->ReleaseBuffer(sample->ResampledSamplesCount, 0);

				DequeueNewSample();
			}
		}
		else if(handleIndex == (WAIT_OBJECT_0 + 1))
		{
			AudioMessage msg;
			while(m_audioMessages.tryDequeue(msg))
			{
#if 0
				if(msg->Type == AudioMessage::NewFrame)
				{
					QueuedSample *sample = new QueuedSample();
					sample->BytesPerSample = msg->BytesPerSample;
					sample->NumSamples = msg->NumSamples;

					sample->ResampledSamplesCount = msg->ResampledSamplesCount;
					sample->ResampledSamples = std::move(msg->ResampledSamples);

					m_sampleQueue.enqueue(std::shared_ptr<QueuedSample>(sample));
				}
#endif
				if(msg.Type == AudioMessage::Close)
				{
					hr = audioClient->Stop();
					m_quitAudio = true;

					std::cout << "Quitting audio thread." << std::endl;
					break;
				}
				else if(msg.Type == AudioMessage::StartPlayback)
				{
					m_canUnderflow = true;
					hr = audioClient->Start();
				}
				else if(msg.Type == AudioMessage::UpdateClock)
				{
					hr = audioClock->GetPosition(&devicePos, NULL);
					if(SUCCEEDED(hr))
					{
						m_devicePos = devicePos / (double)clockFrequency;
					}
				}
			}
		}
		else
		{
			audioClient->Stop();
			return;
		}
	}
}

void AudioThread::DequeueNewSample()
{
	if(m_currentSample != NULL)
	{
		m_objectPool->ReturnInstance(m_currentSample);
	}

	//m_currentSample = m_sampleQueue.moveDequeueValue();
	m_currentSample = m_sampleQueue.dequeue();
	m_currentSamplePos = 0;
	m_currentSampleChunkSize = std::min(1024, m_currentSample->ResampledSamplesCount);
}

void AudioThread::PostAudioMessage(AudioMessage::MessageType type)
{
	m_audioMessages.enqueue(AudioMessage(type));
	SetEvent(m_audioMessageEvent);
}

void AudioThread::ResampleAndQueueSample(AVFrame *audioFrame, uint64_t pts)
{
	int64_t dstSampleCount = av_rescale_rnd(swr_get_delay(m_audioResampler, m_audioSamplingRate) + audioFrame->nb_samples, m_deviceSamplingRate, m_audioSamplingRate, AV_ROUND_UP);
	if(dstSampleCount > m_resampleBufferSampleCount)
	{
		m_resampleBuffer.resize(dstSampleCount * m_deviceBytesPerSample * m_deviceChannelCount);
		m_resampleBufferSampleCount = dstSampleCount;
	}

	uint8_t *resampleBuffer[8] = { (unsigned char *)m_resampleBuffer.data(), 0, 0, 0, 0, 0, 0, 0 };
	int outputSamples = swr_convert(m_audioResampler, (uint8_t **)resampleBuffer, m_resampleBufferSampleCount, (const uint8_t **)audioFrame->data, audioFrame->nb_samples);

	bool noBuffering = outputSamples == dstSampleCount;
	bool correctBufferSize = outputSamples <= dstSampleCount;

	//QueuedSample *sample = new QueuedSample();
	QueuedSample *sample = m_objectPool->GetInstance();
	sample->PTS = pts;
	//sample->BytesPerSample = av_get_bytes_per_sample((AVSampleFormat)m_decodedFrame->format);
	sample->NumSamples = audioFrame->nb_samples;

	sample->ResampledSamplesCount = outputSamples;
	//sample->ResampledSamples.resize(outputSamples * m_deviceBytesPerSample * m_deviceChannelCount);
	size_t byteCount = outputSamples * m_deviceBytesPerSample * m_deviceChannelCount;
	if(sample->ResampledSamples.size() < byteCount)
	{
		sample->ResampledSamples.resize(byteCount);
	}
	memcpy(sample->ResampledSamples.data(), m_resampleBuffer.data(), byteCount);

	//m_sampleQueue.enqueue(std::unique_ptr<QueuedSample>(sample));
	m_sampleQueue.enqueue(sample);

	m_queuedSampleCount += sample->NumSamples;
	m_queuedResampledCount += sample->ResampledSamplesCount;
}

size_t AudioThread::GetSampleQueueSize()
{
	return m_sampleQueue.size();
}

int64_t AudioThread::GetCurrentPTS()
{
	return m_currentAudioPTS;
}

double AudioThread::GetCurrentPosition()
{
	return m_currentAudioPos;
}

bool AudioThread::GetUnderflowState()
{
	return m_underflowing;
}

int AudioThread::GetSampleCountInQueue()
{
	return m_queuedSampleCount;
}

int AudioThread::GetResampledCountInQueue()
{
	return m_queuedResampledCount;
}

double AudioThread::GetDevicePosition()
{
	return m_devicePos;
}

void AudioThread::SetObjectPool(ObjectPool<QueuedSample> &pool)
{
	m_objectPool = &pool;
}
