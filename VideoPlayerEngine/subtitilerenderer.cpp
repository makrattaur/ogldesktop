#include "subtitlerenderer.h"
#include "iclocksource.h"


SubtitleRenderer::SubtitleRenderer()
	: m_displaySubtitile(false)
{
}

void SubtitleRenderer::Init(AVFormatContext *formatCtx, unsigned int streamIndex)
{
	m_formatCtx = formatCtx;
	m_subtitileStreamIndex = streamIndex;

	m_font.loadFromFile("C:\\Windows\\Fonts\\DejaVuSansMono.ttf");
	m_textRenderer.SetSize(20);
	m_textRenderer.SetFont(m_font);
	m_textRenderer.SetText("qwertyuiop");
}

void SubtitleRenderer::Update(float deltaTime)
{
	if(!m_displaySubtitile)
	{
		if(!m_currentSubtitle)
		{
			DequeueSubtitle();
		}

		if(m_currentSubtitle)
		{
			float subTime = GetSubtitleTime(m_currentSubtitle) / 1000.0f;
			float masterClock = m_masterClock->GetPosition();
			if(masterClock >= subTime)
			{
				m_displaySubtitile = true;
				m_textRenderer.SetText(m_currentSubtitle->String);
				m_displayTimer = masterClock - subTime;
			}
		}
	}
	else
	{
		m_displayTimer += deltaTime;
		if(m_displayTimer >= (m_currentSubtitle->Duration / 1000.0f))
		{
			m_displaySubtitile = false;
			m_currentSubtitle.reset();
		}
	}
}

void SubtitleRenderer::Draw()
{
	if(m_displaySubtitile && m_currentSubtitle)
	{
		m_textRenderer.Draw();
	}
}

void SubtitleRenderer::QueueSubtitle(std::unique_ptr<QueuedSubtitle> frame)
{
	m_subtitleQueue.enqueue(std::move(frame));
}

void SubtitleRenderer::OnSeek()
{
	m_currentSubtitle.reset();
	m_displaySubtitile = false;
	
	std::unique_ptr<QueuedSubtitle> tmp;
	while(m_subtitleQueue.tryMoveDequeue(tmp))
	{
	}
}

void SubtitleRenderer::SetMasterClock(IClockSource &source)
{
	m_masterClock = &source;
}

void SubtitleRenderer::DequeueSubtitle()
{
	m_currentSubtitle.reset();
	m_subtitleQueue.tryMoveDequeue(m_currentSubtitle);
}

double SubtitleRenderer::GetSubtitleTime(std::unique_ptr<QueuedSubtitle> &subtitle)
{
	return av_q2d(m_formatCtx->streams[m_subtitileStreamIndex]->time_base) * subtitle->PTS;
}
