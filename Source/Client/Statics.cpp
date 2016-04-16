#include <Core/ScriptManager.hpp>
#include <Core/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

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