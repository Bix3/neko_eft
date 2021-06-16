#ifndef PLAYER_HPP
#define PLAYER_HPP
#pragma once

#include "../../utils/vectors/vectors.hpp"

#include "../entity/entity.hpp"
#include "../../utils/crypto.hpp"

class baseplayer : public entity {
private:
	uintptr_t _values;
public:
	game_object* get_game_object()
	{
		if (!this)
			return nullptr;

		const auto unk0 = OMG::KERNAL::read<uintptr_t>((uintptr_t)this + 0x10);
		if (!unk0)
			return nullptr;

		const auto unk1 = OMG::KERNAL::read<game_object*>(unk0 + 0x30);
		if (!unk1)
			return nullptr;

		return unk1;
	}

	Vector3 head_pos;
	bool is_local = false;
	std::string player_name;

	bool is_scav()
	{
		uintptr_t profile = OMG::KERNAL::read<uintptr_t>(entity + 0x410); // (type: EFT.Profile)
		if (!profile)
			return false;

		uintptr_t account_id = OMG::KERNAL::read<uintptr_t>(profile + 0x18);//AccountId (type: System.String) from EFT.Profile
		if (!account_id)
			return false;

		auto acc_id_strlen = OMG::KERNAL::read<uint32_t>(account_id + 0x10);
		return acc_id_strlen < 2 || acc_id_strlen > 32;
	}

	uintptr_t get_bone_transform(int bone) {
		uintptr_t bone_info = OMG::KERNAL::read<uintptr_t>(OMG::KERNAL::read<uintptr_t>(OMG::KERNAL::read<uintptr_t>(OMG::KERNAL::read<uintptr_t>(entity + 0xa0) + 0x28) + 0x28) + 0x10);

		uintptr_t transform = OMG::KERNAL::read<uintptr_t>(OMG::KERNAL::read<uintptr_t>(bone_info + 0x20 + (bone * 0x8)) + 0x10);
		return transform;
	}

	std::string get_player_name() {
		uintptr_t profile = OMG::KERNAL::read<uintptr_t>(entity + 0x410);
		if (!profile)
			return {};

		uintptr_t info = OMG::KERNAL::read<uintptr_t>(profile + 0x28);//Info (type: GClass1042) //GClass doesn't appear unless in Emu.
		if (!info)
			return {};

		uintptr_t name = OMG::KERNAL::read<uintptr_t>(info + 0x10);//Nickname (type: System.String) //15 matches, be more specific.
		if (!name)
			return {};

		//crashes
		return OMG::KERNAL::read_unicode(name);
	}

	Vector3 get_fireport_pos() // maybe outdated
	{
		auto procedural_weapon_animation = OMG::KERNAL::read<uintptr_t>(entity + 0x70);//_elbowBends (type: UnityEngine.Transform[])
		if (!procedural_weapon_animation)
			return {};

		const auto firearm_controller = OMG::KERNAL::read<uintptr_t>(procedural_weapon_animation + 0x80); // firearmController_0 (type: EFT.Player.FirearmController) // Won't appear unless EmuTarkov
		if (!firearm_controller)
			return {};


		uint32_t Fireport = 0xe8;//Fireport (type: EFT.BifacialTransform)
		const auto bf_fireport = OMG::KERNAL::read<uintptr_t>(firearm_controller + Fireport);
		if (!bf_fireport)
			return {};

		auto original = OMG::KERNAL::read<uintptr_t>(bf_fireport + 0x10);
		if (!original)
			return {};

		return GetBonePosition(transform);
	}

	Vector2 get_view_angles() {
		auto movement_ctx = OMG::KERNAL::read<uintptr_t>(entity + 0x40 ); // fixed
		if (!movement_ctx)
			return {};

		return OMG::KERNAL::read<Vector2>(movement_ctx + 0x200 ); //vector2_2 (type: UnityEngine.Vector2)
	}

	void set_view_angles(Vector3 angles) {
		// nan check
		if (angles.x != angles.x || angles.y != angles.y || angles.z != angles.z)
			return;

		if (!angles.x || !angles.y || !angles.z)
			return;

		auto movement_ctx = OMG::KERNAL::read<uintptr_t>(entity + 0x40 ); // maybe
																				   // outdated
		if (!movement_ctx) return;

		OMG::KERNAL::write<Vector2>(movement_ctx + 0x200, Vector2(angles.x, angles.y)); // updated from UC
	}

	//void remove_sway()
	//{
	//	auto procedural_weapon_animation = OMG::KERNAL::read<uintptr_t>(entity + 0x190);
	//	if (!procedural_weapon_animation)
	//		return;
	//	OMG::KERNAL::write<int32_t>(procedural_weapon_animation + 0xf0, 1);//Mask (type: EFT.Animations.EProceduralAnimationMask)

	//	const auto breath_effector = OMG::KERNAL::read<uintptr_t>(procedural_weapon_animation + 0x28);//Breath (type: EFT.Animations.BreathEffector)
	//	if (!breath_effector)
	//		return;

