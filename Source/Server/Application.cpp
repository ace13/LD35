#include "Application.hpp"

#include <Core/FileWatcher.hpp>
#include <Core/Math.hpp>
#include <Core/Time.hpp>
#include <Core/ScriptManager.hpp>

#include <Core/AS_Addons/scriptarray/scriptarray.h>
#include <Core/AS_Addons/scripthelper/scripthelper.h>
#include <Core/AS_Addons/scriptmath/scriptmath.h>
#include <Core/AS_Addons/scriptstdstring/scriptstdstring.h>
#include <Core/AS_SFML/AS_SFML.hpp>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <codecvt>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _DEBUG
#include <fstream>
#endif

namespace
{
	struct CArrayType : public CUserType
	{
		void Store(CSerializedValue *val, void *ptr)
		{
			CScriptArray *arr = (CScriptArray*)ptr;
			for (unsigned int i = 0; i < arr->GetSize(); i++)
				val->m_children.push_back(new CSerializedValue(val, "", "", arr->At(i), arr->GetElementTypeId()));
		}
		void Restore(CSerializedValue *val, void *ptr)
		{
			CScriptArray *arr = (CScriptArray*)ptr;
			arr->Resize(val->m_children.size());
			for (size_t i = 0; i < val->m_children.size(); ++i)
				val->m_children[i]->Restore(arr->At(i), arr->GetElementTypeId());
		}
	};

	template<typename T>
	void print(T in)
	{
		std::cout << in;
	}

#ifdef WIN32
	template<>
	void print(const std::string& str)
	{
		typedef std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		std::wcout << convert().from_bytes(str);
	}
#endif

	void printany(const void* ref, int type)
	{
		asIScriptContext* ctx = asGetActiveContext();
		asIObjectType* objType = ctx->GetEngine()->GetObjectTypeById(type);
		asIScriptFunction* func = objType->GetMethodByDecl("string ToString() const");
		std::string output;

		if (func)
		{
			asIScriptContext* callCtx = ctx->GetEngine()->RequestContext();
			callCtx->Prepare(func);
			callCtx->SetObject(const_cast<void*>(ref));
			callCtx->Execute();

			output = *reinterpret_cast<std::string*>(callCtx->GetReturnAddress());

			ctx->GetEngine()->ReturnContext(callCtx);
		}
		else
		{
			std::ostringstream oss;

			if (type & asTYPEID_OBJHANDLE)
				oss << "@";

			oss << objType->GetName();

			if (type & asTYPEID_SCRIPTOBJECT)
				oss << "[SO]";

			oss << "{" << ref << "}";

			output = oss.str();
		}

		std::cout << output;
	}

	template<typename T>
	void println(T in)
	{
		std::cout << in << std::endl;
	}

#ifdef WIN32
	template<>
	void println(const std::string& str)
	{
		typedef std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		std::wcout << convert().from_bytes(str) << std::endl;
	}
#endif

	void printlnany(void* ref, int type)
	{
		printany(ref, type);
		std::cout << std::endl;
	}

	void println()
	{
		std::cout << std::endl;
	}

	enum EventTypes : uint16_t
	{
		/* Script data
		  
		   <string> Module
		   <char[]> Bytecode
		*/
		PacketType_Script = 0x0597,

		/* Player information

		   <uint32> ID
		*/
		PacketType_Player = 0x14A3,

		/* Object creation
		
		   <uint32> ID
		   <uint32> OwnerID
		   <string> Module
		   <string> Name
		*/
		PacketType_Create = 0xC347,

		/* Object destruction

		   <uint32> ID
		*/
		PacketType_Destroy = 0xDE57,

		/* Object update
		
		   Repeating:
		   <uint32> ID
		   <uint8> PropertyCount

		   Repeating:
		   <uint8> PropertyID
		   <uint8> Bytes
		   <char[Bytes]> Memory
		*/
		PacketType_Update = 0x6473
	};
}

Application::ServerProperty::ServerProperty()
	: Type(Type_Invalid)
	, Bool(false)
{

}
Application::ServerProperty::ServerProperty(bool b)
	: Type(Type_Bool)
	, Bool(b)
{

}
Application::ServerProperty::ServerProperty(int i)
	: Type(Type_Int)
	, Int(i)
{

}
Application::ServerProperty::ServerProperty(float f)
	: Type(Type_Float)
	, Float(f)
{

}

