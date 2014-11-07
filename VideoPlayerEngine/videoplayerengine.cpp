#include "videoplayerengine.h"
#include <sstream>
#include <iostream>
#include <cmath>
#include "scopedcominitialisation.h"
#include "ffutils.h"
extern "C"
{
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}
#include "videorendererstats.h"

#define ENGINE_ROOT "C:\\Users\\Michael\\Documents\\Visual Studio 2012\\Projects\\OGLDesktop\\VideoPlayerEngine\\"
#define MAKE_PATH(file) ENGINE_ROOT file
#define VIDEO_PATH MAKE_PATH("l4d2_intro.mp4")



VideoPlayerEngine::VideoPlayerEngine()
	: m_audioStarted(false),
	m_monotonicCounter(0),
	m_currentVideoPos(0.0),
	m_diffDeviceHistory(60 * 0.25f), m_diffAudioHistory(60 * 0.25f),
	m_wireframe(false),
	m_syncRefreshTimer(0.030f),
	m_masterClock(*this),
	m_audioStreamAvailable(false),
	m_videoStreamAvailable(false),
	m_subtitleStreamAvailable(false),
	m_useSubtitle(false),
	m_useAudio(false),
	m_diffClockHistory(60 * 0.25f),
	m_useScale(false),
	m_autoScale(false),
	m_autoScaleMode(false),
	m_autoScaleSmall(false),
	m_xScalingFactor(1.0f),
	m_yScalingFactor(1.0f),
	m_videoPool(5, true),
	m_audioPool(10, true)
{
}

bool VideoPlayerEngine::Init()
{
	InitOpenGL();

	//m_font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	m_font.loadFromFile("C:\\Windows\\Fonts\\DejaVuSansMono.ttf");
	m_textRenderer.SetSize(20);
	m_textRenderer.SetFont(m_font);
	//m_textRenderer.SetText("Hello World !");
	m_textRenderer.SetText("qwertyuiop");


	if(!InitFFmpeg())
	{
		return false;
	}

	m_useAudio = true;
	OpenAvailableStreams();

	InitVideo();
	InitAudio();
	InitSubtitle();
	
	if(!InitDemuxer())
	{
		return false;
	}

	InitMediaInfo();

	return true;
}

void VideoPlayerEngine::Update(float deltaTime)
{
	//std::cout << "\r" << wiper;
	//std::cout << "\rFrame time: " << deltaTime << " ms; qV: " << m_videoFrameQueue.size() << " frames; qA: " << m_sampleQueue.size() << " samples;";
	//std::ostringstream ss;
	//ss << "Frame time: " << deltaTime << " ms; qV: " << m_videoFrameQueue.size() << " frames; qA: " << m_sampleQueue.size() << " samples;";
	m_currentDeltaTime = deltaTime;
	m_currentVideoQueueSize = m_videoRenderer.FrameQueueSize();
	m_currentAudioQueueSize = m_audioThread.GetSampleQueueSize();
	//m_currentAudioPos = m_audioThread.GetCurrentPosition() - 0.02; // audio buffer is 35 ms, subtract 20 ms.
	m_currentAudioPos = m_audioThread.GetCurrentPosition() - 0.030;
	//m_currentAudioPos = m_audioThread.GetCurrentPosition();
	m_currentDevicePos = m_audioThread.GetDevicePosition();
	m_currentVideoPos = m_videoRenderer.GetPosition();

	m_currentMasterClockPos = m_masterClock.GetPosition();

	if(!m_decodingThread.IsAtEndOfMedia())
	{
		m_monotonicCounter += deltaTime;
	}

	m_syncRefreshTimer += deltaTime;
	if(m_syncRefreshTimer > 0.030f)
	{
		m_syncRefreshTimer = m_syncRefreshTimer - 0.030f;
		m_currentSyncDiff = m_currentVideoPos - m_currentAudioPos;
	}

	//m_textRenderer.SetText(ss.str());

	m_videoRenderer.Update(deltaTime);
	if(m_subtitleStreamAvailable)
	{
		m_subtitleRenderer.Update(deltaTime);
	}

	if(!m_audioStarted)
	{
		m_audioStarted = true;
		m_audioThread.PostAudioMessage(AudioMessage::StartPlayback);
	}

	m_audioThread.PostAudioMessage(AudioMessage::UpdateClock);

	m_diffDeviceHistory.Add(m_currentVideoPos - m_currentDevicePos);
	m_diffAudioHistory.Add(m_currentVideoPos - m_currentAudioPos);
	m_diffClockHistory.Add(m_currentVideoPos - m_currentMasterClockPos);
}

