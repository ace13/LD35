#pragma once

#include <Core/Time.hpp>

namespace sf { class RenderTarget; }
class StateManager;

class IState
{
public:
	virtual ~IState() = default;
	
	virtual void enterState() { }
	virtual void exitState() { }

	virtual void update(const Timespan& dt) = 0;
	virtual void tick(const Timespan& dt) = 0;
	virtual void draw(sf::RenderTarget& rt) = 0;
	virtual void drawUI(sf::RenderTarget& rt) = 0;

protected:
	StateManager& getSM() { return *mManager; }

private:
	friend class StateManager;

	StateManager* mManager;
};