	//	OMG::KERNAL::write<float>(breath_effector + 0xA4, 0.f);//Intensity (type: System.Single)
	//	OMG::KERNAL::write<float>(breath_effector + 0xB4, 0.f);//HipPenalty (type: System.Single)
	//}

	//void set_weapon_automatic() {
	//	auto procedural_weapon_animation = OMG::KERNAL::read<uintptr_t>(entity + 0x190);
	//	if (!procedural_weapon_animation)
	//		return;

	//	auto firearm_controller = OMG::KERNAL::read<uintptr_t>(procedural_weapon_animation + 0x80);//firearmController_0 (type: EFT.Player.FirearmController) /*EFT.Player+FirearmController*/
	//	if (!firearm_controller)
	//		return;

	//	auto item = OMG::KERNAL::read<uintptr_t>(firearm_controller + 0x50);//EFT.InventoryLogic.Weapon
	//	if (!item)
	//		return;

	//	auto fireMode = OMG::KERNAL::read<uintptr_t>(item + 0x90);//EFT.InventoryLogic.FireModeComponent
	//	if (!fireMode)
	//		return;

	//	auto weapon_template = OMG::KERNAL::read<uintptr_t>(item + 0x40);
	//	if (!weapon_template)
	//		return;

	//	OMG::KERNAL::write<int>(fireMode + 0x28, 0);// 0 value for auto, 1 for single
	//	OMG::KERNAL::write<int>(weapon_template + 0x168, 0);// weapFireType (type: EFT.InventoryLogic.Weapon.EFireMode[])
	//}

	void remove_recoil()
	{
		auto procedural_weapon_animation = OMG::KERNAL::read<uintptr_t>(entity + 0x190);
		if (!procedural_weapon_animation)
			return;

		const auto breath = OMG::KERNAL::read<uintptr_t>( procedural_weapon_animation + 0x28 );

		OMG::KERNAL::write<float>( breath + 0xA4, 0.f );
		OMG::KERNAL::write<int>( procedural_weapon_animation + 0xF8, 1 ); // mask
	}

	//void set_max_damage() {
	//	auto hands_controller = OMG::KERNAL::read<uintptr_t>(entity + 0x458);//_handsController (type: EFT.Player.AbstractHandsController)
	//	if (!hands_controller)
	//		return;

	//	auto item_template = OMG::KERNAL::read<uintptr_t>(hands_controller + 0x50);
	//	if (!item_template)
	//		return;
	//	auto weapon_template = OMG::KERNAL::read<uintptr_t>(item_template + 0x40);
	//	if (!weapon_template)
	//		return;

	//	auto ammo_template = OMG::KERNAL::read_chain<uintptr_t>(weapon_template, { 0x98, 0x20, 0x38, 0x40 });

	//	if (ammo_template == 0)
	//		ammo_template = OMG::KERNAL::read<uintptr_t>(weapon_template + 0x158); // Fallback to the weapon template's default ammo template

	//	if (!ammo_template)
	//		return;

	//	OMG::KERNAL::write<int32_t>(ammo_template + 0x14c, 9999999);
	//	OMG::KERNAL::write<int>(ammo_template + 0x178, 100);
	//}

	//void set_time_scale(float speed, int key) {
	//	uintptr_t unityplayer_base = OMG::KERNAL::find(_("UnityPlayer.dll"));

	//	auto weirdmanager = OMG::KERNAL::read<uint64_t>(unityplayer_base + 0x156B440 + (7 * 8));
	//	auto timeScale = OMG::KERNAL::read<float>(weirdmanager + 0xFC);

	//	if (GetAsyncKeyState(key))
	//		OMG::KERNAL::write<float>(weirdmanager + 0xFC, speed);
	//	else
	//		OMG::KERNAL::write<float>(weirdmanager + 0xFC, 1.f);
	//}

	//void instant_bullet() {
	//	auto hands_controller = OMG::KERNAL::read<uintptr_t>(entity + 0x458);//_handsController (type: EFT.Player.AbstractHandsController)
	//	if (!hands_controller)
	//		return;

	//	auto item_template = OMG::KERNAL::read<uintptr_t>(hands_controller + 0x50);
	//	if (!item_template)
	//		return;

	//	auto weapon_template = OMG::KERNAL::read<uintptr_t>(item_template + 0x40);
	//	if (!weapon_template)
	//		return;

	//	auto ammo_template = OMG::KERNAL::read_chain<uintptr_t>(weapon_template, { 0x98, 0x20, 0x38, 0x40 });

	//	if (ammo_template == 0)
	//		ammo_template = OMG::KERNAL::read<uintptr_t>(weapon_template + 0x158); // Fallback to the weapon template's default ammo template

	//	if (!ammo_template)
	//		return;

	//	OMG::KERNAL::write<int>(ammo_template + 0x174, 100000);//InitialSpeed (type: System.Single)
	//	auto rando = OMG::KERNAL::read<int32_t>(ammo_template + 0x170);

	//	OMG::KERNAL::write<int>(weapon_template + 0x1a0, 100000);//1a0 : Velocity (type: System.Single)
	//	OMG::KERNAL::write<int32_t>(weapon_template + 0x1bc, 1);//1bc : bHearDist (type: System.Int32)
	//	OMG::KERNAL::write<float>(hands_controller + 0x154, 100000);
	//	OMG::KERNAL::write<int32_t>(hands_controller + 0x1b8, 100000);
	//}

