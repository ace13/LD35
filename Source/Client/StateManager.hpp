#pragma once

#include "IState.hpp"

#include <list>

class StateManager
{
public:
	StateManager();
	StateManager(const StateManager&) = delete;
	~StateManager();

	StateManager& operator=(const StateManager&) = delete;

	void pushState(IState* newState, bool removeCur = false);
	void popState();

	void tick(const Timespan& dt);
	void update(const Timespan& dt);
	void draw(sf::RenderTarget& rt);
	void drawUI(sf::RenderTarget& rt);

private:
	IState* mCurState;
	std::list<IState*> mOldStates;
};
