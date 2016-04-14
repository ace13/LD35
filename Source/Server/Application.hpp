#pragma once

#include "ConnectionManager.hpp"

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Network/TcpListener.hpp>

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
	ConnectionManager mConnections;
	ScriptManager mClientSM;

	sf::TcpListener mSocket;

	std::thread mWorkThread;
	bool mRunning;
};
