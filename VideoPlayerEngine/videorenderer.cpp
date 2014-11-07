#include "videorenderer.h"

extern "C"
{
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <fstream>
#include <sstream>
#include <iostream>
#include "ffutils.h"
#include "iclocksource.h"
#include <ienginecontext.h>
#include <oglutils.h>


#define ENGINE_ROOT "C:\\Users\\Michael\\Documents\\Visual Studio 2012\\Projects\\OGLDesktop\\VideoPlayerEngine\\"
#define MAKE_PATH(file) ENGINE_ROOT file


VideoRenderer::VideoRenderer()
	: m_textureY(0),
	m_textureCb(0),
	m_textureCr(0),
	m_catchupFrames(0),
	m_dumpFrame(false),
	m_repeatFrames(0),
	m_noSyncFrames(0),
	m_currentVideoPos(0.0),
	m_frameCounter(0),
	m_syncRefreshTimer(0.030f),
	m_longFrames(0),
	m_masterClockSource(NULL),
	m_excessiveDesync(0),
	m_useYCbCrPath(false),
	m_textureRGBA(0),
	m_useScalableYCbCrPath(false),
	m_initialized(false)
{
}

bool VideoRenderer::Init(AVFormatContext *formatCtx, unsigned int streamIndex)
{
	m_formatCtx = formatCtx;
	m_videoStreamIndex = streamIndex;

	if(!InitVideo())
	{
		return false;
	}

	if(m_useYCbCrPath)
	{
		InitShader();

		if(m_useScalableYCbCrPath)
		{
			m_renderTargetYCbCr.create(m_frameWidth, m_frameHeight);
			const_cast<sf::Texture &>(m_renderTargetYCbCr.getTexture()).setSmooth(true);

			m_engineContext->DeactivateOGLContext();
			m_renderTargetYCbCr.setActive(true);

			oglutil::InitOpenGLOrthographic(m_frameWidth, m_frameHeight);
			BindTexturesToUnits();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glScalef(m_frameWidth, m_frameHeight, 0.0f);

			m_renderTargetYCbCr.setActive(false);
			m_engineContext->ActivateOGLContext();
		}
	}

	m_initialized = true;

	return true;
}

void VideoRenderer::DeInit()
{
	if(m_useYCbCrPath)
	{
		glDeleteTextures(1, &m_textureY);
		glDeleteTextures(1, &m_textureCb);
		glDeleteTextures(1, &m_textureCr);
	}
	else
	{
		glDeleteTextures(1, &m_textureRGBA);
	}
}

void VideoRenderer::Draw()
{
	glEnable(GL_TEXTURE_2D);

	glColor3ub(255, 255, 255);
	glPushMatrix();
	//glScalef(m_displayFrameWidth, m_displayFrameHeight, 0.0f);
	glScalef(m_frameWidth, m_frameHeight, 0.0f);

	if(m_useYCbCrPath)
	{
		if(!m_useScalableYCbCrPath)
		{
			sf::Shader::bind(&m_shader);
			DrawTexturedQuad();
			sf::Shader::bind(NULL);
		}
		else
		{
			m_engineContext->DeactivateOGLContext();
			m_renderTargetYCbCr.setActive(true);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glColor3ub(255, 255, 255);
			sf::Shader::bind(&m_shader);
			DrawTexturedQuad();
			sf::Shader::bind(NULL);

			m_renderTargetYCbCr.setActive(false);
			m_engineContext->ActivateOGLContext();

			sf::Texture::bind(&m_renderTargetYCbCr.getTexture());

			//glMatrixMode(GL_TEXTURE);
			//glLoadIdentity();

			DrawTexturedQuad();
			sf::Texture::bind(NULL);
		}
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glMatrixMode(GL_TEXTURE);
		//glLoadIdentity();
		////glScalef(1.0f, -1.0f, 1.0f);
		//glMatrixMode(GL_MODELVIEW);

		glBindTexture(GL_TEXTURE_2D, m_textureRGBA);
		DrawTexturedQuad2();
		glDisable(GL_BLEND);
	}

	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void VideoRenderer::Update(float deltaTime)
{
	m_frameUnderflowing = false;
	CheckFrameRefresh(deltaTime);
}

void VideoRenderer::SetMasterClock(IClockSource &source)
{
	m_masterClockSource = &source;
}

double VideoRenderer::GetPosition()
{
	return m_currentVideoPos;
}

int64_t VideoRenderer::GetCurrentPTS()
{
	return m_currentVideoPTS;
}

void VideoRenderer::QueueFrame(QueuedVideoFrame *frame)
{
	m_videoFrameQueue.enqueue(std::move(frame));
}

size_t VideoRenderer::FrameQueueSize()
{
	return m_videoFrameQueue.size();
}

void VideoRenderer::SetSeek()
{
	m_allFramesDisplayed = false;
	m_seeking = true;
}

void VideoRenderer::ClearSeek()
{
	m_allFramesDisplayed = false;
	m_seeking = false;
}

bool VideoRenderer::AllFramesDisplayed()
{
	return m_allFramesDisplayed;
}

int VideoRenderer::GetFrameWidth()
{
	return m_frameWidth;
}

int VideoRenderer::GetFrameHeight()
{
	return m_frameHeight;
}

void VideoRenderer::DumpNextFrame()
{
	m_dumpFrame = true;
}

bool VideoRenderer::GetUnderflowState()
{
	return m_frameUnderflowing;
}

void VideoRenderer::SetUseScalableYCbCrPath(bool state)
{
	if(m_initialized)
	{
		return;
	}

	m_useScalableYCbCrPath = state;
}

void VideoRenderer::SetEngineContext(IEngineContext *engineContext)
{
	m_engineContext = engineContext;
}

void VideoRenderer::SetObjectPool(ObjectPool<QueuedVideoFrame> &pool)
{
	m_objectPool = &pool;
}

void VideoRenderer::InitShader()
{
	m_shader.loadFromFile(MAKE_PATH("YCbCr-420p_to_RGB888.vert"), MAKE_PATH("YCbCr-420p_to_RGB888.frag"));

	BindTexturesToUnits();
}

void VideoRenderer::BindTexturesToUnits()
{
	sf::Shader::bind(&m_shader);
	//glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&m_programId);

	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);

	glUniform1iARB(glGetUniformLocationARB(program, "textureY"), 1);
	glActiveTextureARB(GL_TEXTURE0_ARB + 1);
	glBindTexture(GL_TEXTURE_2D, m_textureY);

	glUniform1iARB(glGetUniformLocationARB(program, "textureCb"), 2);
	glActiveTextureARB(GL_TEXTURE0_ARB + 2);
	glBindTexture(GL_TEXTURE_2D, m_textureCb);

	glUniform1iARB(glGetUniformLocationARB(program, "textureCr"), 3);
	glActiveTextureARB(GL_TEXTURE0_ARB + 3);
	glBindTexture(GL_TEXTURE_2D, m_textureCr);

	glActiveTextureARB(GL_TEXTURE0_ARB);

	int maxTextures;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &maxTextures);

	sf::Shader::bind(NULL);

	//glBindTexture(GL_TEXTURE_2D, m_textureY);
	//m_shader.setParameter("currentTexture", sf::Shader::CurrentTexture);
}

