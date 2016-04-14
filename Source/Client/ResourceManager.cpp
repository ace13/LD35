#include "ResourceManager.hpp"
#include <Core/ScriptManager.hpp>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>

namespace
{
	template<typename T>
	void createRes(void* mem)
	{
		new (mem) ResourceManager::res_ptr<T>();
	}
	template<typename T>
	void destroyRes(ResourceManager::res_ptr<T>* mem)
	{
		mem->~res_ptr();
	}

}

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::addref(void* data)
{
	for (auto& it : mResources)
		if (it.second.Memory == data)
		{
			++it.second.Counter;
			break;
		}
}

void ResourceManager::release(void* data)
{
	for (auto it = mResources.begin(); it != mResources.end(); ++it)
		if (it->second.Memory == data)
		{
			if (--it->second.Counter == 0)
			{
				it->second.Destructor(it->second.Memory);
				mResources.erase(it);
			}

			break;
		}
}

void ResourceManager::registerScript(ScriptManager& man)
{
	man.addExtension("Resources", [this](asIScriptEngine* eng) {
		AS_ASSERT(eng->SetDefaultNamespace("Resources"));

		AS_ASSERT(eng->RegisterObjectType("Texture", sizeof(Texture), asOBJ_VALUE | asGetTypeTraits<Texture>()));
		AS_ASSERT(eng->RegisterObjectBehaviour("Texture", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createRes<sf::Texture>), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Texture", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyRes<sf::Texture>), asCALL_CDECL_OBJFIRST));
		
		AS_ASSERT(eng->RegisterObjectMethod("Texture", "Texture& opAssign(const Texture&in)", asMETHOD(res_ptr<sf::Texture>, operator=), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("Texture", "sf::Texture@ get_Texture()", asMETHODPR(res_ptr<sf::Texture>, operator*, (), sf::Texture&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Texture", "const sf::Texture@ get_Texture() const", asMETHODPR(res_ptr<sf::Texture>, operator*, () const, const sf::Texture&), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectType("SoundBuffer", sizeof(Sound), asOBJ_VALUE | asGetTypeTraits<Sound>()));
		AS_ASSERT(eng->RegisterObjectBehaviour("SoundBuffer", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createRes<sf::SoundBuffer>), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("SoundBuffer", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyRes<sf::SoundBuffer>), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("SoundBuffer", "SoundBuffer& opAssign(const SoundBuffer&in)", asMETHOD(res_ptr<sf::SoundBuffer>, operator=), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("SoundBuffer", "sf::SoundBuffer@ get_SoundBuffer()", asMETHODPR(res_ptr<sf::SoundBuffer>, operator*, (), sf::SoundBuffer&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("SoundBuffer", "const sf::SoundBuffer@ get_SoundBuffer() const", asMETHODPR(res_ptr<sf::SoundBuffer>, operator*, () const, const sf::SoundBuffer&), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectType("Music", sizeof(Music), asOBJ_VALUE | asGetTypeTraits<Music>()));
		AS_ASSERT(eng->RegisterObjectBehaviour("Music", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createRes<sf::Music>), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Music", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyRes<sf::Music>), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Music", "Music& opAssign(const Music&in)", asMETHOD(res_ptr<sf::Music>, operator=), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("Music", "sf::Music@ get_Music()", asMETHODPR(res_ptr<sf::Music>, operator*, (), sf::Music&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Music", "const sf::Music@ get_Music() const", asMETHODPR(res_ptr<sf::Music>, operator*, () const, const sf::Music&), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectType("Font", sizeof(Font), asOBJ_VALUE | asGetTypeTraits<Font>()));
		AS_ASSERT(eng->RegisterObjectBehaviour("Font", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createRes<sf::Font>), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Font", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyRes<sf::Font>), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Font", "Font& opAssign(const Font&in)", asMETHOD(res_ptr<sf::Font>, operator=), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("Font", "sf::Font@ get_Font()", asMETHODPR(res_ptr<sf::Font>, operator*, (), sf::Font&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Font", "const sf::Font@ get_Font() const", asMETHODPR(res_ptr<sf::Font>, operator*, () const, const sf::Font&), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectType("Shader", sizeof(Shader), asOBJ_VALUE | asGetTypeTraits<Shader>()));
		AS_ASSERT(eng->RegisterObjectBehaviour("Shader", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createRes<sf::Shader>), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Shader", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyRes<sf::Shader>), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Shader", "Shader& opAssign(const Shader&in)", asMETHOD(res_ptr<sf::Shader>, operator=), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("Shader", "sf::Shader@ get_Shader()", asMETHODPR(res_ptr<sf::Shader>, operator*, (), sf::Shader&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Shader", "const sf::Shader@ get_Shader() const", asMETHODPR(res_ptr<sf::Shader>, operator*, () const, const sf::Shader&), asCALL_THISCALL));


		AS_ASSERT(eng->RegisterGlobalFunction("Texture GetTexture(const string&in)", asMETHODPR(ResourceManager, get<sf::Texture>, (const std::string&), ResourceManager::res_ptr<sf::Texture>), asCALL_THISCALL_ASGLOBAL, this));
		AS_ASSERT(eng->RegisterGlobalFunction("SoundBuffer GetSoundBuffer(const string&in)", asMETHODPR(ResourceManager, get<sf::SoundBuffer>, (const std::string&), ResourceManager::res_ptr<sf::SoundBuffer>), asCALL_THISCALL_ASGLOBAL, this));
		AS_ASSERT(eng->RegisterGlobalFunction("Music GetMusic(const string&in)", asMETHODPR(ResourceManager, get<sf::Music>, (const std::string&), ResourceManager::res_ptr<sf::Music>), asCALL_THISCALL_ASGLOBAL, this));
		AS_ASSERT(eng->RegisterGlobalFunction("Font GetFont(const string&in)", asMETHODPR(ResourceManager, get<sf::Font>, (const std::string&), ResourceManager::res_ptr<sf::Font>), asCALL_THISCALL_ASGLOBAL, this));
		AS_ASSERT(eng->RegisterGlobalFunction("Shader GetShader(const string&in)", asMETHODPR(ResourceManager, get<sf::Shader>, (const std::string&), ResourceManager::res_ptr<sf::Shader>), asCALL_THISCALL_ASGLOBAL, this));

		AS_ASSERT(eng->SetDefaultNamespace(""));
	});

	man.registerSerializedType<res_ptr<sf::Font>>("Font");
	man.registerSerializedType<res_ptr<sf::Texture>>("Texture");
	man.registerSerializedType<res_ptr<sf::SoundBuffer>>("SoundBuffer");
	man.registerSerializedType<res_ptr<sf::Music>>("Music");
	man.registerSerializedType<res_ptr<sf::Shader>>("Shader");
}

template<>
bool ResourceManager::loadResource<sf::Font>(sf::Font* res, const std::string& file)
{
#ifdef SFML_SYSTEM_WINDOWS
	std::string fontPath = "C:\\Windows\\Fonts\\" + file;
	struct stat asdf;
	if (stat(fontPath.c_str(), &asdf) >= 0)
	{
		return res->loadFromFile(fontPath);
	}
#endif
	return res->loadFromFile(file);
}
template<>
bool ResourceManager::loadResource<sf::Texture>(sf::Texture* res, const std::string& file)
{
	return res->loadFromFile(file);
}
template<>
bool ResourceManager::loadResource<sf::SoundBuffer>(sf::SoundBuffer* res, const std::string& file)
{
	return res->loadFromFile(file);
}
template<>
bool ResourceManager::loadResource<sf::Music>(sf::Music* res, const std::string& file)
{
	return res->openFromFile(file);
}
template<>
bool ResourceManager::loadResource<sf::Shader>(sf::Shader* res, const std::string& file)
{
	if (file.find_last_of(".glsl") != std::string::npos)
		return res->loadFromFile(file + ".vert", file + ".frag");
	else if (file.find_last_of(".vert") != std::string::npos)
		return res->loadFromFile(file, sf::Shader::Vertex);
#if SFML_VERSION_MAJOR > 2 || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR > 3) || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR == 3 && SFML_VERSION_PATCH > 2) || defined SFML_DEPRECATED
	else if (file.find_last_of(".geom") != std::string::npos)
		return res->loadFromFile(file, sf::Shader::Geometry);
#endif
	else if (file.find_last_of(".frag") != std::string::npos)
		return res->loadFromFile(file, sf::Shader::Fragment);
	else
#if SFML_VERSION_MAJOR > 2 || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR > 3) || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR == 3 && SFML_VERSION_PATCH > 2) || defined SFML_DEPRECATED
		return res->loadFromFile(file + ".vert", file + ".geom", file + ".frag");
#else
		return res->loadFromFile(file + ".vert", file + ".frag");
#endif

	return false;
}