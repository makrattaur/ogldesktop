#include <memory>
#include <Windows.h>
#include <autohandle.h>
#include <autohandle_win32.h>

#include <genericenginerunner.h>
#include <enginehost.h>
#include <engine.h>


int main(int argc, char *argv[])
{
	std::string engineDLLName;

	//engineDLLName = "TestOGLEngine.dll";
	//engineDLLName = "VideoPlayerEngine.dll";
	engineDLLName = "MeshRendererEngine.dll";

	ModuleAutoHandle engineLib(LoadLibrary(engineDLLName.c_str()));
	if(engineLib.IsInvalid())
	{
		return 1;
	}

	ENGINE_PFN_CREATE_ENGINE creator = (ENGINE_PFN_CREATE_ENGINE)GetProcAddress(engineLib, "CreateEngine");
	if(creator == NULL)
	{
		return 1;
	}

	GenericEngineRunner runner(creator());
	//runner.Run(VideoMode(1280, 720), "Test OpenGL Window", ContextSettings(24, 8));
	auto engine = runner.GetEngine();
	runner.Run(VideoMode(engine->PreferredWidth(), engine->PreferredHeight()), engine->PreferredTitle(), ContextSettings(24, 8));

	return 0;
}

