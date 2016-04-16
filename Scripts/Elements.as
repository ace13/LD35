shared interface IElement
{
	string get_Name() const;
	sf::Color get_Color() const;
}

namespace Elements
{
	shared class Fire : IElement
	{
		string get_Name() const { return "Fire"; }
		sf::Color get_Color() const { return sf::Color(255,128,0); }
	}
	shared class Earth : IElement
	{
		string get_Name() const { return "Earth"; }
		sf::Color get_Color() const { return sf::Color(255,196,96); }
	}
	shared class Water : IElement
	{
		string get_Name() const { return "Water"; }
		sf::Color get_Color() const { return sf::Color(0,128,255); }
	}
	shared class Air : IElement
	{
		string get_Name() const { return "Air"; }
		sf::Color get_Color() const { return sf::Color(255,255,196); }
	}

	shared class Light : IElement
	{
		string get_Name() const { return "Light"; }
		sf::Color get_Color() const { return sf::Color(196,255,255); }
	}
	shared class Darkness : IElement
	{
		string get_Name() const { return "Darkness"; }
		sf::Color get_Color() const { return sf::Color(64,0,96); }
	}
}