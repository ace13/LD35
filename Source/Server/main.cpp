#include "Application.hpp"

Application* server_app = nullptr;

#ifdef SFML_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HANDLE hEv = nullptr;
#define EXPORT _declspec(dllexport)
#else
#define EXPORT
#endif

int main(int argc, char** argv)
{
#ifdef SFML_SYSTEM_WINDOWS
	if (hEv)
	{
		SetEvent(hEv);
		while (hEv)
		{
			Sleep(1000);
		}

		return 0;
	}
#endif

	server_app = new Application();
	auto& app = *server_app;

	app.init();

	for (int i = 1; i < argc; ++i)
	{

	}

	app.run();

	delete server_app;

	return 0;
}

extern "C"
{
#ifdef SFML_SYSTEM_WINDOWS
	EXPORT void WinMainCRTStartup();
	EXPORT void e0(HANDLE h)
	{
		hEv = h;
		WinMainCRTStartup();
	}
#endif

	EXPORT void server_init()
	{
		if (server_app)
			return;

		server_app = new Application();
		server_app->init();

		server_app->setBoolProp("headless", true);
	}
	EXPORT void server_cmd(const char* cmd)
	{
		if (!server_app)
			return;

		server_app->runCommand(cmd);
	}
	EXPORT void server_setBProp(const char* name, bool value)
	{
		if (!server_app)
			return;

		server_app->setBoolProp(name, value);
	}
	EXPORT void server_setIProp(const char* name, int value)
	{
		if (!server_app)
			return;

		server_app->setIntProp(name, value);
	}
	EXPORT void server_setFProp(const char* name, float value)
	{
		if (!server_app)
			return;

		server_app->setFloatProp(name, value);
	}
	EXPORT bool server_getBProp(const char* name)
	{
		if (!server_app)
			return false;

		return server_app->getBoolProp(name);
	}
	EXPORT int server_getIProp(const char* name)
	{
		if (!server_app)
			return 0;

		return server_app->getIntProp(name);
	}
	EXPORT float server_getFProp(const char* name)
	{
		if (!server_app)
			return 0.f;

		return server_app->getFloatProp(name);
	}
	EXPORT void server_run()
	{
		if (!server_app)
			return;

		server_app->run();
	}
	EXPORT void server_stop()
	{
		if (!server_app)
			return;

		server_app->stop();
		hEv = nullptr;
	}
	EXPORT int server_state()
	{
		if (!server_app)
			return Application::State_Stopped;
		return server_app->getState();
	}
	EXPORT void server_reset()
	{
		if (!server_app)
			return;

		delete server_app;
		server_app = nullptr;
	}

}