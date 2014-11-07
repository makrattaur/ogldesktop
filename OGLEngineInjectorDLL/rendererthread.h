#ifndef H_RENDERTHREAD
#define H_RENDERTHREAD

#define NOMINMAX
#include <Windows.h>
#include "inc_ogl.h"
#include <memory>
#include <atomic>
#include <tuple>

#include <SFML\Window.hpp>
#include <SFML\System\Thread.hpp>

#include <threadsafequeue.h>
#include <utils_win32.h>
#include <autohandle_win32.h>

class InjectedEngineRunner;


class RendererThread
{
public:
	RendererThread();

	bool Init();
	void Start();
	void Stop();
	void Deinit();

	void QueueUpdate();
	void EmptyEventQueue();

	void SetIconsTexture(GLuint texture);
	GLuint GetCurrentWallpaperTexture();

	void SetEngine(InjectedEngineRunner &engine, int monitorIndex);
	void SetEnginePrimary(InjectedEngineRunner &engine);
	void ClearEngine(int monitorIndex);
	void ClearEnginePrimary();

	int GetMonitorWidth(int monitorIndex);
	int GetMonitorHeight(int monitorIndex);

	int GetPrimaryMonitorWidth();
	int GetPrimaryMonitorHeight();
private:
	class Message
	{
	public:
		enum MessageType
		{
			Unspecified,
			Close,
			Update,
			EmptyEventQueue,
			FadeWallpaper
		};

		Message();
		Message(MessageType msgType);

		MessageType Type;
	};

	void InitMonitors();
	void CheckMonitorShift();
	bool IsMonitorIndexValid(int monitorIndex);

	void ThreadProc();
	void ProcessMessages();
	void QueueMessage(Message::MessageType msg);

	void InitOpenGL();
	void DrawStep();
	void Draw();

	void DrawWallpaper();
	void DrawEngine(GLuint texture, util::Rectangle &rect);
	void DrawEngines();
	void DrawIcons();

	GLuint GetFrontWallpaperTexture();
	GLuint GetFrontWallpaperTexture(bool flip);
	GLuint GetBackWallpaperTexture();
	GLuint GetBackWallpaperTexture(bool flip);
	void GetFrontBackWallpaperTextures(GLuint &front, GLuint &back);
	void GetFrontBackWallpaperTextures(GLuint &front, GLuint &back, bool flip);
	void FlipWallpaperTextures();
	void FlipWallpaperTextures(bool currentFlip);

	HWND m_window;
	LONG_PTR m_oldUserData;
	int m_width;
	int m_height;
	sf::Window m_oglWindow;

	ThreadSafeQueue<Message> m_commandQueue;
	GenericAutoHandle m_messageEvent;
	std::unique_ptr<sf::Thread> m_thread;

	GLuint m_iconsTexture;
	GLuint m_frontWallpaperTexture;
	GLuint m_backWallpaperTexture;
	std::atomic<bool> m_flipWallpaperTextures;

	bool m_quitLoop;

	bool m_firstChange;
	float m_currentFadeAmount;
	bool m_fading;
	long long m_lastFadeTime;

	long long m_lastFlushTime;

	std::vector<util::Monitor> m_monitors;
	std::vector<std::tuple<InjectedEngineRunner *, bool>> m_engines;
};


#endif