void VideoPlayerEngine::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	if(!m_useScale)
	{
		glTranslatef(GetWidth() / 2.0f - m_videoRenderer.GetFrameWidth() / 2.0f, GetHeight() / 2.0f - m_videoRenderer.GetFrameHeight() / 2.0f, 0.0f);
	}
	else
	{
		glTranslatef(GetWidth() / 2.0f - (m_videoRenderer.GetFrameWidth() * m_xScalingFactor) / 2.0f, GetHeight() / 2.0f - (m_videoRenderer.GetFrameHeight() * m_yScalingFactor) / 2.0f, 0.0f);
		glScalef(m_xScalingFactor, m_yScalingFactor, 0.0f);
	}
	m_videoRenderer.Draw();
	glPopMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	Color4ubPM(128, 140, 128, 128);

	glPushMatrix();
	glTranslatef(0.0, GetHeight(), 0.0f);
	glScalef(210.0f, -650.0f, 0.0f);
	DrawTexturedQuad();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 20.0f, 0.0f);
	glScalef(1150.0f, 85.0f, 0.0f);
	DrawTexturedQuad();
	glPopMatrix();

	glDisable(GL_BLEND);

	if(m_subtitleStreamAvailable)
	{
		glColor3ub(255, 255, 255);
		glPushMatrix();
		glTranslatef(200.0, 200.0f, 0.0f);
		m_subtitleRenderer.Draw();
		glPopMatrix();
	}

	glColor3ub(255, 255, 255);
	DrawPlaybackStats();
	DrawMediaInfo();

	glColor3ub(0, 255, 0);
	DrawMarker(m_currentAudioPos, 155.0f);

	glColor3ub(255, 0, 0);
	DrawMarker(m_currentVideoPos, 145.0f);
}

void VideoPlayerEngine::DeInit()
{
	m_decodingThread.DeInit();

	if(m_audioStreamAvailable)
	{
		m_audioSink.Quit();
		m_audioThread.DeInitAudio();
	}

	if(m_videoStreamAvailable)
	{
		m_videoSink.Quit();
		m_videoRenderer.DeInit();
	}

	if(m_subtitleStreamAvailable)
	{
		m_subtitleSink.Quit();
	}

	DeInitFFmpeg();
}

int VideoPlayerEngine::PreferredWidth()
{
	return 1600;
}

int VideoPlayerEngine::PreferredHeight()
{
	return 900;
}

const char *VideoPlayerEngine::PreferredTitle()
{
	return "Video Player";
}

void VideoPlayerEngine::OnKeyDown(KeyboardKey key)
{
	if(key == KeyboardKey::O)
	{
		m_videoRenderer.DumpNextFrame();
	}
	else if(key == KeyboardKey::L)
	{
		m_wireframe = !m_wireframe;

		glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);
	}
}

void VideoPlayerEngine::OnMouseButtonDown(MouseButton button, int x, int y)
{
	if(button == MouseButton::Left)
	{
		double frac = x / (double)GetWidth();
		int64_t ts = frac * m_formatCtx->duration;
		std::cout << "Seeking to ts " << ts << " of " << m_formatCtx->duration << "." << std::endl;
		m_decodingThread.Seek(ts);
		m_diffAudioHistory.Clear();
		m_diffDeviceHistory.Clear();
		m_diffClockHistory.Clear();
		//m_monotonicCounter = frac * m_formatCtx->duration / (double)(AV_TIME_BASE);
	}
}

VideoPlayerEngine::MasterClock::MasterClock(VideoPlayerEngine &parent)
	: m_parent(parent)
{
}

