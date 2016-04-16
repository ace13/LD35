#include "Application.hpp"
#include "ConnectionManager.hpp"
#include "ResourceManager.hpp"
#include "ServerContainer.hpp"
#include "StateManager.hpp"

#include "States/IntroState.hpp"

#include <Core/Math.hpp>
#include <Core/Time.hpp>
//#include <Core/FileWatcher.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Sleep.hpp>

#include <Core/AS_Addons/scriptarray/scriptarray.h>
#include <Core/AS_Addons/scripthelper/scripthelper.h>
#include <Core/AS_Addons/scriptmath/scriptmath.h>
#include <Core/AS_Addons/scriptstdstring/scriptstdstring.h>
#include <Core/AS_SFML/AS_SFML.hpp>

#include <codecvt>
#include <iostream>
#include <sstream>

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
}

Application::Application()
{
	auto dur = Time::getClockPrecision();

	std::cout << Clock::now() << std::endl;
	std::cout << "Clock precision seems to be ~" << dur << std::endl;

	if (dur > std::chrono::milliseconds(2))
		std::cout << "This might lead to problems running the application on your platform..." << std::endl;

	mEngine.add<ScriptManager>();
	mEngine.add<sf::RenderWindow>();
	mEngine.add<ConnectionManager>();
	mEngine.add<ResourceManager>();
	mEngine.add<ServerContainer>();
	mEngine.add<StateManager>();
}

Application::~Application()
{
}

void Application::init()
{
	auto beg = Clock::now();
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
	Math::registerScriptData(man);
	Time::registerTimeTypes(man);
	as::SFML::registerTypes(man);
	mEngine.get<ResourceManager>().registerScript(man);
	man.addExtension("ScriptHooks", [&](asIScriptEngine* eng) {
		eng->SetDefaultNamespace("Hooks");

		eng->RegisterGlobalFunction("void Add(const string&in, const string&in)", asMETHOD(ScriptManager, addHookFromScript), asCALL_THISCALL_ASGLOBAL, &man);
		eng->RegisterGlobalFunction("void Remove(const string&in, const string&in = \"\")", asMETHOD(ScriptManager, removeHookFromScript), asCALL_THISCALL_ASGLOBAL, &man);

		eng->SetDefaultNamespace("");
	});

	man.init();

	man.getEngine()->SetUserData(&mEngine.get<sf::RenderWindow>(), 0x6AE1);

	man.addDefine("CLIENT");
	man.registerHook("Tick",   "void f(const Timespan&in)");
	man.registerHook("Update", "void f(const Timespan&in)");
	man.registerHook("Draw",   "void f(sf::Renderer@)");
	man.registerHook("DrawUI", "void f(sf::Renderer@)");

	man.registerHook("Keyboard.Key", "void f(sf::Keyboard::Key,bool)");
	man.registerHook("Joystick.Button", "void f(uint,uint,bool)");
	man.registerHook("Joystick.Moved", "void f(uint,sf::Joystick::Axis,float)");
	man.registerHook("Mouse.Button", "void f(Vec2&in,sf::Mouse::Button,bool)");
	man.registerHook("Mouse.Moved", "void f(Vec2&in)");
	man.registerHook("Text.Entered", "void f(uint)");

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

	WriteConfigToFile(man.getEngine(), "ClientEngineConfig.txt");

	/*
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
	*/

	auto end = Clock::now();
	std::cout << "Init took " << (end - beg) << std::endl;
}

