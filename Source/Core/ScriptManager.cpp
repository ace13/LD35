#include "ScriptManager.hpp"

#include <SFML/System/InputStream.hpp>

#include <angelscript.h>

#include <Core/AS_Addons/scriptbuilder/scriptbuilder.h>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>

namespace
{
	void error(ScriptManager& sman, const asSMessageInfo* msg)
	{
		std::cerr << "Angelscript ";
		switch (msg->type)
		{
		case asMSGTYPE_ERROR: std::cerr << "error"; break;
		case asMSGTYPE_WARNING: std::cerr << "warning"; break;
		case asMSGTYPE_INFORMATION: std::cerr << "info"; break;
		}

		if (msg->section && msg->section[0] != 0)
			std::cerr << " (" << msg->section << ":" << msg->row << ":" << msg->col << ")";
		std::cerr << ";" << std::endl
			<< ">  ";
#ifdef SFML_SYSTEM_LINUX
		std::cerr << "\x1b[31m";
#endif
		std::cerr << msg->message << std::endl;

#ifdef SFML_SYSTEM_LINUX
		std::cerr << "\x1b[0m";
#endif
	}

	int loadInclude(const char *include, const char *from, CScriptBuilder *builder, void *userParam)
	{
		std::string file = from;
		auto last = file.find_last_of('/');
		if (last != std::string::npos)
			file.erase(last + 1);
		if (file[0] == '.')
			file.erase(0, 2);

		return builder->AddSectionFromFile((file + include).c_str());
	}
}


ScriptManager::BytecodeStore::BytecodeStore() : mTellg(0) { }
ScriptManager::BytecodeStore::BytecodeStore(const char* data, size_t len)
	: mTellg(0)
{
	mStore.assign(data, data + len);
}

void ScriptManager::BytecodeStore::Read(void *ptr, asUINT size)
{
	char* data = (char*)ptr;

	for (uint32_t i = 0; i < size; ++i)
	{
		data[i] = mStore[mTellg + i];
	}

	mTellg += size;
}
void ScriptManager::BytecodeStore::Write(const void *ptr, asUINT size)
{
	const char* data = (const char*)ptr;

	for (uint32_t i = 0; i < size; ++i)
		mStore.push_back(data[i]);
}
const char* ScriptManager::BytecodeStore::getData() const
{
	return mStore.data();
}
size_t ScriptManager::BytecodeStore::getSize() const
{
	return mStore.size();
}


ScriptManager::ScriptManager()
	: mEngine(nullptr)
	, mReloading(false)
	, mSerialize(true)
{

}

ScriptManager::~ScriptManager()
{
	unloadAll();

	mEngine->Release();
	mEngine = nullptr;
}

void ScriptManager::addExtension(const std::string& name, const ScriptExtensionFun& function)
{
	mExtensions.emplace_back(name, function);
}
void ScriptManager::registerSerializedType(const std::string& name, const std::function<CUserType*()>& ser)
{
	mSerializers[name] = ser;
}

void ScriptManager::init()
{
	asIScriptEngine* eng = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	eng->SetUserData(this, 0x4547);
	eng->SetMessageCallback(asFUNCTION(error), this, asCALL_CDECL_OBJFIRST);

	eng->RegisterGlobalProperty("const bool RELOADING", &mReloading);

#ifndef NDEBUG
	std::unordered_map<std::string, uint32_t> counts;
#define CHECK_COUNT(Name, Check) do { \
	auto cnt = eng->Get ## Check ## Count(); \
	if (counts.count(Name) > 0 && cnt != counts.at(Name)) \
	{ \
		std::cout << ">  " << (cnt - counts.at(Name)) << " new " Name "(s)" << std::endl; \
	} \
	counts[Name] = cnt; \
} while (false);
#else
#define CHECK_COUNT(A, B)
#endif
	for (auto& ext : mExtensions)
	{
		std::cout << "Registering " << std::get<0>(ext) << "..." << std::endl;
		try
		{
			std::get<1>(ext)(eng);

			CHECK_COUNT("enum", Enum);
			CHECK_COUNT("funcdef", Funcdef);
			CHECK_COUNT("global function", GlobalFunction);
			CHECK_COUNT("global property", GlobalProperty);
			CHECK_COUNT("object type", ObjectType);
			CHECK_COUNT("typedef", Typedef);
		}
		catch (const std::runtime_error& ex)
		{
			std::cerr << std::string(ex.what()) << std::endl;
			std::cout << "Failed to register " << std::get<0>(ext) << std::endl;
		}
	}