double VideoPlayerEngine::MasterClock::GetPosition()
{
	if(m_parent.m_audioStreamAvailable)
	{
		return m_parent.m_currentAudioPos;
	}
	else
	{
		//return m_parent.m_monotonicCounter;
		return m_parent.m_currentVideoPos;
	}
}

void VideoPlayerEngine::InitOpenGL()
{
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	bool glewOk = glewErr == GLEW_OK;

	if(glewIsSupported("GL_ARB_texture_non_power_of_two") != GL_TRUE)
	{
		std::cerr << "NPOT texture creation will fail !" << std::endl;
	}

	int width = GetWidth();
	int height = GetHeight();

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//glClearColor(0.0f, 0.25f, 0.5f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
}

bool VideoPlayerEngine::InitFFmpeg()
{
	av_register_all();

	m_formatCtx = NULL;
	if(avformat_open_input(&m_formatCtx, VIDEO_PATH, NULL, NULL) != 0)
	{
		return false;
	}

	if(avformat_find_stream_info(m_formatCtx, NULL) != 0)
	{
		return false;
	}

	return true;
}

void VideoPlayerEngine::OpenAvailableStreams()
{
	if(ffutil::OpenFirstMatchingCodecContext(m_formatCtx, AVMEDIA_TYPE_VIDEO, m_videoStreamIndex))
	{
		m_videoStreamAvailable = true;
		m_videoCodecCtx = m_formatCtx->streams[m_videoStreamIndex]->codec;
	}

	if(m_useAudio && ffutil::OpenFirstMatchingCodecContext(m_formatCtx, AVMEDIA_TYPE_AUDIO, m_audioStreamIndex))
	{
		m_audioStreamAvailable = true;
		m_audioCodecCtx = m_formatCtx->streams[m_audioStreamIndex]->codec;
	}

	if(m_useSubtitle && ffutil::OpenFirstMatchingCodecContext(m_formatCtx, AVMEDIA_TYPE_SUBTITLE, m_subtitleStreamIndex))
	{
		m_subtitleStreamAvailable = true;
		m_subtitleCodecCtx = m_formatCtx->streams[m_subtitleStreamIndex]->codec;
	}
}

void VideoPlayerEngine::InitVideo()
{
	if(!m_videoStreamAvailable)
	{
		return;
	}

	m_useScale = true;
	m_xScalingFactor = 1.5f;
	m_yScalingFactor = 1.5f;

	m_videoRenderer.SetObjectPool(m_videoPool);
	m_videoRenderer.SetEngineContext(GetEngineContext());
	if(m_useScale)
	{
		m_videoRenderer.SetUseScalableYCbCrPath(true);
	}
	m_videoRenderer.Init(m_formatCtx, m_videoStreamIndex);
	m_videoRenderer.SetMasterClock(m_masterClock);
	m_videoRendererStats.reset(new VideoRendererStats(m_videoRenderer));

	m_videoSink.SetObjectPool(m_videoPool);
	m_videoSink.SetVideoFrameQueue(m_videoRenderer);
	m_videoSink.InitSink(m_formatCtx, m_videoStreamIndex);

	m_decodingThread.AddSink(m_videoStreamIndex, m_videoSink);
}

void VideoPlayerEngine::InitAudio()
{
	if(!m_audioStreamAvailable)
	{
		return;
	}

	m_audioThread.SetObjectPool(m_audioPool);
	m_audioThread.InitAudio(m_formatCtx, m_audioStreamIndex);

	m_audioSink.SetAudioThread(m_audioThread);
	m_audioSink.InitSink(m_formatCtx, m_audioStreamIndex);

	m_decodingThread.AddSink(m_audioStreamIndex, m_audioSink);
}

void VideoPlayerEngine::InitSubtitle()
{
	if(!m_subtitleStreamAvailable)
	{
		return;
	}

	m_subtitleRenderer.Init(m_formatCtx, m_subtitleStreamIndex);
	m_subtitleRenderer.SetMasterClock(m_masterClock);

	m_subtitleSink.SetSubtitleQueue(m_subtitleRenderer);
	m_subtitleSink.InitSink(m_formatCtx, m_subtitleStreamIndex);

	m_decodingThread.AddSink(m_subtitleStreamIndex, m_subtitleSink);
}

