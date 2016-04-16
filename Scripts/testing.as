#include "Elements.as"

Player@ p;

void OnLoad()
{
	@p = Player();
}

namespace Input
{
	enum Values
	{
		Input_Up    = 1 << 0,
		Input_Down  = 1 << 1,
		Input_Left  = 1 << 2,
		Input_Right = 1 << 3,
		Input_Fire  = 1 << 4
	}
}

class Player
{

	Player()
	{
		Hooks::Add("Tick", "tick");

#if CLIENT
		Hooks::Add("Update", "update");
		Hooks::Add("DrawUI", "draw");

		font = Resources::GetFont("arial.ttf");
		ElementFade = 0;
#endif
		InputValues = 0;
	}

	~Player()
	{
		if (!RELOADING)
		{
			Hooks::Remove("Tick", "tick");

#if CLIENT
			Hooks::Remove("DrawUI");
			Hooks::Remove("Update");
#endif
		}
	}

#if CLIENT
	float max(float a, float b)
	{
		return (a < b ? b : a);
	}

	void update(const Timespan&in dt)
	{
		secs += dt.Seconds;
		if (ElementFade > 0)
			ElementFade = max(0, ElementFade - dt.Seconds);

		InputValues = 0;

		if (sf::Keyboard::IsPressed(sf::Keyboard::W))
			InputValues |= Input::Input_Up;
		if (sf::Keyboard::IsPressed(sf::Keyboard::S))
			InputValues |= Input::Input_Down;
		if (sf::Keyboard::IsPressed(sf::Keyboard::A))
			InputValues |= Input::Input_Left;
		if (sf::Keyboard::IsPressed(sf::Keyboard::D))
			InputValues |= Input::Input_Right;
		if (sf::Mouse::IsPressed(sf::Mouse::Left))
		{
			InputValues |= Input::Input_Fire;
			Target = sf::Mouse::Position;
		}

		if (sf::Keyboard::IsPressed(sf::Keyboard::Num1))
		{
			@Element = Elements::Fire();
			ElementFade = 1;
		}
		if (sf::Keyboard::IsPressed(sf::Keyboard::Num2))
		{
			@Element = Elements::Earth();
			ElementFade = 1;
		}
		if (sf::Keyboard::IsPressed(sf::Keyboard::Num3))
		{
			@Element = Elements::Water();
			ElementFade = 1;
		}
		if (sf::Keyboard::IsPressed(sf::Keyboard::Num4))
		{
			@Element = Elements::Air();
			ElementFade = 1;
		}
		if (sf::Keyboard::IsPressed(sf::Keyboard::Num5))
		{
			@Element = Elements::Light();
			ElementFade = 1;
		}
		if (sf::Keyboard::IsPressed(sf::Keyboard::Num6))
		{
			@Element = Elements::Darkness();
			ElementFade = 1;
		}
	}
#endif

	void tick(const Timespan&in dt)
	{
#if CLIENT
/*
		sf::Packet packet;
		packet << InputValues;

*/
#endif

		sf::Vec2 targetVelocity(
			((InputValues & Input::Input_Right) == Input::Input_Right ? 1 : 0) - ((InputValues & Input::Input_Left) == Input::Input_Left ? 1 : 0),
			((InputValues & Input::Input_Down) == Input::Input_Down ? 1 : 0) - ((InputValues & Input::Input_Up) == Input::Input_Up ? 1 : 0)
		);
		Velocity += (targetVelocity - Velocity) * dt.Seconds * 2;
		Position += Velocity * 100 * dt.Seconds;
	}

#if CLIENT
	void draw(sf::Renderer@ rend)
	{
		sf::CircleShape player(32);

		player.Origin = sf::Vec2(32,32);
		player.Position = Position;

		player.Scale(sf::Vec2(1 + sin(secs * 2) / 10, 1 + sin(secs * 2) / 10));

		if (!(Element is null))
			player.FillColor = Element.Color;

		if ((InputValues & Input::Input_Fire) == Input::Input_Fire)
		{
			sf::RectangleShape line(sf::Vec2(200, 1));
			line.Origin = sf::Vec2(0, 0.5);
			line.Position = Position;
			line.Rotation = atan2(Target.Y - Position.Y, Target.X - Position.X) * RAD2DEG;

			if (!(Element is null))
				line.FillColor = Element.Color;

			rend.Draw(line);
		}

		rend.Draw(player);

		if (ElementFade > 0)
		{
			sf::Text name(Element.Name);

			name.SetFont(@font.Font);
			name.Position = Position - sf::Vec2(0, 50);
			name.Origin = name.LocalBounds.Center;

			rend.Draw(name);
		}
	}

	float secs, ElementFade;
	Resources::Font font;
#endif

	int InputValues;
	sf::Vec2 Position, Velocity, Target;
	IElement@ Element;
}
