#ifndef H_SUBTITLERENDERER
#define H_SUBTITLERENDERER

extern "C"
{
#include <libavformat/avformat.h>
}

#include <SFML/Graphics/Font.hpp>

#include "isubtitlequeue.h"
#include <threadsafequeue.h>
#include "textrenderer.h"

class IClockSource;


class SubtitleRenderer : public ISubtitleQueue
{
public:
	SubtitleRenderer();
	void Init(AVFormatContext *formatCtx, unsigned int streamIndex);
	void Update(float deltaTime);
	void Draw();
	virtual void QueueSubtitle(std::unique_ptr<QueuedSubtitle> frame);
	virtual void OnSeek();
	void SetMasterClock(IClockSource &source);
private:
	void DequeueSubtitle();
	double GetSubtitleTime(std::unique_ptr<QueuedSubtitle> &frame);

	AVFormatContext *m_formatCtx;
	unsigned int m_subtitileStreamIndex;

	ThreadSafeQueue<std::unique_ptr<QueuedSubtitle>> m_subtitleQueue;

	TextRenderer m_textRenderer;
	sf::Font m_font;

	IClockSource *m_masterClock;

	std::unique_ptr<QueuedSubtitle> m_currentSubtitle;

	bool m_displaySubtitile;
	float m_displayTimer;
};

#endif
