#include "Application.hpp"

#include <Core/FileWatcher.hpp>
#include <Core/Time.hpp>
#include <Core/ScriptManager.hpp>

#include <Core/AS_Addons/scriptarray/scriptarray.h>
#include <Core/AS_Addons/scriptmath/scriptmath.h>
#include <Core/AS_Addons/scriptstdstring/scriptstdstring.h>
#include <Core/AS_SFML/AS_SFML.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <codecvt>
#include <iostream>
#include <sstream>
#include <string>

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
	mEngine.add<ScriptManager>();
	mEngine.add<FileWatcher>();
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
	as::SFML::registerTypes(man);

	man.init();

	man.addDefine("SERVER");
	man.registerHook("Tick", "void f(const Timespan&in)");

	man.addPostLoadCallback("OnLoad", [](asIScriptModule* mod) {
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
	mRunning = true;

	std::cout << "Spinning up worker thread..." << std::endl;
	mWorkThread = std::thread(&Application::serverLoop, this);

	std::cout << "Server online." << std::endl;

	do
	{
		std::cout << "> ";
		std::string line;

		std::getline(std::cin, line);
	} while (std::cin);

	mRunning = false;

	if (mWorkThread.joinable())
		mWorkThread.join();
}

void Application::serverLoop()
{
	std::cout << "Worker thread running." << std::endl;

	auto& watch = mEngine.get<FileWatcher>();
	auto& man = mEngine.get<ScriptManager>();
	std::string modified;

	const Timespan tickLength = std::chrono::milliseconds(15);
	Timespan tickTime(0);
	Timestamp now = Clock::now(), nextGC = now + std::chrono::seconds(2);
	auto oldframe = now;

	do
	{
		now = Clock::now();
		Timespan dt = now - oldframe;
		oldframe = now;

		tickTime += dt;

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
int ScriptManager::setCTXArg<const sf::Vector2f&>(asIScriptContext* ctx, uint32_t id, const sf::Vector2f& arg)
{
	return ctx->SetArgObject(id, const_cast<sf::Vector2f*>(&arg));
}
template<>
int ScriptManager::setCTXArg<const sf::Vector3f&>(asIScriptContext* ctx, uint32_t id, const sf::Vector3f& arg)
{
	return ctx->SetArgObject(id, const_cast<sf::Vector3f*>(&arg));
}