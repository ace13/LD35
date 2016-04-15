#include "StateManager.hpp"

StateManager::StateManager()
	: mCurState(nullptr)
{

}
StateManager::~StateManager()
{
	if (mCurState)
	{
		mCurState->exitState();
		delete mCurState;
	}

	for (auto& state : mOldStates)
	{
		state->exitState();
		delete state;
	}
}

void StateManager::pushState(IState* newState, bool removeCur)
{
	if (mCurState)
		mCurState->exitState();

	if (!removeCur && mCurState)
		mOldStates.push_back(mCurState);
	else if (removeCur && mCurState)
		delete mCurState;

	newState->mManager = this;
	newState->enterState();
	mCurState = newState;
}
void StateManager::popState()
{
	if (mCurState)
	{
		mCurState->exitState();
		delete mCurState;
	}

	mCurState = nullptr;

	if (mOldStates.empty())
		return;

	mCurState = mOldStates.back();
	mOldStates.pop_back();

	mCurState->enterState();
}

void StateManager::tick(const Timespan& dt)
{
	if (mCurState)
		mCurState->tick(dt);
}
void StateManager::update(const Timespan& dt)
{
	if (mCurState)
		mCurState->update(dt);
}
void StateManager::draw(sf::RenderTarget& rt)
{
	if (mCurState)
		mCurState->draw(rt);
}
void StateManager::drawUI(sf::RenderTarget& rt)
{
	if (mCurState)
		mCurState->drawUI(rt);
}
