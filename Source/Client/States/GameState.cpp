#include "GameState.hpp"

#include <Client/ConnectionManager.hpp>
#include <Client/ServerContainer.hpp>
#include <Client/StateManager.hpp>
#include <Core/Engine.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <iostream>

using namespace States;

GameState::GameState()
	: mLocal(true)
{

}
GameState::GameState(uint16_t port, const sf::IpAddress& addr)
	: mLocal(false)
	, mPort(port)
	, mAddress(addr)
{

}

GameState::~GameState()
{
}

void GameState::enterState()
{
	if (mLocal)
	{
		mServer = &getSM().getEngine().get<ServerContainer>();
		auto& server = *mServer;
		server.init();
		server.launch();

		mAddress = sf::IpAddress::LocalHost;
		mPort = server.getPort();
	}

	mConnection = &getSM().getEngine().get<ConnectionManager>();
	auto& connection = *mConnection;

	connection.connect(mPort, mAddress);

	auto& rt = getSM().getEngine().get<sf::RenderWindow>();
	auto view = rt.getView();
	view.setCenter(0, 0);
	view.zoom(0.5);
	rt.setView(view);
}

void GameState::update(const Timespan& dt)
{

}
void GameState::tick(const Timespan& dt)
{
	for (auto& obj : mObjects)
	{
		obj.second.tick();

		sf::Packet p;
		if (obj.second.buildPacket(p))
			mConnection->send(p);
	}

	mServer->tick();
	mConnection->tick();

	ConnectionManager::Event netEv;
	if (mConnection->pollEvent(netEv))
	{
		switch (netEv.Type)
		{
		case ConnectionManager::Event::Type_Script:
			{
				std::string name;
				netEv.Data >> name;

				ScriptManager::BytecodeStore store;
				do
				{
					uint8_t c;
					netEv.Data >> c;
					store.Write(&c, 1);
				} while (netEv.Data);

				std::cout << "Received script state for " << name << " from the server, integrating..." << std::endl;
				if (!getSM().getEngine().get<ScriptManager>().loadFromStream(name, store))
					std::cout << "Integration of new script code failed." << std::endl;
			} break;
		case ConnectionManager::Event::Type_Create:
			{
				uint32_t id;
				netEv.Data >> id;
				std::string mod, obj;
				netEv.Data >> mod >> obj;

				auto& sman = getSM().getEngine().get<ScriptManager>();

				auto* module = sman.getEngine()->GetModule(mod.c_str());
				if (module)
				{
					auto* objtype = module->GetObjectTypeByName(obj.c_str());
					auto* scriptobj = reinterpret_cast<asIScriptObject*>(sman.getEngine()->CreateScriptObject(objtype));

					mObjects[id] = NetworkedObject(id, scriptobj);
				}
			} break;
		case ConnectionManager::Event::Type_Update:
			{
				do
				{
					uint32_t id;
					netEv.Data >> id;
					
					if (mObjects.count(id) > 0)
						mObjects[id].injectPacket(netEv.Data);
					else
						break;
				} while (netEv.Data && !netEv.Data.endOfPacket());
			} break;
		}
	}
}
void GameState::draw(sf::RenderTarget& rt)
{

}
void GameState::drawUI(sf::RenderTarget& rt)
{

}
