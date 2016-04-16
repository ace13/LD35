#include "NetworkedObject.hpp"

#include <Core/Math.hpp>

#include <SFML/Network/Packet.hpp>

#include <algorithm>

namespace
{
	int localID = -1;
}

NetworkedObject::NetworkedObject()
	: mID(0)
	, mOwnerID(-1)
	, mObject(nullptr)
	, mWeakRef(nullptr)
	, mWaitTime(0)
	, mSyncTime(0)
{
}
NetworkedObject::NetworkedObject(int id, asIScriptObject* obj)
	: mID(id)
	, mOwnerID(-1)
	, mObject(nullptr)
	, mWeakRef(nullptr)
	, mWaitTime(0)
	, mSyncTime(0)
{
	updateObject(obj);
}
NetworkedObject::NetworkedObject(const NetworkedObject& other)
	: mID(other.mID)
	, mOwnerID(other.mOwnerID)
	, mObject(nullptr)
	, mWeakRef(nullptr)
	, mWaitTime(other.mWaitTime)
	, mSyncTime(other.mSyncTime)
{
	updateObject(other.mObject);
}
NetworkedObject::NetworkedObject(NetworkedObject&& other)
	: mID(std::move(other.mID))
	, mOwnerID(std::move(other.mOwnerID))
	, mObject(std::move(other.mObject))
	, mWeakRef(std::move(other.mWeakRef))
	, mTracked(std::move(other.mTracked))
	, mWaitTime(std::move(other.mWaitTime))
	, mSyncTime(std::move(other.mSyncTime))
{
	other.mObject = nullptr;
	other.mWeakRef = nullptr;
	other.mDirty.clear();
}
NetworkedObject::~NetworkedObject()
{
	if (mObject)
	{
		if (!mWeakRef->Get())
			mObject->Release();

		mWeakRef->Release();
	}
}

NetworkedObject& NetworkedObject::operator=(const NetworkedObject& other)
{
	if (this != &other)
	{
		mID = other.mID;
		mWaitTime = other.mWaitTime;

		updateObject(other.mObject);
	}

	return *this;
}

void NetworkedObject::tick(const Timespan& dt)
{
	if (localID >= 0 && mOwnerID != localID)
		return;

	mWaitTime += dt;

	if (mWaitTime > std::chrono::milliseconds(UpdateTick))
	{
		mWaitTime = Timespan(0);

		mDirty.clear();
		mDirty.reserve(mTracked.size());
		for (auto& tracked : mTracked)
		{
			if (!tracked.Owned)
				continue;

			uint32_t newHash = Math::HashMemory(tracked.Address, tracked.Size);
			if (newHash != tracked.Hash)
			{
				mDirty.push_back(&tracked);
				tracked.Hash = newHash;
			}
		}
	}

	if (localID < 0)
	{
		mSyncTime += dt;

		if (mSyncTime > std::chrono::milliseconds(WorldResync))
		{
			mSyncTime = Timespan(0);

			mDirty.clear();
			mDirty.reserve(mTracked.size());
			for (auto& tracked : mTracked)
				if (tracked.Owned)
					mDirty.push_back(&tracked);
		}
	}
}
bool NetworkedObject::buildCreatePacket(sf::Packet& out)
{
	if (!mObject)
		return false;

	out << uint32_t(mID);
	out << uint32_t(mOwnerID);
	out << mObject->GetObjectType()->GetModule()->GetName();
	out << mObject->GetObjectType()->GetName();

	return true;
}
bool NetworkedObject::buildPacket(sf::Packet& out)
{
	if (mDirty.empty())
		return false;

	out << uint32_t(mID);
	out << uint8_t(mDirty.size());

	for (auto& dirty : mDirty)
	{
		out << uint8_t(dirty->ID);
		out << uint8_t(dirty->Size);
		out.append(dirty->Address, dirty->Size);
	}

	mDirty.clear();

	return true;
}
bool NetworkedObject::injectPacket(sf::Packet& in)
{
	uint8_t count;
	in >> count;

	for (uint8_t i = 0; i < count; ++i)
	{
		uint8_t id, size;
		in >> id >> size;

		auto& it = std::find_if(mTracked.begin(), mTracked.end(), [id](TrackedProperty& prop) { return prop.ID == id; });
		if (it != mTracked.end())
		{
			std::vector<uint8_t> data(size);
			for (int i = 0; i < size; ++i)
			{
				uint8_t c;
				in >> c;
				data[i] = std::move(c);
			}

			std::memcpy(it->Address, data.data(), size);
		}
	}

	return true;
}

asIScriptObject* NetworkedObject::getObject()
{
	return mObject;
}

int NetworkedObject::getID() const
{
	return mID;
}

void NetworkedObject::setOwner(int id)
{
	mOwnerID = id;
}

void NetworkedObject::setLocalID(int id)
{
	localID = id;
}
int NetworkedObject::getLocalID()
{
	return localID;
}

void NetworkedObject::updateObject(asIScriptObject* newObj)
{
	if (mObject)
	{
		auto* man = reinterpret_cast<ScriptManager*>(mObject->GetEngine()->GetUserData(0x4547));

		if (!mWeakRef->Get())
			mObject->Release();

		mWeakRef->Release();

		man->removeChangeNotice(mObject, "NetworkedObject");
	}

	mObject = newObj;
	mTracked.clear();
	mDirty.clear();

	if (mObject)
	{
		mObject->AddRef();
		mWeakRef = mObject->GetWeakRefFlag();
		mWeakRef->AddRef();

		uint32_t id = 0;
		for (uint32_t i = 0; i < mObject->GetPropertyCount(); ++i)
		{
			std::string name = mObject->GetPropertyName(i);
			if (name.substr(0, 3) == "sv_" || name.substr(0, 3) == "cl_")
			{
				auto* addr = mObject->GetAddressOfProperty(i);
				auto typeId = mObject->GetPropertyTypeId(i);

				size_t size;
				if (typeId & asTYPEID_MASK_OBJECT)
				{
					auto type = mObject->GetEngine()->GetObjectTypeById(typeId);
					size = type->GetSize();
				}
				else
				{
					switch (typeId)
					{
					case asTYPEID_BOOL:
					case asTYPEID_INT8:
					case asTYPEID_UINT8:
						size = 1; break;

					case asTYPEID_INT16:
					case asTYPEID_UINT16:
						size = 2; break;

					case asTYPEID_FLOAT:
					case asTYPEID_INT32:
					case asTYPEID_UINT32:
						size = 4; break;

					case asTYPEID_DOUBLE:
					case asTYPEID_INT64:
					case asTYPEID_UINT64:
						size = 8; break;
					}
				}

				bool owned =
#if defined LD35_CLIENT
					name.substr(0, 3) == "cl_";
#elif defined LD35_SERVER
					name.substr(0, 3) == "sv_";
#endif

				uint32_t hash = owned ? Math::HashMemory(addr, size) : 0;

				mTracked.push_back({
					uint8_t(id++),
					owned,
#ifdef _DEBUG
					name,
#endif
					size,
					addr,
					hash
				});
			}
		}

		auto* man = reinterpret_cast<ScriptManager*>(mObject->GetEngine()->GetUserData(0x4547));
		man->addChangeNotice(mObject, "NetworkedObject", std::bind1st(std::mem_fn(&NetworkedObject::updateObject), this));
	}
	else
	{
		mWeakRef = nullptr;
	}
}