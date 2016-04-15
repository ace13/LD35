#include "IntroState.hpp"
#include "MenuState.hpp"

#include <Client/StateManager.hpp>

using namespace States;

IntroState::IntroState()
{

}
IntroState::~IntroState()
{

}

void IntroState::enterState()
{
	mTime = std::chrono::seconds(0);

	puts("Entered the intro state");
}

void IntroState::update(const Timespan& dt)
{
	mTime += dt;
}
void IntroState::tick(const Timespan& dt)
{
	if (mTime > std::chrono::seconds(5))
		getSM().pushState(new MenuState(), true);
}
void IntroState::draw(sf::RenderTarget& rt) { }
void IntroState::drawUI(sf::RenderTarget& rt)
{

}