#undef CHECK_COUNT

	//eng->ClearMessageCallback();
	mEngine = eng;
}

void ScriptManager::initBare()
{
	asIScriptEngine* eng = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	eng->SetUserData(this, 0x4547);
	eng->SetMessageCallback(asFUNCTION(error), this, asCALL_CDECL_OBJFIRST);

	mEngine = eng;
}

bool ScriptManager::isLoaded(const std::string& name) const
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	return mScripts.count(lower) > 0;
}

bool ScriptManager::loadFromFile(const std::string& file, ScriptType type)
{
	std::ifstream ifs(file.c_str());
	if (!ifs)
		return false;

	ifs.seekg(0, std::ios::end);
	size_t len = size_t(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	if (len == 0)
		return false;

	std::vector<char> data(len);
	ifs.read(&data[0], len);

	// Remove extraneous nullbytes
	auto it = std::remove(data.begin(), data.end(), '\0');
	if (it != data.end())
		data.erase(it, data.end());

	return loadFromMemory(file, &data[0], data.size(), type);
}
bool ScriptManager::loadFromMemory(const std::string& name, const void* data, size_t len, ScriptType type)
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	if (type == Type_Autodetect)
	{
		if (lower.substr(lower.size() - 4) == ".asb")
			type = Type_Bytecode;
		else if (lower.substr(lower.size() - 3) == ".as")
			type = Type_Text;
		else
			return false;
	}

	BytecodeStore bcode;
	if (type == Type_Text)
	{
		static const char* scratchName = "!!ScratchSpace!!";
		CScriptBuilder builder;
		builder.SetIncludeCallback(loadInclude, this);

		for (auto& def : mDefines)
			builder.DefineWord(def.c_str());

		builder.StartNewModule(mEngine, scratchName);

		for (auto& callback : mPreLoadCallbacks)
			if (!callback.second(builder.GetModule()))
			{
				mEngine->DiscardModule(scratchName);
				return false;
			}

		builder.AddSectionFromMemory(lower.c_str(), (const char*)data, len);

		int r = builder.BuildModule();
		if (r < 0)
		{
#ifndef NDEBUG
			puts(ASException::GetMessage(r));
#endif

			mEngine->DiscardModule(scratchName);
			return false;
		}

#ifdef NDEBUG
		builder.GetModule()->SaveByteCode(&bcode, true);
#else
		builder.GetModule()->SaveByteCode(&bcode, false);
#endif

		mEngine->DiscardModule(scratchName);
	}
	else
	{
		bcode = BytecodeStore((const char*)data, len);
	}

	return loadFromStream(name, bcode);
}
bool ScriptManager::loadFromStream(const std::string& name, sf::InputStream& stream, ScriptType type)
{
	auto len = size_t(stream.getSize());
	std::vector<char> data(len);
	stream.read(&data[0], len);

	return loadFromMemory(name, &data[0], len, type);
}
bool ScriptManager::loadFromStream(const std::string& name, asIBinaryStream& stream, ScriptType type)
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	if (type == Type_Autodetect)
	{
		if (lower.substr(lower.size() - 4) == ".asb")
			type = Type_Bytecode;
		else if (lower.substr(lower.size() - 3) == ".as")
			type = Type_Text;
		else
			return false;
	}

	if (type != Type_Bytecode)
		return false;

	bool reload = mScripts.count(lower) > 0;

	std::list<asIScriptObject*> changes;

	asIScriptModule* module = mEngine->GetModule(lower.c_str(), asGM_ONLY_IF_EXISTS);
	CSerializer serial;

	if (reload && module && mSerialize)
	{
		mReloading = true;

		for (auto& reg : mSerializers)
			serial.AddUserType(reg.second(), reg.first);

		for (auto it = mChangeNotice.begin(); it != mChangeNotice.end();)
		{
			if (it->second.WeakRef->Get())
			{
				it->second.WeakRef->Release();
				it = mChangeNotice.erase(it);
				continue;
			}

			auto* obj = it->first;

			if (obj->GetObjectType()->GetModule() == module)
			{
				serial.AddExtraObjectToStore(obj);

				changes.push_back(it->first);
			}

			++it;
		}

		serial.Store(module);
	}

	if (module)
		module->Discard();

	module = mEngine->GetModule(lower.c_str(), asGM_ALWAYS_CREATE);

	int r = module->LoadByteCode(&stream);
	if (r < 0)
	{
		module->Discard();
		mReloading = false;
		return false;
	}

	module->BindAllImportedFunctions();

	if (mScripts.count(lower) == 0)
	{
		mScripts[lower].Name = name;
		mScripts[lower].DirectLoad = true;
	}

	if (reload && mSerialize)
	{
		serial.Restore(module);

		for (auto& it : changes)
		{
			auto* newObj = (asIScriptObject*)serial.GetPointerToRestoredObject(it);

			auto notice = mChangeNotice[it];
			mChangeNotice.erase(it);

			notice.WeakRef->Release();

			for (auto& callback : notice.Callbacks)
				callback.Callback(newObj);
		}

		module->SetUserData((void*)1, Data_Reloaded);
		mEngine->GarbageCollect(asGC_FULL_CYCLE);

		mReloading = false;
	}
	else
		module->SetUserData((void*)0, Data_Reloaded);

	for (auto& callback : mPostLoadCallbacks)
		callback.second(module);

	return true;
}