bool VideoRenderer::InitVideo()
{
	m_videoCodecCtx = m_formatCtx->streams[m_videoStreamIndex]->codec;

	if(m_videoCodecCtx->pix_fmt != PIX_FMT_YUV420P &&
		m_videoCodecCtx->pix_fmt != PIX_FMT_BGRA)
	{
		return false;
	}

	bool isYCbCr420p = m_videoCodecCtx->pix_fmt == PIX_FMT_YUV420P;
	bool hasBFrames = m_videoCodecCtx->has_b_frames != 0;

	m_useYCbCrPath = isYCbCr420p;
	InitVideoFrame(m_videoCodecCtx);

	const AVPixFmtDescriptor *pixDesc = av_pix_fmt_desc_get(m_videoCodecCtx->pix_fmt);
	int ffSize = avpicture_get_size(m_videoCodecCtx->pix_fmt, m_frameWidth, m_frameHeight);
	int bpc = 2;
	int mySize = m_frameWidth * m_frameHeight * bpc + 2 * (m_sampledChromaWidth * m_sampledChromaHeight * bpc);

	//m_frameInterval = (float)av_q2d(m_videoCodecCtx->time_base);
	//m_frameInterval *= (float)m_videoCodecCtx->ticks_per_frame;

	if(!(m_videoCodecCtx->codec_id == AV_CODEC_ID_GIF))
	{
		m_frameInterval = (float)av_q2d(av_guess_frame_rate(m_formatCtx, m_formatCtx->streams[m_videoStreamIndex], NULL));
		m_frameInterval = 1.0f / m_frameInterval;
	}
	else
	{
		m_frameInterval = (float)av_q2d(m_formatCtx->streams[m_videoStreamIndex]->avg_frame_rate);
		m_frameInterval = 1.0f / m_frameInterval;
	}

	m_nextFrameInterval = 1.0f;
	m_frameTimer = 1.0f;
	//m_frameTimer = 0.0f;

	return true;
}

