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

//		font = Resources::GetFont("arial.ttf");
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
	void update(const Timespan&in dt)
	{
		secs += dt.Seconds;

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

		rend.Draw(player);

		if ((InputValues & Input::Input_Fire) == Input::Input_Fire)
		{
			sf::RectangleShape line(sf::Vec2(200, 1));
			line.Origin = sf::Vec2(0, 0.5);
			line.Position = Position;
			line.Rotation = atan2(Target.Y - Position.Y, Target.X - Position.X) * RAD2DEG;

			rend.Draw(line);
		}
	}

	float secs;
#endif

	int InputValues;
	sf::Vec2 Position, Velocity, Target;
}
