#include "GameState.hpp"

#include <Client/ConnectionManager.hpp>
#include <Client/ServerContainer.hpp>
#include <Client/StateManager.hpp>
#include <Core/Engine.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

using namespace States;

GameState::GameState()
{

}
GameState::~GameState()
{
}

void GameState::enterState()
{
	auto& server = getSM().getEngine().get<ServerContainer>();
	auto& connection = getSM().getEngine().get<ConnectionManager>();

	server.init();
	server.launch();
	connection.connect(server.getPort(), sf::IpAddress::LocalHost);

	auto& rt = getSM().getEngine().get<sf::RenderWindow>();
	auto view = rt.getView();
	view.setCenter(0, 0);
	rt.setView(view);
}

void GameState::update(const Timespan& dt)
{

}
void GameState::tick(const Timespan& dt)
{
	auto& server = getSM().getEngine().get<ServerContainer>();
	server.tick();
}
void GameState::draw(sf::RenderTarget& rt)
{

}
void GameState::drawUI(sf::RenderTarget& rt)
{

}
