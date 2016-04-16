#include "NetworkedObject.hpp"

#include <Core/Math.hpp>

#include <SFML/Network/Packet.hpp>

#include <algorithm>

NetworkedObject::NetworkedObject()
	: mID(0)
	, mObject(nullptr)
	, mWeakRef(nullptr)
{
}
NetworkedObject::NetworkedObject(int id, asIScriptObject* obj)
	: mID(id)
	, mObject(nullptr)
	, mWeakRef(nullptr)
{
	updateObject(obj);
}
NetworkedObject::NetworkedObject(const NetworkedObject& other)
	: mID(other.mID)
	, mObject(nullptr)
	, mWeakRef(nullptr)
{
	updateObject(other.mObject);
}
NetworkedObject::NetworkedObject(NetworkedObject&& other)
	: mID(std::move(other.mID))
	, mObject(std::move(other.mObject))
	, mWeakRef(std::move(other.mWeakRef))
	, mTracked(std::move(other.mTracked))
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

		updateObject(other.mObject);
	}

	return *this;
}

void NetworkedObject::tick()
{
	mDirty.clear();
	mDirty.reserve(mTracked.size());
	for (auto& tracked : mTracked)
	{
		uint32_t newHash = Math::HashMemory(tracked.Address, tracked.Size);
		if (newHash != tracked.Hash)
		{
			mDirty.push_back(&tracked);
			tracked.Hash = newHash;
		}
	}
}
bool NetworkedObject::buildPacket(sf::Packet& out)
{
	if (mDirty.empty())
		return false;

	out << (uint32_t)mID;
	out << mDirty.size();

	for (auto& dirty : mDirty)
	{
		out << dirty->ID;
		out << dirty->Size;
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

int NetworkedObject::getID() const
{
	return mID;
}

void NetworkedObject::updateObject(asIScriptObject* newObj)
{
	if (mObject)
	{
		if (!mWeakRef->Get())
			mObject->Release();

		mWeakRef->Release();
	}

	mObject = newObj;
	mTracked.clear();
	mDirty.clear();

	if (mObject)
	{
		mObject->AddRef();
		mWeakRef = mObject->GetWeakRefFlag();
		mWeakRef->AddRef();

		for (uint32_t i = 0; i < mObject->GetPropertyCount(); ++i)
		{
			std::string name = mObject->GetPropertyName(i);
#if defined LD35_SERVER
			if (name.substr(0, 3) == "sv_")
#elif defined LD35_CLIENT
			if (name.substr(0, 3) == "cl_")
#else
#error "Unknown source project"
#endif
			{
				auto* addr = mObject->GetAddressOfProperty(i);
				auto type = mObject->GetEngine()->GetObjectTypeById(mObject->GetPropertyTypeId(i));
				uint32_t hash = Math::HashMemory(addr, type->GetSize());

				mTracked.push_back({
					uint8_t(i),
					type->GetSize(),
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