void ScriptManager::unload(const std::string& name)
{
	asIScriptModule* module = mEngine->GetModule(name.c_str(), asGM_ONLY_IF_EXISTS);
	if (!module)
		return;

	std::list<asIScriptObject*> toRemove;
	auto copy = mChangeNotice;
	for (auto& it : copy)
	{
		auto* obj = it.first;

		if (it.second.WeakRef->Get() || obj->GetObjectType()->GetModule() == module)
		{
			for (auto& callback : it.second.Callbacks)
				callback.Callback(nullptr);

			toRemove.push_back(it.first);
		}
	}

	for (auto& obj : toRemove)
		removeChangeNotice(obj);

	for (auto& hooks : mScriptHooks)
	{
		for (auto it = hooks.second.begin(); it != hooks.second.end();)
		{
			auto* obj = it->Object;

			if (obj->GetObjectType()->GetModule() == module)
			{
				it->WeakRef->Release();
				it = hooks.second.erase(it);
			}
			else
				++it;
		}
	}

	module->Discard();

	mScripts.erase(name);

	mEngine->GarbageCollect(asGC_FULL_CYCLE, 2);
}

void ScriptManager::unloadAll()
{
	auto toUnload = std::move(mScripts);
	for (auto script : toUnload)
	{
		unload(script.first);
	}

	mEngine->GarbageCollect(asGC_FULL_CYCLE, 5);
}

bool ScriptManager::isDefined(const std::string& define) const
{
	return std::find(mDefines.begin(), mDefines.end(), define) != mDefines.end();
}
void ScriptManager::addDefine(const std::string& define)
{
	if (std::find(mDefines.begin(), mDefines.end(), define) != mDefines.end())
		return;

	mDefines.push_back(define);
}
void ScriptManager::removeDefine(const std::string& define)
{
	auto it = std::find(mDefines.begin(), mDefines.end(), define);
	if (it != mDefines.end())
		mDefines.erase(it);
}
void ScriptManager::clearDefines()
{
	mDefines.clear();
}


void ScriptManager::setSerialization(bool serialize)
{
	mSerialize = serialize;
}
bool ScriptManager::getSerialization() const
{
	return mSerialize;
}


void ScriptManager::clearPreLoadCallbacks()
{
	mPreLoadCallbacks.clear();
}
void ScriptManager::removePreLoadCallback(const std::string& id)
{
	mPreLoadCallbacks.erase(id);
}
void ScriptManager::addPreLoadCallback(const std::string& id, const ScriptPreLoadCallbackFun& func)
{
	mPreLoadCallbacks[id] = func;
}
bool ScriptManager::hasPreLoadCallback(const std::string& id) const
{
	return mPreLoadCallbacks.count(id) > 0;
}