	//void set_fast_fire_rate(float fire_rate) {
	//	auto hands_controller = OMG::KERNAL::read<uintptr_t>(entity + 0x458);//_handsController (type: EFT.Player.AbstractHandsController)
	//	if (!hands_controller)
	//		return;

	//	auto item_template = OMG::KERNAL::read<uintptr_t>(hands_controller + 0x50);
	//	if (!item_template)
	//		return;
	//	auto weapon_template = OMG::KERNAL::read<uintptr_t>(item_template + 0x40);
	//	if (!weapon_template)
	//		return;

	//	//auto ammo_template = i6::read_chain<uintptr_t>(weapon_template, { 0x98, 0x20, 0x38, 0x40 });

	//	//if (ammo_template == 0)
	//	//	ammo_template = i6::read<uintptr_t>(weapon_template + 0x158); // Fallback to the weapon template's default ammo template

	//	//if (!ammo_template)
	//	//	return;

	//	OMG::KERNAL::write<int32_t>(weapon_template + 0x1b4, fire_rate);
	//}

	void remove_spread()
	{
		auto procedural_weapon_animation = OMG::KERNAL::read<uintptr_t>(entity + 0x190);
		if (!procedural_weapon_animation)
			return;

		auto firearm_controller = OMG::KERNAL::read<uintptr_t>(procedural_weapon_animation + 0x80);//firearmController_0 (type: EFT.Player.FirearmController) /*EFT.Player+FirearmController*/
		if (!firearm_controller)
			return;

		uint32_t HipInaccuracy = 0x14c;//HipInaccuracy (type: System.Single)

		OMG::KERNAL::write<float>(firearm_controller + HipInaccuracy, 0.f);
		OMG::KERNAL::write<float>(firearm_controller + HipInaccuracy - 4, 0.f); // RandomSpread, name's fucked now though.

		auto hands_controller = OMG::KERNAL::read<uintptr_t>(entity + 0x418);//_handsController (type: EFT.Player.AbstractHandsController)
		if (!hands_controller)
			return;

		OMG::KERNAL::write<float>(hands_controller + 0x150, 0);
		OMG::KERNAL::write<float>(hands_controller + 0x148, 0);
	}


	bool is_local_player() {
		bool local = OMG::KERNAL::read<int>(entity + 0x18);
		return local;
	}

	uint64_t get_bone_matrix()
	{
		return OMG::KERNAL::read_chain<uintptr_t>(entity, { /*PlayerBody*/0xA8, /*SkeletonRootJoint*/0x28, /*_values*/0x28, 0x10 });
	}

	//uintptr_t get_worldinteractiveobject() // in EFT.PLAYER
	//{
	//	return OMG::KERNAL::read<uintptr_t>(entity + 0x280);//interactableObject_0 (type: EFT.Interactive.InteractableObject)
	//}

	//void open_door(uintptr_t interactableobject, int32_t   interaction_type)
	//{
	//	OMG::KERNAL::write<int8_t>(interactableobject + 0xf9, interaction_type);//_doorState (type: EFT.Interactive.EDoorState)
	//}

	class sprinting {
	public:
		//void set_stamina(float stamina)
		//{
		//	OMG::KERNAL::write<float>((uintptr_t)this + 0x48, stamina);
		//}
	};

	class physical {
	public:
		//void disable_fall_damage() {
		//	OMG::KERNAL::write<float>((uintptr_t)this + 0xA0, 0.0f); // FallDamageMultiplier (type: System.Single)
		//}

		//sprinting* get_sprinting()
		//{
		//	return OMG::KERNAL::read<sprinting*>((uintptr_t)this + 0x28);//Stamina (type: GClass334)
		//}
	};

	//physical* get_physical() {
	//	return OMG::KERNAL::read<physical*>(entity + 0x420);//Physical (type: GClass335)
	//}

	class movement_context {
	public:
		enum EPhysicalCondition {
			None = 0,
			OnPainkillers = 1,
			LeftLegDamaged = 2,
			RightLegDamaged = 4,
			BoggedDown = 8,
			LeftArmDamaged = 16,
			RightArmDamaged = 32,
			Tremor = 64,
			UsingMeds = 128,
			HealingLegs = 256
		};

		//void fix_physical_condition()
		//{
		//	auto physical_condition = OMG::KERNAL::read<EPhysicalCondition>((uintptr_t)this + 0x260);

		//	if (physical_condition & HealingLegs || physical_condition & UsingMeds)
		//		return;

		//	OMG::KERNAL::write<EPhysicalCondition>((uintptr_t)this + 0x260, None);//ephysicalCondition_0 (type: EFT.EPhysicalCondition)
		//}

		void fall_speed(float speed)
		{
			OMG::KERNAL::write<float>((uintptr_t)this + 0x2e8, speed);//freefalltime
		}
	};

	movement_context* get_movement()
	{
		return OMG::KERNAL::read<movement_context*>(entity + 0x40 );
	}
};

#endif