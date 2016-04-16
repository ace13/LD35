#include "MenuState.hpp"

#include <Client/StateManager.hpp>
#include <Core/Engine.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>

using namespace States;

MenuState::MenuState()
{
	puts("Entered the menu state");
}
MenuState::~MenuState()
{

}

void MenuState::enterState()
{
	mFont = getSM().getEngine().get<ResourceManager>().get<sf::Font>("arial.ttf");
}

void MenuState::update(const Timespan& dt)
{

}
void MenuState::tick(const Timespan& dt)
{

}
void MenuState::draw(sf::RenderTarget& rt)
{

}
void MenuState::drawUI(sf::RenderTarget& rt)
{
	sf::Vector2f mouse(sf::Mouse::getPosition(getSM().getEngine().get<sf::RenderWindow>()));

	sf::RectangleShape box;
	sf::Text label("", *mFont);

	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color::White);
	box.setOutlineThickness(2.f);

	box.setPosition(rt.getView().getSize().x / 2, 100);

	label.setString("Singleplayer");
	auto rect = label.getLocalBounds();
	label.setOrigin(rect.width / 2, rect.height / 2);
	box.setSize({ rect.width + 20, rect.height + 20 });
	box.setOrigin(rect.width / 2 + 10, rect.height / 2 + 10);
	label.setPosition(box.getPosition());

	rect = box.getGlobalBounds();
	if (rect.contains(mouse))
		box.setFillColor(sf::Color(255, 255, 0, 128));

	rt.draw(box);
	rt.draw(label);

	box.setFillColor(sf::Color::Transparent);
	box.setPosition(rt.getView().getSize().x / 2, 180);

	label.setString("Multiplayer");
	rect = label.getLocalBounds();
	label.setOrigin(rect.width / 2, rect.height / 2);
	box.setSize({ rect.width + 20, rect.height + 20 });
	box.setOrigin(rect.width / 2 + 10, rect.height / 2 + 10);
	label.setPosition(box.getPosition());

	rect = box.getGlobalBounds();
	if (rect.contains(mouse))
		box.setFillColor(sf::Color(255, 255, 0, 128));

	rt.draw(box);
	rt.draw(label);
}
