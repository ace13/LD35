#pragma once

#include <Core/ScriptManager.hpp>
#include <vector>

namespace sf { class Packet; }

class NetworkedObject
{
public:
	enum
	{
		UpdateTick = 1000 / 20
	};

	NetworkedObject();
	NetworkedObject(int id, asIScriptObject* obj);
	NetworkedObject(const NetworkedObject&);
	NetworkedObject(NetworkedObject&&);
	~NetworkedObject();

	NetworkedObject& operator=(const NetworkedObject&);

	void tick();
	bool buildPacket(sf::Packet& out);
	bool injectPacket(sf::Packet& in);

	int getID() const;

private:
	void updateObject(asIScriptObject* newObj);

	struct TrackedProperty
	{
		uint8_t ID;
		size_t Size;
		void* Address;
		uint32_t Hash;
	};

	int mID;
	asIScriptObject* mObject;
	asILockableSharedBool* mWeakRef;

	std::vector<TrackedProperty> mTracked;
	std::vector<TrackedProperty*> mDirty;
};