#pragma once

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <deque>

class ConnectionManager
{
public:
	struct Event
	{
		enum EventTypes : uint16_t
		{
			Type_Script = 0x0597
		} Type;

		sf::Packet Data;
	};

	ConnectionManager();
	ConnectionManager(const ConnectionManager&) = delete;
	~ConnectionManager();

	ConnectionManager& operator=(const ConnectionManager&) = delete;

	void connect(uint16_t port, const sf::IpAddress& ip);
	void tick();

	bool hasEvent() const;
	bool pollEvent(Event& out);

private:
	sf::TcpSocket mSocket;

	std::deque<Event> mEvents;
};
