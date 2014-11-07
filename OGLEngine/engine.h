#ifndef G_H_ENGINE
#define G_H_ENGINE

#include "enginecommon.h"
#include "input.h"

class IEngineContext;


class ENGINE_API Engine
{
public:
	Engine();
	virtual ~Engine();

	virtual bool Init();
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void DeInit();

	int GetWidth();
	int GetHeight();

	bool GetQuitState();

	void SignalKeyDown(KeyboardKey key);
	void SignalKeyUp(KeyboardKey key);
	void SignalMouseButtonDown(MouseButton button, int x, int y);
	void SignalMouseButtonUp(MouseButton button, int x, int y);
	void SignalMouseWheelMove(int delta, int x, int y);
	void SignalMouseMove(int x, int y);
	void SignalResize(int newW, int newH);

	void SignalClose();
	void SetEngineContext(IEngineContext &context);

	virtual int PreferredWidth();
	virtual int PreferredHeight();
	virtual const char *PreferredTitle();
protected:
	void QuitEngine();
	virtual void OnKeyDown(KeyboardKey key);
	virtual void OnKeyUp(KeyboardKey key);
	virtual void OnMouseButtonDown(MouseButton button, int x, int y);
	virtual void OnMouseButtonUp(MouseButton button, int x, int y);
	virtual void OnMouseWheelMove(int delta, int x, int y);
	virtual void OnMouseMove(int x, int y);
	virtual void OnResize(int newW, int newH);
	virtual void OnClose();

	void ActivateOGLContext();
	void DeactivateOGLContext();
	IEngineContext *GetEngineContext();
private:
	int m_width;
	int m_height;

	bool m_hasEngineContext;
	IEngineContext *m_engineContext;

	bool m_quit;
};


#endif

