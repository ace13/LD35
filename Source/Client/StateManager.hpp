#pragma once

#include "IState.hpp"

#include <list>

class Engine;

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

	Engine& getEngine() { return *mEngine; }
	void setEngine(Engine& engine) { mEngine = &engine; }

private:
	IState* mCurState;
	std::list<IState*> mOldStates;

	Engine* mEngine;
};