void Application::run()
{
	std::cout << "Application started in " << Time::getRunTime() << std::endl;

	sf::Event ev;
	std::string modified;
	ConnectionManager::Event netEv;

	auto& window = mEngine.get<sf::RenderWindow>();
	auto& server = mEngine.get<ServerContainer>();
	auto& man = mEngine.get<ScriptManager>();
	auto& connection = mEngine.get<ConnectionManager>();
	auto& state = mEngine.get<StateManager>();
	state.pushState(new States::IntroState());

	window.create({ 800, 600 }, "LD35 Preparational Client");
	sf::View uiView = window.getDefaultView(), gameView({}, { 0, 2500 });
	{
		sf::Vector2f size = (sf::Vector2f)window.getSize();
		uiView.setSize(size);
		uiView.setCenter(size / 2.f);

		gameView.setSize(gameView.getSize().y * (size.x / size.y), gameView.getSize().y);
	}

	const Timespan tickLength = std::chrono::milliseconds(15);
	Timespan tickTime(0);
	Timestamp now = Clock::now(), nextGC = now + std::chrono::seconds(2);

	auto oldframe = now;

	server.init();
	server.launch();
	connection.connect(server.getPort(), sf::IpAddress::LocalHost);

	while (window.isOpen())
	{
		now = Clock::now();
		Timespan dt = now - oldframe;
		oldframe = now;

		tickTime += dt;
/*
		if (watch.pollChange(modified) && man.isLoaded(modified))
		{
			std::cout << "Reloading " << modified << "..." << std::endl;
			man.loadFromFile(modified);
		}
*/

		// -------------
		// Handle Events

		if (connection.pollEvent(netEv))
		{
			switch (netEv.Type)
			{
			case ConnectionManager::Event::Type_Script:
				{
					std::string name;
					netEv.Data >> name;

					ScriptManager::BytecodeStore store;
					do
					{
						uint8_t c;
						netEv.Data >> c;
						store.Write(&c, 1);
					} while (netEv.Data);

					std::cout << "Received script state for " << name << " from the server, integrating..." << std::endl;
					if (!man.loadFromStream(name, store))
						std::cout << "Integration of new script code failed." << std::endl;
				}
				break;
			}
		}

		if (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				window.close();
			else if (ev.type == sf::Event::Resized)
			{
				sf::Vector2f size = (sf::Vector2f)window.getSize();
				uiView.setSize(size);
				uiView.setCenter(size / 2.f);

				gameView.setSize(gameView.getSize().y * (size.x / size.y), gameView.getSize().y);
			}
			else
			{
				if (ev.type == sf::Event::KeyPressed ||
					ev.type == sf::Event::KeyReleased)
				{
					bool pressed = ev.type == sf::Event::KeyPressed;
					
					man.runHook<sf::Keyboard::Key, bool>("Keyboard.Key", ev.key.code, pressed);
				}
				else if(ev.type == sf::Event::JoystickButtonPressed ||
					ev.type == sf::Event::JoystickButtonReleased)
				{
					bool pressed = ev.type == sf::Event::JoystickButtonPressed;

					man.runHook<uint32_t, uint32_t, bool>("Joystick.Button", ev.joystickButton.joystickId, ev.joystickButton.button, pressed);
				}
				else if (ev.type == sf::Event::JoystickMoved)
				{
					man.runHook<uint32_t, sf::Joystick::Axis, float>("Joystick.Moved", ev.joystickMove.joystickId, ev.joystickMove.axis, ev.joystickMove.position);
				}
				else if (ev.type == sf::Event::MouseButtonPressed ||
					ev.type == sf::Event::MouseButtonReleased)
				{
					bool pressed = ev.type == sf::Event::MouseButtonPressed;

					sf::Vector2f pos(sf::Mouse::getPosition(window));
					man.runHook<const sf::Vector2f&, sf::Mouse::Button, bool>("Mouse.Button", pos, ev.mouseButton.button, pressed);
				}
				else if (ev.type == sf::Event::MouseMoved)
				{
					sf::Vector2f pos(sf::Mouse::getPosition(window));
					man.runHook<const sf::Vector2f&>("Mouse.Moved", pos);
				}
				else if (ev.type == sf::Event::TextEntered)
				{
					man.runHook<uint32_t>("Text.Entered", ev.text.unicode);
				}
			}
		}


		// -----------
		// Run updates

		while (tickTime >= tickLength)
		{
			// Run fixed updates
			server.tick();
			connection.tick();
			state.tick(tickLength);
			man.runHook<const Timespan&>("Tick", tickLength);

			tickTime -= tickLength;
		}

		// Run per-frame updates
		state.update(dt);
		man.runHook<const Timespan&>("Update", dt);


		// ----------------
		// Draw the results

		window.clear();

		// Draw things
		window.setView(gameView);
		state.draw(window);
		man.runHook<sf::RenderTarget*>("Draw", &window);
		gameView = window.getView();

		// Draw UI things
		window.setView(uiView);
		state.drawUI(window);
		man.runHook<sf::RenderTarget*>("DrawUI", &window);

		window.display();

		if (now > nextGC)
		{
			man.getEngine()->GarbageCollect(asGC_ONE_STEP);

			nextGC = now + std::chrono::seconds(1);
		}
	}
}

sf::RenderTarget& Application::getRT()
{
	return mEngine.get<sf::RenderWindow>();
}

template<>
int ScriptManager::setCTXArg<sf::Mouse::Button>(asIScriptContext* ctx, uint32_t id, sf::Mouse::Button arg)
{
	return ctx->SetArgDWord(id, arg);
}
template<>
int ScriptManager::setCTXArg<sf::Joystick::Axis>(asIScriptContext* ctx, uint32_t id, sf::Joystick::Axis arg)
{
	return ctx->SetArgDWord(id, arg);
}
template<>
int ScriptManager::setCTXArg<sf::Keyboard::Key>(asIScriptContext* ctx, uint32_t id, sf::Keyboard::Key arg)
{
	return ctx->SetArgDWord(id, arg);
}
template<>
int ScriptManager::setCTXArg<const std::string*>(asIScriptContext* ctx, uint32_t id, const std::string* arg)
{
	return ctx->SetArgObject(id, (std::string*)arg);
}
template<>
int ScriptManager::setCTXArg<const Timespan*>(asIScriptContext* ctx, uint32_t id, const Timespan* arg)
{
	return ctx->SetArgObject(id, (Timespan*)arg);
}
template<>
int ScriptManager::setCTXArg<const Timespan&>(asIScriptContext* ctx, uint32_t id, const Timespan& arg)
{
	return ctx->SetArgObject(id, const_cast<Timespan*>(&arg));
}
template<>
int ScriptManager::setCTXArg<sf::RenderTarget*>(asIScriptContext* ctx, uint32_t id, sf::RenderTarget* arg)
{
	return ctx->SetArgObject(id, arg);
}
template<>
int ScriptManager::setCTXArg<const sf::Vector2f&>(asIScriptContext* ctx, uint32_t id, const sf::Vector2f& arg)
{
	return ctx->SetArgObject(id, const_cast<sf::Vector2f*>(&arg));
}
template<>
int ScriptManager::setCTXArg<const sf::Vector3f&>(asIScriptContext* ctx, uint32_t id, const sf::Vector3f& arg)
{
	return ctx->SetArgObject(id, const_cast<sf::Vector3f*>(&arg));
}