void ScriptManager::clearPostLoadCallbacks()
{
	mPostLoadCallbacks.clear();
}
void ScriptManager::removePostLoadCallback(const std::string& id)
{
	mPostLoadCallbacks.erase(id);
}
void ScriptManager::addPostLoadCallback(const std::string& id, const ScriptPostLoadCallbackFun& func)
{
	mPostLoadCallbacks[id] = func;
}
bool ScriptManager::hasPostLoadCallback(const std::string& id) const
{
	return mPostLoadCallbacks.count(id) > 0;
}


void ScriptManager::addChangeNotice(asIScriptObject* obj, const std::string& name, const ScriptObjectCallbackFun& callback)
{
	if (mChangeNotice.count(obj) == 0)
	{
		mChangeNotice[obj] = { obj->GetWeakRefFlag(), { { name, callback } } };
		obj->GetWeakRefFlag()->AddRef();
	}
	else
	{
		auto& callbacks = mChangeNotice[obj].Callbacks;

		auto it = std::find_if(callbacks.begin(), callbacks.end(), [name](ChangeNotice::NoticeCallback& it) { return it.Name == name; });
		if (it != callbacks.end())
			callbacks.erase(it);

		callbacks.push_back({ name, callback });
	}
}
void ScriptManager::removeChangeNotice(asIScriptObject* obj, const std::string& name)
{
	if (mChangeNotice.count(obj) > 0)
	{
		auto& callbacks = mChangeNotice[obj].Callbacks;

		auto it = std::find_if(callbacks.begin(), callbacks.end(), [name](ChangeNotice::NoticeCallback& it) { return it.Name == name; });
		if (it != callbacks.end())
			callbacks.erase(it);

		if (callbacks.empty())
		{
			mChangeNotice[obj].WeakRef->Release();
			mChangeNotice.erase(obj);
		}
	}
}
void ScriptManager::removeChangeNotice(asIScriptObject* obj)
{
	if (mChangeNotice.count(obj) > 0)
	{
		auto& notice = mChangeNotice.at(obj);
		notice.WeakRef->Release();

		mChangeNotice.erase(obj);
	}
}


void ScriptManager::registerHook(const std::string& name, const std::string& decl)
{
	mRegisteredHooks.emplace(name, decl);
}

bool ScriptManager::addHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj)
{
	if (mRegisteredHooks.count(hook) == 0)
		return false;

	auto& data = mRegisteredHooks.at(hook);
	std::string decl = func->GetDeclaration(false);
	std::string name = func->GetName();
	auto off = decl.find(name);
	decl.replace(off, name.length(), "f");

	if (decl != data)
		return false;

	auto& hooks = mScriptHooks[hook];
	auto it = std::find_if(hooks.begin(), hooks.end(), [obj, func](ScriptHook& hook) { return hook.Function == func && hook.Object == obj; });
	if (it == hooks.end())
	{
		hooks.push_back({ func->GetDeclaration(false), func, obj, obj->GetWeakRefFlag() });
		obj->GetWeakRefFlag()->AddRef();
	}

	return true;
}
bool ScriptManager::removeHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj)
{
	if (mRegisteredHooks.count(hook) == 0 || mScriptHooks.count(hook) == 0)
		return false;

	auto& hooks = mScriptHooks.at(hook);
	auto it = std::find_if(hooks.begin(), hooks.end(), [obj, func](ScriptHook& hook) { return hook.Function == func && hook.Object == obj; });

	if (it != hooks.end())
	{
		it->WeakRef->Release();
		hooks.erase(it, hooks.end());

		return true;
	}

	return false;
}

