#include "Shared.hpp"

#include <SFML/Network/Packet.hpp>

namespace
{
	bool opImplConvPacket(const sf::Packet& p)
	{
		return p;
	}
}

void as::priv::RegPacket(asIScriptEngine* eng)
{
	AS_ASSERT(eng->SetDefaultNamespace("sf"));

	AS_ASSERT(eng->RegisterObjectType("Packet", 0, asOBJ_REF | asOBJ_NOCOUNT));

	AS_ASSERT(eng->RegisterObjectMethod("Packet", "bool opImplConv() const", asFUNCTION(opImplConvPacket), asCALL_CDECL_OBJLAST));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "bool get_AtEnd() const", asMETHOD(sf::Packet, endOfPacket), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "uint32 get_Size() const", asMETHOD(sf::Packet, getDataSize), asCALL_THISCALL));

	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(bool)",   asMETHODPR(sf::Packet, operator<<, (bool),     sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(int8)",   asMETHODPR(sf::Packet, operator<<, (int8_t),   sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(uint8)",  asMETHODPR(sf::Packet, operator<<, (uint8_t),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(int16)",  asMETHODPR(sf::Packet, operator<<, (int16_t),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(uint16)", asMETHODPR(sf::Packet, operator<<, (uint16_t), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(int32)",  asMETHODPR(sf::Packet, operator<<, (int32_t),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(uint32)", asMETHODPR(sf::Packet, operator<<, (uint32_t), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(int64)",  asMETHODPR(sf::Packet, operator<<, (int64_t),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(uint64)", asMETHODPR(sf::Packet, operator<<, (uint64_t), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(float)",  asMETHODPR(sf::Packet, operator<<, (float),    sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(double)", asMETHODPR(sf::Packet, operator<<, (double),   sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShl(string&in)", asMETHODPR(sf::Packet, operator<<, (const std::string&), sf::Packet&), asCALL_THISCALL));

	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(bool&out)",   asMETHODPR(sf::Packet, operator>>, (bool&),     sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(int8&out)",   asMETHODPR(sf::Packet, operator>>, (int8_t&),   sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(uint8&out)",  asMETHODPR(sf::Packet, operator>>, (uint8_t&),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(int16&out)",  asMETHODPR(sf::Packet, operator>>, (int16_t&),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(uint16&out)", asMETHODPR(sf::Packet, operator>>, (uint16_t&), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(int32&out)",  asMETHODPR(sf::Packet, operator>>, (int32_t&),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(uint32&out)", asMETHODPR(sf::Packet, operator>>, (uint32_t&), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(int64&out)",  asMETHODPR(sf::Packet, operator>>, (int64_t&),  sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(uint64&out)", asMETHODPR(sf::Packet, operator>>, (uint64_t&), sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(float&out)",  asMETHODPR(sf::Packet, operator>>, (float&),    sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(double&out)", asMETHODPR(sf::Packet, operator>>, (double&),   sf::Packet&), asCALL_THISCALL));
	AS_ASSERT(eng->RegisterObjectMethod("Packet", "Packet& opShr(string&out)", asMETHODPR(sf::Packet, operator>>, (std::string&), sf::Packet&), asCALL_THISCALL));

	AS_ASSERT(eng->SetDefaultNamespace(""));
}