void VideoRenderer::InitVideoFrame(AVCodecContext *videoCodecCtx)
{
	m_frameWidth = videoCodecCtx->width;
	m_frameHeight = videoCodecCtx->height;
	m_displayFrameWidth = m_frameWidth * 1.0f;
	m_displayFrameHeight = m_frameHeight * 1.0f;

	if(m_useYCbCrPath)
	{
		m_sampledChromaWidth = m_frameWidth / 2;
		m_sampledChromaHeight = m_frameHeight / 2;

		CreatePlaneTexture(m_textureY, m_frameWidth, m_frameHeight);

		CreatePlaneTexture(m_textureCb, m_sampledChromaWidth, m_sampledChromaHeight);
		CreatePlaneTexture(m_textureCr, m_sampledChromaWidth, m_sampledChromaHeight);
	}
	else
	{
		CreateRGBATexture(m_textureRGBA, m_frameWidth, m_frameHeight);
	}
}

void VideoRenderer::UpdateVideoFrameYCbCr(QueuedVideoFrame *frame)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, m_textureY);
	// 16bpc
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneYStride / 2);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_frameWidth, m_frameHeight, GL_RED, GL_UNSIGNED_SHORT, frame->PlaneY.data());
	// 8bpc
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneYStride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_frameWidth, m_frameHeight, GL_RED, GL_UNSIGNED_BYTE, frame->PlaneY.data());

	glBindTexture(GL_TEXTURE_2D, m_textureCb);
	// 16bpc
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneCbStride / 2);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_sampledChromaWidth, m_sampledChromaHeight, GL_RED, GL_UNSIGNED_SHORT, frame->PlaneCb.data());
	// 8bpc
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneCbStride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_sampledChromaWidth, m_sampledChromaHeight, GL_RED, GL_UNSIGNED_BYTE, frame->PlaneCb.data());

	glBindTexture(GL_TEXTURE_2D, m_textureCr);
	// 16bpc
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneCrStride / 2);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_sampledChromaWidth, m_sampledChromaHeight, GL_RED, GL_UNSIGNED_SHORT, frame->PlaneCr.data());
	// 8bpc
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->PlaneCrStride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_sampledChromaWidth, m_sampledChromaHeight, GL_RED, GL_UNSIGNED_BYTE, frame->PlaneCr.data());

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void VideoRenderer::UpdateVideoFrameRGBA(QueuedVideoFrame *frame)
{
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, m_textureRGBA);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->RGBAStride / 4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_frameWidth, m_frameHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frame->RGBABytes.data());

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void VideoRenderer::UpdateVideoFrame(QueuedVideoFrame *frame)
{
	if(m_useYCbCrPath)
	{
		UpdateVideoFrameYCbCr(frame);
	}
	else
	{
		UpdateVideoFrameRGBA(frame);
	}
}

void VideoRenderer::DrawTexturedQuad()
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

void VideoRenderer::DrawTexturedQuad2()
{
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 0.0f);
	glEnd();
}

#define SYNC_THRESHOLD 0.02
#define SYNC_THRESHOLD_F 0.02f
#define NOSYNC_THRESHOLD 10.0
#define NOSYNC_THRESHOLD_F 10.0f

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.0001
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.5
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

