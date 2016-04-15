#pragma once

#include <Client/IState.hpp>

namespace States
{

class MenuState : public IState
{
public:
	MenuState();
	~MenuState();

	virtual void update(const Timespan& dt);
	virtual void tick(const Timespan& dt);
	virtual void draw(sf::RenderTarget& rt);
	virtual void drawUI(sf::RenderTarget& rt);

private:
};

}