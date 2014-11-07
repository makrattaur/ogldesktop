#ifndef H_INJECTEDENGINERUNNER
#define H_INJECTEDENGINERUNNER

#include <memory>
#include <atomic>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Thread.hpp>

#include "rendererthread.h"

#include <enginerunnerbase.h>
#include <ienginecontext.h>


class InjectedEngineRunner : public EngineRunnerBase
{
public:
	InjectedEngineRunner(Engine *engine, RendererThread &rendererThread);
	~InjectedEngineRunner();
	bool Init(int width, int height);
	void Start();
	void Stop();
	GLuint GetRenderTargetTexture();
protected:
	virtual void ProcessEvents(bool &needsToQuit);
	virtual void SwapBuffers();
private:
	class CEngineContext : public IEngineContext
	{
	public:
		CEngineContext(InjectedEngineRunner &parent);
		virtual int GetWidth();
		virtual int GetHeight();
		virtual void ActivateOGLContext();
		virtual void DeactivateOGLContext();
	private:
		InjectedEngineRunner &m_parent;
	};

	struct GLExt
	{
		GLExt();
		void Init();
		PFNGLGENFRAMEBUFFERSEXTPROC m_glGenFramebuffersEXT;
		PFNGLBINDFRAMEBUFFEREXTPROC m_glBindFramebufferEXT;
		PFNGLGENRENDERBUFFERSEXTPROC m_glGenRenderbuffersEXT;
		PFNGLBINDRENDERBUFFEREXTPROC m_glBindRenderbufferEXT;
		PFNGLRENDERBUFFERSTORAGEEXTPROC m_glRenderbufferStorageEXT;
		PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC m_glFramebufferRenderbufferEXT;
		PFNGLFRAMEBUFFERTEXTURE2DEXTPROC m_glFramebufferTexture2DEXT;
		PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC m_glCheckFramebufferStatusEXT;
		PFNGLDELETERENDERBUFFERSEXTPROC m_glDeleteRenderbuffersEXT;
		PFNGLDELETEFRAMEBUFFERSEXTPROC m_glDeleteFramebuffersEXT;
	};

	class RenderTarget
	{
	public:
		RenderTarget();
		//~RenderTarget();

		void SetExtensions(GLExt &extensions);
		void Init(int width, int height);
		void DeInit();
		void MakeCurrent();
		GLuint GetTexture();
		static void MakeDefaultFramebufferCurrent(GLExt *glExt);
	private:
		GLExt *m_glExt;
		bool m_destroyed;
		GLuint m_depthBuffer;
		GLuint m_targetTexture;
		GLuint m_frameBuffer;
	};

	void ThreadProc();
	void FlipBuffers();
	RenderTarget &GetFrontRenderTarget();
	RenderTarget &GetBackRenderTarget();

	RendererThread &m_rendererThread;

	sf::Context m_context;

	GLExt m_glExt;
	std::atomic<bool> m_flipTargets;
	RenderTarget m_rt1;
	RenderTarget m_rt2;

	std::unique_ptr<sf::Thread> m_thread;
	std::atomic<bool> m_quit;
	int m_width;
	int m_height;

	CEngineContext m_engineContext;
};


#endif

