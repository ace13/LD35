#include "Elements.as"

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
		Hooks::Add("Draw", "draw");

		font = Resources::GetFont("arial.ttf");
		ElementFade = 0;
		secs = 0;

		localPlayer = false;
#endif

		cl_InputValues = 0;
	}

	~Player()
	{
		if (!RELOADING)
		{
			Hooks::Remove("Tick", "tick");

#if CLIENT
			Hooks::Remove("Draw");
			Hooks::Remove("Update");
#endif
		}
	}

	string side()
	{
		#if CLIENT
		return "CLIENT";
		#endif
		#if SERVER
		return "SERVER";
		#endif
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

		if (!localPlayer)
			return;

		cl_InputValues = 0;
		if (sf::Keyboard::IsPressed(sf::Keyboard::W))
			cl_InputValues |= Input::Input_Up;
		if (sf::Keyboard::IsPressed(sf::Keyboard::S))
			cl_InputValues |= Input::Input_Down;
		if (sf::Keyboard::IsPressed(sf::Keyboard::A))
			cl_InputValues |= Input::Input_Left;
		if (sf::Keyboard::IsPressed(sf::Keyboard::D))
			cl_InputValues |= Input::Input_Right;
		if (sf::Mouse::IsPressed(sf::Mouse::Left))
		{
			cl_InputValues |= Input::Input_Fire;
			cl_Target = Rend.MapPixel(sf::Mouse::Position);
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
//#if SERVER
		sf::Vec2 targetVelocity(
			((cl_InputValues & Input::Input_Right) == Input::Input_Right ? 1 : 0) - ((cl_InputValues & Input::Input_Left) == Input::Input_Left ? 1 : 0),
			((cl_InputValues & Input::Input_Down) == Input::Input_Down ? 1 : 0) - ((cl_InputValues & Input::Input_Up) == Input::Input_Up ? 1 : 0)
		);

		sv_Velocity += (targetVelocity - sv_Velocity) * dt.Seconds * 2;
//#endif

		sv_Position += sv_Velocity * 250 * dt.Seconds;
	}

#if CLIENT
	void draw(sf::Renderer@ rend)
	{
		@Rend = rend;
		sf::CircleShape player(32);

		player.Origin = sf::Vec2(32,32);
		player.Position = sv_Position;

		player.Scale(sf::Vec2(1 + sin(secs * 2) / 10, 1 + sin(secs * 2) / 10));

		if (!(Element is null))
			player.FillColor = Element.Color;

		if ((cl_InputValues & Input::Input_Fire) == Input::Input_Fire)
		{
			sf::RectangleShape line(sf::Vec2(200, 1));
			line.Origin = sf::Vec2(0, 0.5);
			line.Position = sv_Position;
			line.Rotation = atan2(cl_Target.Y - sv_Position.Y, cl_Target.X - sv_Position.X) * RAD2DEG;

			if (!(Element is null))
				line.FillColor = Element.Color;

			rend.Draw(line);
		}

		rend.Draw(player);

		if (ElementFade > 0)
		{
			sf::Text name(Element.Name);

			name.SetFont(@font.Font);
			name.Position = sv_Position - sf::Vec2(0, 50);
			name.Origin = name.LocalBounds.Center;

			rend.Draw(name);
		}
	}

	bool localPlayer;

	sf::Renderer@ Rend;
	float secs, ElementFade;
	Resources::Font font;
#endif

	int cl_InputValues, cl_Element;
	sf::Vec2 sv_Position, sv_Velocity, cl_Target;

	IElement@ Element;
}
