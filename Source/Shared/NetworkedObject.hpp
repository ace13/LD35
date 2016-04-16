#pragma once

#include <Core/ScriptManager.hpp>
#include <Core/Time.hpp>

#include <vector>

namespace sf { class Packet; }

class NetworkedObject
{
public:
	enum
	{
		UpdateTick = 1000 / 10
	};

	NetworkedObject();
	NetworkedObject(int id, asIScriptObject* obj);
	NetworkedObject(const NetworkedObject&);
	NetworkedObject(NetworkedObject&&);
	~NetworkedObject();

	NetworkedObject& operator=(const NetworkedObject&);

	void tick(const Timespan& dt);
	bool buildCreatePacket(sf::Packet& out);
	bool buildPacket(sf::Packet& out);
	bool injectPacket(sf::Packet& in);

	int getID() const;

private:
	void updateObject(asIScriptObject* newObj);

	struct TrackedProperty
	{
		uint8_t ID;
		bool Owned;

#ifdef _DEBUG
		std::string Name;
#endif

		size_t Size;
		void* Address;
		uint32_t Hash;
	};

	int mID;
	asIScriptObject* mObject;
	asILockableSharedBool* mWeakRef;

	std::vector<TrackedProperty> mTracked;
	std::vector<TrackedProperty*> mDirty;

	Timespan mWaitTime;
};