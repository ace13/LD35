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

	bool hasClient(Client cid) const;
	sf::TcpSocket& getSocket(Client cid);
	const std::string& getName(Client cid) const;
	void setName(Client cid, const std::string& name);

	sf::SocketSelector getSelector();
	std::vector<Client> getClients(const sf::SocketSelector& selector);

	size_t numClients() const;

private:
	struct ClientObject
	{
		Client ID;
		std::unique_ptr<sf::TcpSocket> Socket;
		std::string Name;
	};

	std::unordered_map<Client, ClientObject> mClients;
	Client mClientCounter;
};