void VideoRenderer::CheckFrameRefresh(float deltaTime)
{
	m_frameTimer += deltaTime;

	if(m_frameTimer >= m_nextFrameInterval)
	{
		if(!m_videoFrameQueue.empty())
		{
			double remainder = m_frameTimer - m_nextFrameInterval;
			m_frameTimer = remainder;

			QueuedVideoFrame *newVideoFrame = m_videoFrameQueue.dequeue();

			//double nextDelay = m_frameInterval;
			double nextDelay = newVideoFrame->Duration * av_q2d(m_formatCtx->streams[m_videoStreamIndex]->time_base);

			double diff = m_currentVideoPos - m_masterClockSource->GetPosition();
			//double diff = m_currentVideoPos - m_currentDevicePos;
			double sync_threshold  = std::max(AV_SYNC_THRESHOLD_MIN, std::min(AV_SYNC_THRESHOLD_MAX, nextDelay));
			//double sync_threshold = 0.01;
			if (fabs(diff) < NOSYNC_THRESHOLD)
			{
				if (diff <= -sync_threshold)
				{
					nextDelay = std::max(0.0, nextDelay + diff);
					m_catchupFrames++;

					//m_frameTimer = 0.0f;
				}
				else if (diff >= sync_threshold && nextDelay > AV_SYNC_FRAMEDUP_THRESHOLD)
				{
					nextDelay = nextDelay + diff;
					m_longFrames++;

					//m_frameTimer = 0.0f;
				}
				else if (diff >= sync_threshold)
				{
					nextDelay = 2 * nextDelay;
					m_repeatFrames++;

					//m_frameTimer = 0.0f;
				}
				else
				{
					m_noSyncFrames++;
				}
			}
			else
			{
				// handle excessive desync.
				m_excessiveDesync++;
			}

			m_nextFrameInterval = nextDelay;

			m_currentFrameType = (AVPictureType)newVideoFrame->Type;
			m_currentVideoPTS = newVideoFrame->PTS;
			m_currentVideoPos = av_q2d(m_formatCtx->streams[m_videoStreamIndex]->time_base) * newVideoFrame->PTS;

			UpdateVideoFrame(newVideoFrame);
			if(m_dumpFrame)
			{
				DumpFrame(newVideoFrame);
				m_dumpFrame = false;
			}

			m_frameCounter++;
			m_objectPool->ReturnInstance(newVideoFrame);
			//std::cout << "Return" << std::endl;
		}
		else
		{
			if(m_seeking)
			{
				m_allFramesDisplayed = true;
			}
			else
			{
				m_frameUnderflowing = true;
			}
		}
	}
//	else
//	{
//		if(!m_endOfMedia)
//		{
//			std::cout << " Uv?";
//		}
//	}
}

void VideoRenderer::CreatePlaneTexture(GLuint &texture, int width, int height)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// 8bpc
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	// 16bpc
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
}

void VideoRenderer::CreateRGBATexture(GLuint &texture, int width, int height)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void VideoRenderer::DumpFrame(QueuedVideoFrame *frame)
{
	int linesizes[4];
	av_image_fill_linesizes(linesizes, m_videoCodecCtx->pix_fmt, m_videoCodecCtx->width);
	DWORD pid = GetCurrentProcessId();
	uint64_t uid = frame->PTS;

	if(m_useYCbCrPath)
	{
		{
			std::ostringstream ss;
			ss << "dump_P" << pid << "_U" << uid << "_" << m_frameWidth << "x" << m_frameHeight << "_ycbcr420p.y.raw";
			std::ofstream outputY(ss.str(), std::ios::out | std::ios::trunc | std::ios::binary);

			bool fileOk = outputY.is_open();

			for(int i = 0; i < m_frameHeight; i++)
			{
				outputY.write(((char *)frame->PlaneY.data()) + (frame->PlaneYStride * i),
					linesizes[0]
				);
			}
			outputY.close();
		}

		{
			std::ostringstream ss;
			ss << "dump_P" << pid << "_U" << uid << "_" << m_sampledChromaWidth << "x" << m_sampledChromaHeight << "_ycbcr420p.cb.raw";
			std::ofstream outputCb(ss.str(), std::ios::out | std::ios::trunc | std::ios::binary);

			bool fileOk = outputCb.is_open();

			for(int i = 0; i < m_sampledChromaHeight; i++)
			{
				outputCb.write(((char *)frame->PlaneCb.data()) + (frame->PlaneCbStride * i),
					linesizes[1]
				);
			}
			outputCb.close();
		}

		{
			std::ostringstream ss;
			ss << "dump_P" << pid << "_U" << uid << "_" << m_sampledChromaWidth << "x" << m_sampledChromaHeight << "_ycbcr420p.cr.raw";
			std::ofstream outputCr(ss.str(), std::ios::out | std::ios::trunc | std::ios::binary);

			bool fileOk = outputCr.is_open();

			for(int i = 0; i < m_sampledChromaHeight; i++)
			{
				outputCr.write(((char *)frame->PlaneCr.data()) + (frame->PlaneCrStride * i),
					linesizes[2]
				);
			}
			outputCr.close();
		}
	}
	else
	{
		{
			std::ostringstream ss;
			ss << "dump_P" << pid << "_U" << uid << "_" << m_frameWidth << "x" << m_frameHeight << "_rgba.raw";
			std::ofstream outputRGBA(ss.str(), std::ios::out | std::ios::trunc | std::ios::binary);

			bool fileOk = outputRGBA.is_open();

			for(int i = 0; i < m_frameHeight; i++)
			{
				outputRGBA.write(((char *)frame->RGBABytes.data()) + (frame->RGBAStride * i),
					linesizes[0]
				);
			}
			outputRGBA.close();
		}
	}
}

