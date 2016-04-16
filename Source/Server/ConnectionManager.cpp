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
	auto pair = mClients.emplace(mClientCounter, ClientObject{ mClientCounter, std::move(socket), "" });

	while (mClients.count(mClientCounter) > 0)
		mClientCounter = (mClientCounter + 1) % 2048;

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
			ret = client.second.Socket->send(copy);
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
			ret = client.Socket->send(copy);
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
			ret = client.second.Socket->send(copy);
	}
}

bool ConnectionManager::hasClient(Client cid) const
{
	return mClients.count(cid) > 0;
}

sf::TcpSocket& ConnectionManager::getSocket(Client cid)
{
	return *mClients.at(cid).Socket;
}

const std::string& ConnectionManager::getName(Client cid) const
{
	return mClients.at(cid).Name;
}
void ConnectionManager::setName(Client cid, const std::string& name)
{
	mClients.at(cid).Name = name;
}

sf::SocketSelector ConnectionManager::getSelector()
{
	sf::SocketSelector selector;

	for (auto& client : mClients)
		selector.add(*client.second.Socket);

	return selector;
}
std::vector<ConnectionManager::Client> ConnectionManager::getClients(const sf::SocketSelector& selector)
{
	std::vector<Client> ready;

	for (auto& client : mClients)
		if (selector.isReady(*client.second.Socket))
			ready.push_back(client.first);

	return ready;
}

size_t ConnectionManager::numClients() const
{
	return mClients.size();
}
