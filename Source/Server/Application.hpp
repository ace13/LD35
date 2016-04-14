#pragma once

#include <Core/Engine.hpp>

#include <thread>

class Application
{
public:
	Application();
	Application(const Application&) = default;
	Application(Application&&) = default;
	~Application() = default;

	Application& operator=(const Application&) = default;

	void init();
	void run();

private:
	void serverLoop();

	Engine mEngine;
	std::thread mWorkThread;
	bool mRunning;
};