bool VideoPlayerEngine::InitDemuxer()
{
	return m_decodingThread.Init(m_formatCtx);
}

void VideoPlayerEngine::DeInitFFmpeg()
{
	if(m_videoStreamAvailable)
	{
		avcodec_close(m_formatCtx->streams[m_videoStreamIndex]->codec);
	}

	if(m_audioStreamAvailable)
	{
		avcodec_close(m_formatCtx->streams[m_audioStreamIndex]->codec);
	}

	if(m_subtitleStreamAvailable)
	{
		avcodec_close(m_formatCtx->streams[m_subtitleStreamIndex]->codec);
	}

	avformat_close_input(&m_formatCtx);
}

void VideoPlayerEngine::DrawTexturedQuad()
{
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
	glEnd();
}

float RoundToNearest(float n)
{
    return (n > 0.0f) ? floor(n + 0.5f) : ceil(n - 0.5f);
}

void VideoPlayerEngine::DrawPlaybackStats()
{
	std::ostringstream ss;

	glPushMatrix();

	glTranslatef(10.0f, GetHeight() - 10 - 20, 0.0f);
	ss << "FPS: " << (1.0f / m_currentDeltaTime);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "DT: " << m_currentDeltaTime;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vFQ: " << m_currentVideoQueueSize;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "aPSQ: " << m_currentAudioQueueSize;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	if(m_audioThread.GetUnderflowState() && !m_decodingThread.IsAtEndOfMedia())
	{
		ss.str("");
		ss << "Ua!";
		m_textRenderer.SetText(ss.str());
		m_textRenderer.Draw();
	}

	glTranslatef(0.0f, -25.0f, 0.0f);
	if(m_videoRenderer.GetUnderflowState() && !m_decodingThread.IsAtEndOfMedia())
	{
		ss.str("");
		ss << "Uv!";
		m_textRenderer.SetText(ss.str());
		m_textRenderer.Draw();
	}

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vP: " << m_currentVideoPos;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "aP: " << m_currentAudioPos;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "V-A: " <<  (m_currentVideoPos - m_currentAudioPos);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "dP: " << m_currentDevicePos;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "V-D: " <<  (m_currentVideoPos - m_currentDevicePos);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vQBS: " << (m_videoSink.GetByteQueueSize() / 1024.0f);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "aQBS: " << (m_audioSink.GetByteQueueSize() / 1024.0f);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vCF: " << (m_videoRendererStats->m_catchupFrames);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vRF: " << (m_videoRendererStats->m_repeatFrames);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vLF: " << (m_videoRendererStats->m_longFrames);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vNSF: " << (m_videoRendererStats->m_noSyncFrames);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	auto currentFrameType = m_videoRendererStats->m_currentFrameType;
	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vFT: ";
	ss << (currentFrameType == AVPictureType::AV_PICTURE_TYPE_I ? "I" : " ");
	ss << (currentFrameType == AVPictureType::AV_PICTURE_TYPE_P ? "P" : " ");
	ss << (currentFrameType == AVPictureType::AV_PICTURE_TYPE_B ? "B" : " ");
	if(currentFrameType != AVPictureType::AV_PICTURE_TYPE_I &&
		currentFrameType != AVPictureType::AV_PICTURE_TYPE_P &&
		currentFrameType != AVPictureType::AV_PICTURE_TYPE_B)
	{
		ss << "?";
	}
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vFC: " << (m_videoRendererStats->m_frameCounter);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "avg V-D: " << Average(m_diffDeviceHistory);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	float avgAudio = Average(m_diffAudioHistory);
	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "avg V-A: " << avgAudio;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();


	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "30ms V-A: " << (m_currentSyncDiff < 0.0 ? "" : " ") << m_currentSyncDiff;
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "V-C: " <<  (m_currentVideoPos - m_currentMasterClockPos);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "avg V-C: " << Average(m_diffClockHistory);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glTranslatef(0.0f, -25.0f, 0.0f);
	ss.str("");
	ss << "vEF: " << (m_videoRendererStats->m_excessiveDesync);
	m_textRenderer.SetText(ss.str());
	m_textRenderer.Draw();

	glPopMatrix();
}