void ScriptManager::addHookFromScript(const std::string& hook, const std::string& func)
{
	auto* ctx = asGetActiveContext();
	if (mRegisteredHooks.count(hook) == 0)
	{
		ctx->SetException(("No hook named '" + hook + "'").c_str());
		return;
	}

	asIScriptObject* obj = (asIScriptObject*)ctx->GetThisPointer();
	auto* funcptr = obj->GetObjectType()->GetMethodByName(func.c_str());

	if (!addHook(hook, funcptr, obj))
	{
		std::string decl = funcptr->GetDeclaration(false);
		std::string name = funcptr->GetName();
		auto off = decl.find(name);
		decl.replace(off, name.length(), "f");

		ctx->SetException(("Invalid declaration for hook '" + hook + "'\n" +
			"Got " + decl + " but expects " + mRegisteredHooks.at(hook)).c_str());

		return;
	}

	static std::function<void(asIScriptObject* oldObj, asIScriptFunction* func, asIScriptObject* newObj)> updateChangeNotice;
	if (!updateChangeNotice)
		updateChangeNotice = [this, hook](asIScriptObject* oldObj, asIScriptFunction* func, asIScriptObject* newObj)
		{
			for (auto& hooks : mScriptHooks)
			{
				auto it = std::find_if(hooks.second.begin(), hooks.second.end(), [oldObj, func](const ScriptHook& h) { return h.Function == func && h.Object == oldObj; });

				if (it != hooks.second.end())
				{
					if (newObj)
						newObj->GetWeakRefFlag()->AddRef();

					it->WeakRef->Release();
					removeChangeNotice(it->Object, hook);

					if (newObj)
					{
						it->Object = newObj;
						it->Function = newObj->GetObjectType()->GetMethodByDecl(it->FuncDecl.c_str());

						it->WeakRef = it->Object->GetWeakRefFlag();

						ScriptManager::ScriptHook hookObj = *it;
						addChangeNotice(newObj, hook, [hookObj](asIScriptObject* evenNewerObj) {
							updateChangeNotice(hookObj.Object, hookObj.Function, evenNewerObj);
						});
					}
					else
						hooks.second.erase(it);
				}
			}
		};

	addChangeNotice(obj, hook, [=](asIScriptObject* newObj) {
		updateChangeNotice(obj, funcptr, newObj);
	});
}
void ScriptManager::removeHookFromScript(const std::string& hook, const std::string& func)
{
	auto* ctx = asGetActiveContext();
	if (mRegisteredHooks.count(hook) == 0)
	{
		ctx->SetException("No such hook");
		return;
	}

	asIScriptObject* obj = (asIScriptObject*)ctx->GetThisPointer();
	asIScriptFunction* funcptr = nullptr;
	if (func.empty())
	{
		if (mScriptHooks.count(hook) > 0)
		{
			auto& hooks = mScriptHooks.at(hook);
			std::find_if(hooks.begin(), hooks.end(), [&funcptr, obj](const ScriptHook& sh) {
				if (sh.Object == obj)
				{
					funcptr = sh.Function;
					return true;
				}
				return false;
			});
		}

		if (!funcptr)
		{
			ctx->SetException("No such hook registered");
			return;
		}
	}
	else
		funcptr = obj->GetObjectType()->GetMethodByName(func.c_str());

	removeChangeNotice(obj, hook);
	removeHook(hook, funcptr, obj);
}


asIScriptEngine* ScriptManager::getEngine()
{
	return mEngine;
}

#define PRIMITIVE_ARG(Type, SetType) template<> \
int ScriptManager::setCTXArg<Type>(asIScriptContext* ctx, uint32_t id, Type arg) \
{ \
	return ctx->SetArg ## SetType (id, arg); \
}//

PRIMITIVE_ARG(uint8_t, Byte)
PRIMITIVE_ARG(uint16_t, Word)
PRIMITIVE_ARG(uint32_t, DWord)
PRIMITIVE_ARG(uint64_t, QWord)
PRIMITIVE_ARG(bool, Byte)
PRIMITIVE_ARG(int8_t, Byte)
PRIMITIVE_ARG(int16_t, Word)
PRIMITIVE_ARG(int32_t, DWord)
PRIMITIVE_ARG(int64_t, QWord)
PRIMITIVE_ARG(float, Float)
PRIMITIVE_ARG(double, Double)
#undef PRIMITIVE_ARG
