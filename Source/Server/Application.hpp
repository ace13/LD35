#pragma once

#include "ConnectionManager.hpp"

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>
#include <Shared/NetworkedObject.hpp>

#include <SFML/Network/TcpListener.hpp>

#include <thread>
#include <unordered_map>

class Application
{
public:
	enum ServerState
	{
		State_Stopped,
		State_Starting,
		State_Running,
		State_Stopping
	};

	Application();
	Application(const Application&) = default;
	Application(Application&&) = default;
	~Application() = default;

	Application& operator=(const Application&) = default;

	void init();
	void run();
	void stop();

	void runCommand(const std::string& cmd);
	ServerState getState() const;

	void setBoolProp(const std::string& name, bool value);
	void setIntProp(const std::string& name, int value);
	void setFloatProp(const std::string& name, float value);
	bool getBoolProp(const std::string& name) const;
	int getIntProp(const std::string& name) const;
	float getFloatProp(const std::string& name) const;

private:
	struct ServerProperty
	{
		ServerProperty();
		explicit ServerProperty(bool);
		explicit ServerProperty(int);
		explicit ServerProperty(float);

		enum PropertyType
		{
			Type_Invalid,

			Type_Bool,
			Type_Int,
			Type_Float

		} Type;

		union
		{
			bool Bool;
			int Int;
			float Float;
		};

		std::function<void()> Callback;
	};

	void serverLoop();

	Engine mEngine;
	ConnectionManager mConnections;
	ScriptManager mClientSM;

	sf::TcpListener mSocket;

	std::unordered_map<std::string, ServerProperty> mProperties;
	std::unordered_map<int32_t, NetworkedObject> mObjects;

	std::thread mWorkThread;
	bool mRunning;
	ServerState mState;
};
