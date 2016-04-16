#include "Math.hpp"
#include "ScriptManager.hpp"

sf::Vector2f sf::operator*(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return{ a.x * b.x, a.y * b.y };
}
sf::Vector2f sf::operator/(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return{ a.x / b.x, a.y / b.y };
}
sf::Vector2f& sf::operator*=(sf::Vector2f& a, const sf::Vector2f& b)
{
	a.x *= b.x;
	a.y *= b.y;

	return a;
}
sf::Vector2f& sf::operator/=(sf::Vector2f& a, const sf::Vector2f& b)
{
	a.x /= b.x;
	a.y /= b.y;

	return a;
}

uint32_t Math::HashMemory(const void* mem, size_t size)
{
	static const uint32_t base = 2166136261;
	static const uint32_t prime = 16777619;

	const char* cdata = static_cast<const char*>(mem);

	uint32_t hash = base;
	for (size_t i = 0; i < size; ++i)
		hash = (hash ^ uint32_t(cdata[i])) * prime;
	return hash;
}

void Math::registerScriptData(ScriptManager& man)
{
	man.addExtension("Math types", [](asIScriptEngine* eng) {
		AS_ASSERT(eng->RegisterGlobalProperty("const float PI", (void*)&Math::PI));
		AS_ASSERT(eng->RegisterGlobalProperty("const float PI2", (void*)&Math::PI2));
		AS_ASSERT(eng->RegisterGlobalProperty("const float RAD2DEG", (void*)&Math::RAD2DEG));
		AS_ASSERT(eng->RegisterGlobalProperty("const float DEG2RAD", (void*)&Math::DEG2RAD));
	});
}
