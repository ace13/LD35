#pragma once

#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <memory>
#include <unordered_map>

namespace sf { class Packet; }

class ConnectionManager
{
public:
	typedef int Client;
	static constexpr Client InvalidClient = -1;

	ConnectionManager();
	ConnectionManager(const ConnectionManager&) = delete;
	~ConnectionManager();

	ConnectionManager& operator=(const ConnectionManager&) = delete;

	Client addClient(std::unique_ptr<sf::TcpSocket>&& socket);
	void removeClient(Client cid);
		
	void sendPacketTo(Client cid, const sf::Packet& packet);
	void sendPacketToAll(const sf::Packet& packet);
	void sendPacketToAllBut(Client cid, const sf::Packet& packet);
	sf::TcpSocket& getSocket(Client cid);

	sf::SocketSelector getSelector();
	std::vector<Client> getClients(const sf::SocketSelector& selector);

	size_t numClients() const;

private:
	std::unordered_map<Client, std::unique_ptr<sf::TcpSocket>> mClients;
	Client mClientCounter;
};
