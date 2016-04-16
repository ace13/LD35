#pragma once

#include <Client/IState.hpp>
#include <Client/ResourceManager.hpp>

namespace States
{

class MenuState : public IState
{
public:
	MenuState();
	~MenuState();

	virtual void enterState();

	virtual void update(const Timespan& dt);
	virtual void tick(const Timespan& dt);
	virtual void draw(sf::RenderTarget& rt);
	virtual void drawUI(sf::RenderTarget& rt);

private:
	ResourceManager::Font mFont;
};

}