Application::Application()
	: mProperties {
		{ "headless", ServerProperty(false) },
		{ "port",     ServerProperty(42035) },
		{ "tickrate", ServerProperty(66)    },
		{ "local",    ServerProperty(false) }
	}
{
	mEngine.add<ScriptManager>();
	mEngine.add<FileWatcher>();

	mProperties.at("local").Callback = [this]() {
		puts("TODO: Switch interface");
	};
}

void Application::init()
{
	mEngine.init();

	auto& man = mEngine.get<ScriptManager>();

	man.addExtension("Array", [](asIScriptEngine* eng) { RegisterScriptArray(eng, true); });
	man.registerSerializedType("array", []() {
		return new CArrayType();
	});
	man.addExtension("Math", RegisterScriptMath);
	man.addExtension("String", RegisterStdString);
	man.registerSerializedType<std::string>("string");
	man.addExtension("String Utils", RegisterStdStringUtils);
	man.addExtension("Printing", [](asIScriptEngine* eng) {
		AS_ASSERT(eng->RegisterGlobalFunction("void print(int)", asFUNCTION(print<int>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void print(float)", asFUNCTION(print<float>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void print(string&in)", asFUNCTION(print<const std::string&>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void print(const ?&in)", asFUNCTION(printany), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void println()", asFUNCTIONPR(println, (), void), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void println(int)", asFUNCTION(println<int>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void println(float)", asFUNCTION(println<float>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void println(string&in)", asFUNCTION(println<const std::string&>), asCALL_CDECL));
		AS_ASSERT(eng->RegisterGlobalFunction("void println(const ?&in)", asFUNCTION(printlnany), asCALL_CDECL));
	});
	Time::registerTimeTypes(man);
	Math::registerScriptData(man);
	as::SFML::registerTypes(man);
	man.addExtension("ScriptHooks", [&](asIScriptEngine* eng) {
		AS_ASSERT(eng->SetDefaultNamespace("Hooks"));

		AS_ASSERT(eng->RegisterGlobalFunction("void Add(const string&in, const string&in)", asMETHOD(ScriptManager, addHookFromScript), asCALL_THISCALL_ASGLOBAL, &man));
		AS_ASSERT(eng->RegisterGlobalFunction("void Remove(const string&in, const string&in = \"\")", asMETHOD(ScriptManager, removeHookFromScript), asCALL_THISCALL_ASGLOBAL, &man));

		AS_ASSERT(eng->SetDefaultNamespace(""));
	});
	man.addExtension("PacketSend", [this](asIScriptEngine* eng) {
		AS_ASSERT(eng->SetDefaultNamespace("Net"));

		AS_ASSERT(eng->RegisterGlobalFunction("void SendToAll(const sf::Packet&in)", asMETHOD(ConnectionManager, sendPacketToAll), asCALL_THISCALL_ASGLOBAL, &mConnections));
		AS_ASSERT(eng->RegisterGlobalFunction("void SendToAllBut(int, const sf::Packet&in)", asMETHOD(ConnectionManager, sendPacketToAllBut), asCALL_THISCALL_ASGLOBAL, &mConnections));
		AS_ASSERT(eng->RegisterGlobalFunction("void SendTo(int, const sf::Packet&in)", asMETHOD(ConnectionManager, sendPacketTo), asCALL_THISCALL_ASGLOBAL, &mConnections));

		AS_ASSERT(eng->SetDefaultNamespace(""));
	});

	man.init();
	mClientSM.initBare();

	man.addDefine("SERVER");
	mClientSM.addDefine("CLIENT");
	mClientSM.setSerialization(false);

	man.registerHook("Tick", "void f(const Timespan&in)");

	man.addPostLoadCallback("OnLoad", [](asIScriptModule* mod) {
		if (mod->GetUserData(ScriptManager::Data_Reloaded) != (void*)0)
			return;

		auto onLoad = mod->GetFunctionByName("OnLoad");

		if (onLoad)
		{
			auto ctx = mod->GetEngine()->RequestContext();
			ctx->Prepare(onLoad);
			ctx->Execute();
			ctx->Unprepare();
			mod->GetEngine()->ReturnContext(ctx);
		}
	});
	man.addPostLoadCallback("OnReload", [](asIScriptModule* mod) {
		if (mod->GetUserData(ScriptManager::Data_Reloaded) != (void*)1)
			return;

		auto onReload = mod->GetFunctionByName("OnReload");

		if (onReload)
		{
			auto ctx = mod->GetEngine()->RequestContext();
			ctx->Prepare(onReload);
			ctx->Execute();
			ctx->Unprepare();
			mod->GetEngine()->ReturnContext(ctx);
		}
	});
	// Send updated script code to clients.
	man.addPostLoadCallback("ClientTransfer", [&](asIScriptModule* mod) {
		std::string file = mod->GetName();
		if (mClientSM.loadFromFile(file))
		{
			mod = mClientSM.getEngine()->GetModule(file.c_str());

			ScriptManager::BytecodeStore store;
			mod->SaveByteCode(&store);
			
			sf::Packet packet;
			packet << (uint16_t)PacketType_Script;
			packet << file;
			packet.append(store.getData(), store.getSize());

			mConnections.sendPacketToAll(packet);
		}
	});

	std::ifstream config("ClientEngineConfig.txt");
	if (config)
		ConfigEngineFromStream(mClientSM.getEngine(), config);
	else
	{
		std::cout << "No client configuration is available, this is a problem..." << std::endl;
	}

	// Load scripts;
	std::list<std::string> files;
	FileWatcher::recurseDirectory(".", files, "*.as");

	for (auto& entry : files)
	{
		if (entry.substr(entry.length() - 3) != ".as")
			continue;

		man.loadFromFile(entry);
	}

	auto& watch = mEngine.get<FileWatcher>();

	watch.addSource(".", true);
}

void Application::run()
{
	if (mState > State_Stopped)
		return;

	mState = State_Starting;
	mRunning = true;

	if (mProperties["local"].Bool)
		mSocket.listen(mProperties["port"].Int, sf::IpAddress::LocalHost);
	else
		mSocket.listen(mProperties["port"].Int);

	std::cout << "Spinning up worker thread..." << std::endl;
	mWorkThread = std::thread(&Application::serverLoop, this);

	if (!mProperties["headless"].Bool)
	{
		std::cout << "Server online." << std::endl;

		do
		{
			std::cout << "> ";
			std::string line;

			std::getline(std::cin, line);

			runCommand(line);
		} while (std::cin);

		mRunning = false;

		if (mWorkThread.joinable())
			mWorkThread.join();
	}
}

void Application::stop()
{
	if (mState > State_Stopped && mState < State_Stopping)
	{
		mState = State_Stopping;
		mRunning = false;

		if (mWorkThread.joinable())
			mWorkThread.join();
	}
}

void Application::runCommand(const std::string& cmd)
{

}

void Application::setBoolProp(const std::string& name, bool value)
{
	if (mProperties.count(name) == 0) return;
	if (mProperties.at(name).Type != ServerProperty::Type_Bool) return;

	auto& prop = mProperties.at(name);
	prop.Bool = value;
	if (prop.Callback)
		prop.Callback();
}
void Application::setIntProp(const std::string& name, int value)
{
	if (mProperties.count(name) == 0) return;
	if (mProperties.at(name).Type != ServerProperty::Type_Int) return;

	auto& prop = mProperties.at(name);
	prop.Int = value;
	if (prop.Callback)
		prop.Callback();
}
void Application::setFloatProp(const std::string& name, float value)
{
	if (mProperties.count(name) == 0) return;
	if (mProperties.at(name).Type != ServerProperty::Type_Float) return;

	auto& prop = mProperties.at(name);
	prop.Float = value;
	if (prop.Callback)
		prop.Callback();
}
bool Application::getBoolProp(const std::string& name) const
{
	if (mProperties.count(name) == 0) return false;
	if (mProperties.at(name).Type != ServerProperty::Type_Bool) return false;

	return mProperties.at(name).Bool;
}
int Application::getIntProp(const std::string& name) const
{
	if (mProperties.count(name) == 0) return 0;
	if (mProperties.at(name).Type != ServerProperty::Type_Int) return 0;

	return mProperties.at(name).Int;
}
float Application::getFloatProp(const std::string& name) const
{
	if (mProperties.count(name) == 0) return 0.f;
	if (mProperties.at(name).Type != ServerProperty::Type_Float) return 0.f;

	return mProperties.at(name).Float;
}

Application::ServerState Application::getState() const
{
	return mState;
}

void Application::serverLoop()
{
	std::cout << "Worker thread running." << std::endl;

	auto& watch = mEngine.get<FileWatcher>();
	auto& man = mEngine.get<ScriptManager>();
	std::string modified;

	const Timespan tickLength = std::chrono::milliseconds(1000 / mProperties["tickrate"].Int);
	Timespan tickTime(0);
	Timestamp now = Clock::now(), nextGC = now + std::chrono::seconds(2);
	auto oldframe = now;

	mState = State_Running;

	do
	{
		now = Clock::now();
		Timespan dt = now - oldframe;
		oldframe = now;

		tickTime += dt;

		auto selector = mConnections.getSelector();
		selector.add(mSocket);

		if (selector.wait(sf::microseconds(10)))
		{
			for (auto& client : mConnections.getClients(selector))
			{
				auto& sock = mConnections.getSocket(client);
				sf::Packet packet;
				sf::Socket::Status status = sf::Socket::Partial;

				while (status == sf::Socket::Partial)
					status = sock.receive(packet);

				if (status == sf::Socket::Disconnected)
				{
					std::cout << "Client " << client << " (" << sock.getRemoteAddress().toString() << ":" << sock.getRemotePort() << ") disconnected." << std::endl;

					mConnections.removeClient(client);

					if (mObjects.count(client) > 0)
					{
						sf::Packet packet;
						packet << uint16_t(PacketType_Destroy);
						packet << uint32_t(client);
						mConnections.sendPacketToAll(packet);

						mObjects[client].updateObject(nullptr);
						mObjects.erase(client);

						man.getEngine()->GarbageCollect(asGC_FULL_CYCLE, 5);
					}
				}
				else
				{
					std::cout << client << ": " << packet.getDataSize() << "B of data received." << std::endl;

					uint16_t packetType;
					packet >> packetType;

					switch (packetType)
					{
					case PacketType_Update:
						{
							do
							{
								int32_t id;
								packet >> id;

								if (mObjects.count(id) > 0)
									mObjects.at(id).injectPacket(packet);
								else
									break;
							} while (packet && !packet.endOfPacket());
						} break;
					}
				}
			}

			if (selector.isReady(mSocket))
			{
				std::unique_ptr<sf::TcpSocket> client(new sf::TcpSocket());
				auto ret = mSocket.accept(*client);

				if (ret == sf::Socket::Done)
				{
					std::cout << "Client " << client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " connected." << std::endl;

					auto cid = mConnections.addClient(std::move(client));

					sf::Packet packet;
					packet << uint16_t(PacketType_Player);
					packet << cid;
					mConnections.sendPacketTo(cid, packet);

					auto count = mClientSM.getEngine()->GetModuleCount();
					for (unsigned int i = 0; i < count; ++i)
					{
						auto* mod = mClientSM.getEngine()->GetModuleByIndex(i);

						ScriptManager::BytecodeStore store;
						mod->SaveByteCode(&store);

						sf::Packet packet;
						packet << (uint16_t)PacketType_Script;
						packet << mod->GetName();
						packet.append(store.getData(), store.getSize());

						mConnections.sendPacketTo(cid, packet);
					}

					auto* mod = man.getEngine()->GetModule("./scripts/testing.as");
					auto* obj = reinterpret_cast<asIScriptObject*>(man.getEngine()->CreateScriptObject(mod->GetObjectTypeByName("Player")));

					mObjects[cid] = NetworkedObject(cid, obj);
					mObjects[cid].setOwner(cid);

					packet = sf::Packet();
					packet << uint16_t(PacketType_Create);
					mObjects[cid].buildCreatePacket(packet);
					mConnections.sendPacketToAllBut(cid, packet);

					for (auto& obj : mObjects)
					{
						sf::Packet packet;
						packet << (uint16_t)PacketType_Create;
						obj.second.buildCreatePacket(packet);

						mConnections.sendPacketTo(cid, packet);
					}
				}
				else
				{
					std::cout << "Client " << client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " failed to connect! Error " << ret << std::endl;
				}
			}
		}

		for (auto& obj : mObjects)
		{
			obj.second.tick(dt);

			sf::Packet packet;
			packet << uint16_t(PacketType_Update);

			if (obj.second.buildPacket(packet))
			{
				mConnections.sendPacketToAll(packet);
			}
		}

		if (watch.pollChange(modified) && man.isLoaded(modified))
		{
			std::cout << "Reloading " << modified << "..." << std::endl;
			man.loadFromFile(modified);
		}

		while (tickTime >= tickLength)
		{
			// Run fixed updates
			man.runHook<const Timespan&>("Tick", tickLength);

			tickTime -= tickLength;
		}

		if (now > nextGC)
		{
			man.getEngine()->GarbageCollect(asGC_ONE_STEP);

			nextGC = now + std::chrono::seconds(1);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	} while (mRunning);

	mState = State_Stopped;
}
