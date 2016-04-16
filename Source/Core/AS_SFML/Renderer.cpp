#include "Shared.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	const sf::Vector2f& getCenter(const sf::RenderTarget* target)
	{
		return target->getView().getCenter();
	}
	void setCenter(sf::RenderTarget* target, const sf::Vector2f& vec)
	{
		auto view = target->getView();
		view.setCenter(vec);
		target->setView(view);
	}
	const sf::Vector2f& getSize(const sf::RenderTarget* target)
	{
		return target->getView().getSize();
	}
	void setSize(sf::RenderTarget* target, const sf::Vector2f& vec)
	{
		auto view = target->getView();
		view.setSize(vec);
		target->setView(view);
	}

	void mapPixelToCoord(asIScriptGeneric* gen)
	{
		sf::RenderTarget* target = reinterpret_cast<sf::RenderTarget*>(gen->GetObject());
		const sf::Vector2f* pixel = reinterpret_cast<sf::Vector2f*>(gen->GetArgObject(0));
		new (gen->GetAddressOfReturnLocation()) sf::Vector2f(target->mapPixelToCoords(sf::Vector2i(*pixel)));
	}
	void mapCoordToPixel(asIScriptGeneric* gen)
	{
		sf::RenderTarget* target = reinterpret_cast<sf::RenderTarget*>(gen->GetObject());
		const sf::Vector2f* coord = reinterpret_cast<sf::Vector2f*>(gen->GetArgObject(0));
		new (gen->GetAddressOfReturnLocation()) sf::Vector2f(target->mapCoordsToPixel(*coord));
	}
}

void as::priv::RegRenderer(asIScriptEngine* eng)
{
	AS_ASSERT(eng->SetDefaultNamespace("sf"));

	AS_ASSERT(eng->RegisterObjectType("Renderer", 0, asOBJ_REF | asOBJ_NOCOUNT));

	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "const Vec2& get_Center() const", asFUNCTION(getCenter), asCALL_CDECL_OBJFIRST));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "void set_Center(const Vec2&in)", asFUNCTION(setCenter), asCALL_CDECL_OBJFIRST));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "const Vec2& get_Size() const", asFUNCTION(getSize), asCALL_CDECL_OBJFIRST));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "void set_Size(const Vec2&in)", asFUNCTION(setSize), asCALL_CDECL_OBJFIRST));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "const View& get_View() const", asMETHOD(sf::RenderTarget, getView), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "void set_View(const View&in)", asMETHOD(sf::RenderTarget, setView), asCALL_THISCALL));

	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "Vec2 MapPixel(const Vec2&in)", asFUNCTION(mapPixelToCoord), asCALL_GENERIC));
	AS_ASSERT(eng->RegisterObjectMethod("Renderer", "Vec2 MapCoord(const Vec2&in)", asFUNCTION(mapCoordToPixel), asCALL_GENERIC));

	AS_ASSERT(eng->SetDefaultNamespace(""));
}
