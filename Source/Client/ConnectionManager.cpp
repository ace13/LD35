#include "ConnectionManager.hpp"

ConnectionManager::ConnectionManager()
{
	/*
	mSocket.connect(sf::IpAddress::getLocalAddress(), 42035);
	*/
	mSocket.setBlocking(false);
}
ConnectionManager::~ConnectionManager()
{

}

void ConnectionManager::connect(uint16_t port, const sf::IpAddress& ip)
{
	mSocket.connect(ip, port, sf::seconds(5));
}

void ConnectionManager::tick()
{
	sf::Packet temp;
	sf::Socket::Status ret = sf::Socket::Partial;
	while (ret == sf::Socket::Partial)
		ret = mSocket.receive(temp);

	if (ret == sf::Socket::Done)
	{
		uint16_t packetType;
		temp >> packetType;

		mEvents.push_back({
			(Event::EventTypes)packetType,
			std::move(temp)
		});
	}
}

void ConnectionManager::send(sf::Packet& inpPacket)
{
	auto ret = sf::Socket::Partial;
	while (ret == sf::Socket::Partial)
		ret = mSocket.send(inpPacket);
}

bool ConnectionManager::hasEvent() const
{
	return !mEvents.empty();
}
bool ConnectionManager::pollEvent(Event& out)
{
	if (mEvents.empty())
		return false;

	out = mEvents.back();
	mEvents.pop_back();

	return true;
}