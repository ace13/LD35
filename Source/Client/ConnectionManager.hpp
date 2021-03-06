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

			/* Player ID information

			<uint32> ID      (0-2048)
			*/
			Type_PlayerID = 0x14A3,

			/* Player information

			<uint32> ID      (0-2048)
			<string> Name
			*/
			Type_Player = 0x0744,

			/* Object creation

			<uint32> ID      (0-2048 for player objects)
			<uint32> OwnerID (0-2048)
			<string> Module
			<string> Name
			*/
			Type_Create = 0xC347,

			/* Object destruction

			<uint32> ID
			*/
			Type_Destroy = 0xDE57,

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
