#pragma once

#include <Client/IState.hpp>

namespace States
{

class IntroState : public IState
{
public:
	IntroState();
	~IntroState();
	
	virtual void enterState();

	virtual void update(const Timespan& dt);
	virtual void tick(const Timespan& dt);
	virtual void draw(sf::RenderTarget& rt);
	virtual void drawUI(sf::RenderTarget& rt);

private:
	Timespan mTime;
};

}