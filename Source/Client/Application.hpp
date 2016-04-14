#pragma once

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/TcpSocket.hpp>

class Application
{
public:
	Application();
	Application(const Application&) = delete;
	~Application();

	Application& operator=(const Application&) = delete;

	void init();

	void run();

	sf::RenderTarget& getRT();

private:
	Engine mEngine;
	sf::TcpSocket mSocket;
};