void VideoPlayerEngine::InitMediaInfo()
{
	std::ostringstream ss;

	if(m_audioStreamAvailable)
	{
		char audioTag[5];
		memcpy(audioTag, &m_audioCodecCtx->codec_tag, 4);
		audioTag[4] = '\0';
		ss.str("");
		ss << "A: " << ffutil::CodecIdToString(m_audioCodecCtx->codec_id) << " " <<
			"(" << audioTag << "), " <<
			m_audioCodecCtx->sample_rate << " Hz, " <<
			m_audioCodecCtx->channels << " chns, " <<
			ffutil::ChannelLayoutToString(m_audioCodecCtx->channel_layout) << ", " <<
			ffutil::SampleFormatToString(m_audioCodecCtx->sample_fmt) << ", " <<
			(av_q2d(m_formatCtx->streams[m_audioStreamIndex]->time_base) * m_formatCtx->streams[m_audioStreamIndex]->duration) << "s";
		m_mediaInfoAudio = ss.str();
	}

	char videoTag[5];
	memcpy(videoTag, &m_videoCodecCtx->codec_tag, 4);
	videoTag[4] = '\0';
	const char *profile = av_get_profile_name(m_videoCodecCtx->codec, m_videoCodecCtx->profile);
	if(profile == NULL)
	{
		profile = "";
	}
	bool hasBFrames = m_videoCodecCtx->has_b_frames != 0;
	ss.str("");
	ss << "V: " << ffutil::CodecIdToString(m_videoCodecCtx->codec_id) << " " <<
		"(" << profile << ") (" << videoTag << "), " <<
		ffutil::PixelFormatToString(m_videoCodecCtx->pix_fmt) << ", " <<
		m_videoCodecCtx->width << "x" << m_videoCodecCtx->height << ", " <<
		(1.0f / (av_q2d(m_videoCodecCtx->time_base) * (float)m_videoCodecCtx->ticks_per_frame)) << " fps" << ", " <<
		(hasBFrames ? "w/ B-f" : "wo/ B-f") << ", " <<
		(av_q2d(m_formatCtx->streams[m_videoStreamIndex]->time_base) * m_formatCtx->streams[m_videoStreamIndex]->duration) << "s";
	m_mediaInfoVideo = ss.str();

	ss.str("");
	ss << "C: " <<  m_formatCtx->duration / 1000000.0 << "s";
	m_mediaInfoContainer = ss.str();
}

void VideoPlayerEngine::DrawMediaInfo()
{
	glPushMatrix();

	glTranslatef(10.0f, 10.0f + 20.0f, 0.0f);
	m_textRenderer.SetText(m_mediaInfoAudio);
	m_textRenderer.Draw();

	glTranslatef(0.0f, 25.0f, 0.0f);
	m_textRenderer.SetText(m_mediaInfoVideo);
	m_textRenderer.Draw();

	glTranslatef(0.0f, 25.0f, 0.0f);
	m_textRenderer.SetText(m_mediaInfoContainer);
	m_textRenderer.Draw();

	glPopMatrix();
}

void VideoPlayerEngine::Color4ubPM(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	float fR = r / 255.0f;
	float fG = g / 255.0f;
	float fB = b / 255.0f;
	float fA = a / 255.0f;

	glColor4f(fR * fA, fG * fA, fB * fA, fA);
}

double VideoPlayerEngine::Average(CircularHistory<double> &history)
{
	double sum = 0;

	for(int i = 0; i < history.Count(); i++)
	{
		sum += history.Get(i);
	}

	return sum / (double)history.Count();
}

void VideoPlayerEngine::DrawMarker(float mPos, float yPos, float markerSize)
{
	float duration = m_formatCtx->duration / 1000000.0f;
	float xPos = (float)GetWidth() * mPos / duration;

	glPushMatrix();
	glTranslatef(0.0, yPos, 0.0f);
	glTranslatef(xPos - markerSize / 2.0f, 0.0f, 0.0f);
	glScalef(markerSize, markerSize, 0.0f);
	DrawTexturedQuad();
	glPopMatrix();
}

