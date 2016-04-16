#pragma once

#include <string>

class ServerContainer
{
public:
	enum State
	{
		State_Invalid = -1,

		State_Stopped = 0,
		State_Starting,
		State_Running,
		State_Stopping
	};

	ServerContainer();
	ServerContainer(const ServerContainer&) = delete;
	~ServerContainer();

	ServerContainer& operator=(const ServerContainer&) = delete;

	bool init();
	void launch();
	void tick();
	void stop();

	uint16_t getPort() const;

	void runCmd(const std::string& cmd);

private:
	State mServerState;

	typedef void* ServerModuleHandle;
	ServerModuleHandle mServerModule;

	typedef void(*ServerInit_f)();
	ServerInit_f mServerInit;
	typedef void(*ServerRun_f)();
	ServerRun_f mServerRun;
	typedef void(*ServerStop_f)();
	ServerStop_f mServerStop;
	typedef int(*ServerStatus_f)();
	ServerStatus_f mServerStatus;
	typedef void(*ServerReset_f)();
	ServerReset_f mServerReset;
	typedef void(*ServerRunCmd_f)(const char*);
	ServerRunCmd_f mServerRunCmd;
	typedef void(*ServerSetBProp_f)(const char*, bool);
	ServerSetBProp_f mServerSetBProp;
	typedef void(*ServerSetIProp_f)(const char*, int);
	ServerSetIProp_f mServerSetIProp;
	typedef void(*ServerSetFProp_f)(const char*, float);
	ServerSetFProp_f mServerSetFProp;
	typedef bool(*ServerGetBProp_f)(const char*);
	ServerGetBProp_f mServerGetBProp;
	typedef int(*ServerGetIProp_f)(const char*);
	ServerGetIProp_f mServerGetIProp;
	typedef float(*ServerGetFProp_f)(const char*);
	ServerGetFProp_f mServerGetFProp;
};
