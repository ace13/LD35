#pragma once

#pragma once

#include <Client/IState.hpp>
#include <Client/ConnectionManager.hpp>
#include <Client/ServerContainer.hpp>
#include <Shared/NetworkedObject.hpp>

#include <SFML/Network/IpAddress.hpp>

#include <unordered_map>

namespace States
{

class GameState : public IState
{
public:
	GameState();
	GameState(uint16_t port, const sf::IpAddress& address);
	~GameState();

	virtual void enterState();

	virtual void update(const Timespan& dt);
	virtual void tick(const Timespan& dt);
	virtual void draw(sf::RenderTarget& rt);
	virtual void drawUI(sf::RenderTarget& rt);

private:
	bool mLocal;
	uint16_t mPort;
	sf::IpAddress mAddress;

	std::unordered_map<uint32_t, NetworkedObject> mObjects;

	ConnectionManager* mConnection;
	ServerContainer* mServer;
};

}