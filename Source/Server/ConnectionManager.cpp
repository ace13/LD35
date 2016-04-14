#include "ConnectionManager.hpp"

#include <SFML/Network/Packet.hpp>

ConnectionManager::ConnectionManager()
	: mClientCounter(0)
{

}
ConnectionManager::~ConnectionManager()
{
}

ConnectionManager::Client ConnectionManager::addClient(std::unique_ptr<sf::TcpSocket>&& socket)
{
	auto pair = mClients.emplace(mClientCounter++, std::move(socket));

	if (pair.second)
	{
		return pair.first->first;
	}

	return InvalidClient;
}

void ConnectionManager::removeClient(Client cid)
{
	mClients.erase(cid);
}

void ConnectionManager::sendPacketToAll(const sf::Packet& p)
{
	for (auto& client : mClients)
	{
		sf::Packet copy = p;
		sf::Socket::Status ret = sf::Socket::Partial;

		while (ret == sf::Socket::Partial)
			ret = client.second->send(copy);
	}
}
void ConnectionManager::sendPacketTo(Client cid, const sf::Packet& p)
{
	if (mClients.count(cid) > 0)
	{
		auto& client = mClients.at(cid);
		sf::Packet copy = p;
		sf::Socket::Status ret = sf::Socket::Partial;

		while (ret == sf::Socket::Partial)
			ret = client->send(copy);
	}
}
void ConnectionManager::sendPacketToAllBut(Client cid, const sf::Packet& p)
{
	for (auto& client : mClients)
	{
		if (client.first == cid)
			continue;

		sf::Packet copy = p;
		sf::Socket::Status ret = sf::Socket::Partial;

		while (ret == sf::Socket::Partial)
			ret = client.second->send(copy);
	}
}
sf::TcpSocket& ConnectionManager::getSocket(Client cid)
{
	return *mClients.at(cid);
}
sf::SocketSelector ConnectionManager::getSelector()
{
	sf::SocketSelector selector;

	for (auto& client : mClients)
		selector.add(*client.second);

	return selector;
}
std::vector<ConnectionManager::Client> ConnectionManager::getClients(const sf::SocketSelector& selector)
{
	std::vector<Client> ready;

	for (auto& client : mClients)
		if (selector.isReady(*client.second))
			ready.push_back(client.first);

	return ready;
}

size_t ConnectionManager::numClients() const
{
	return mClients.size();
}
