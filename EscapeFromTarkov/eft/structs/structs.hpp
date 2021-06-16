#ifndef STRUCTS
#define STRUCTS
#pragma once

#include <cstdint>

#include "../player/player.hpp"

namespace structs {
	enum Bones : int
	{
		HumanBase = 0,
		HumanPelvis = 14,
		HumanLThigh1 = 15,
		HumanLThigh2 = 16,
		HumanLCalf = 17,
		HumanLFoot = 18,
		HumanLToe = 19,
		HumanRThigh1 = 20,
		HumanRThigh2 = 21,
		HumanRCalf = 22,
		HumanRFoot = 23,
		HumanRToe = 24,
		HumanSpine1 = 29,
		HumanSpine2 = 36,
		HumanSpine3 = 37,
		HumanLCollarbone = 89,
		HumanLUpperarm = 90,
		HumanLForearm1 = 91,
		HumanLForearm2 = 92,
		HumanLForearm3 = 93,
		HumanLPalm = 94,
		HumanRCollarbone = 110,
		HumanRUpperarm = 111,
		HumanRForearm1 = 112,
		HumanRForearm2 = 113,
		HumanRForearm3 = 114,
		HumanRPalm = 115,
		HumanNeck = 132,
		HumanHead = 133
	};

	struct BaseObject
	{
		uint64_t previousObjectLink; //0x0000
		uint64_t nextObjectLink; //0x0008
		uint64_t object; //0x0010
	};
	struct GameObjectManager
	{
		uint64_t lastTaggedObject; //0x0000
		uint64_t taggedObjects; //0x0008
		uint64_t lastActiveObject; //0x0010
		uint64_t activeObjects; //0x0018
	}; //Size: 0x0010

	//struct
	//{
	//	static constexpr uint64_t itemList = 0x60; //LootList
	//	static constexpr uint64_t registeredPlayers = 0x80;
	//	static constexpr uint64_t m_LocalPlayerID = 0x38;//CurrentProfileId
	//} local_game_world_off;


	//struct
	//{
	//	static constexpr uint64_t movementContext = 0x38;//gclass985_0 (type: GClass985)
	//	static constexpr uint64_t proceduralWeaponAnimation = 0x168; //proceduralWeaponAnimation_0
	//	static constexpr uint64_t playerBody = 0xA0; //_playerBody
	//	static constexpr uint64_t m_pHealthController = 0x400; //_healthController
	//	static constexpr uint64_t profile = 0x3D0; //profile_0 (type: EFT.Profile) 

	//} baseplayer;

	struct
	{
		static constexpr uint64_t length = 0x10; 
		static constexpr uint64_t string = 0x14;
	} unicode_string;

	class ListInternal
	{
	public:
		char pad_0x0000[0x20]; //0x0000
		uintptr_t* firstEntry; //0x0020 
	}; //Size=0x0028

	class List
	{
	public:
		char pad_0x0000[0x10]; //0x0000
		ListInternal* listBase; //0x0010 
		__int32 itemCount; //0x0018 
	}; //Size=0x001C
}
#endif