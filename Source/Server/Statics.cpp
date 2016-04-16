#include <Core/ScriptManager.hpp>
#include <Core/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

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