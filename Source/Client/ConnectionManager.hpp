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
			/* Script data

			<string> Module
			<char[]> Bytecode
			*/
			Type_Script = 0x0597,

			/* Object creation

			<uint32> ID
			<string> Module
			<string> Name
			*/
			Type_Create = 0xC347,

			/* Object update

			Repeating:
			<uint32> ID
			<uint8> PropertyCount

			Repeating:
			<uint8> PropertyID
			<uint8> Bytes
			<char[Bytes]> Memory
			*/
			Type_Update = 0x6473
		} Type;

		sf::Packet Data;
	};

	ConnectionManager();
	ConnectionManager(const ConnectionManager&) = delete;
	~ConnectionManager();

	ConnectionManager& operator=(const ConnectionManager&) = delete;

	void connect(uint16_t port, const sf::IpAddress& ip);
	void tick();
	void send(sf::Packet& p);

	bool hasEvent() const;
	bool pollEvent(Event& out);

private:
	sf::TcpSocket mSocket;

	std::deque<Event> mEvents;
};
