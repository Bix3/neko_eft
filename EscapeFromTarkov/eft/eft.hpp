#ifndef EFT_HPP
#define EFT_HPP
#pragma once

#include <array>
#include <mutex>
#include <chrono>

#include "../utils/vectors/vectors.hpp"

#include "extracts/exctracts.hpp"
#include "gameworld/gameworld.hpp"

#include "../settings/settings.hpp"

#include "structs/structs.hpp"
#include "../utils/Logging/Logging.hpp"
#include "../renderer/renderer.hpp"

#include "../string.hpp"
#include <map>

class eft {
private:
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	float distance_cursor(Vector3 vec)
	{
		POINT p;
		if (GetCursorPos(&p))
		{
			float ydist = (vec.y - p.y);
			float xdist = (vec.x - p.x);
			float ret = sqrt(pow(ydist, 2) + pow(xdist, 2));
			return ret;
		}
	}

    uintptr_t eft_base;
    uintptr_t mono_module_base;
	uintptr_t unityplayer_base;
    uintptr_t m_gom;
    uintptr_t game_world;
    gameworld local_game_world;
    uintptr_t fps_camera;

	baseplayer local_player;

	std::mutex          entity_mutex;
	std::vector<baseplayer> players;
	std::vector<extraction> extracts;

	struct unity_string
	{
		char buffer[256];
	};

	uint64_t GetObjectFromList(uint64_t listPtr, uint64_t lastObjectPtr, const char* objectName)
	{
		using structs::BaseObject;
		char* name;
		uint64_t classNamePtr = 0x0;

		BaseObject activeObject = OMG::KERNAL::read<BaseObject>(listPtr);
		BaseObject lastObject = OMG::KERNAL::read<BaseObject>(lastObjectPtr);

		if (activeObject.object != 0x0)
		{
			while (activeObject.object != 0 && activeObject.object != lastObject.object)
			{
				classNamePtr = OMG::KERNAL::read<uint64_t>(activeObject.object + 0x60);
				name = OMG::KERNAL::read<unity_string>(classNamePtr + 0x0).buffer;
				if (strcmp(name, objectName) == 0)
				{
					return activeObject.object;
				}

				activeObject = OMG::KERNAL::read<BaseObject>(activeObject.nextObjectLink);
			}
		}
		if (lastObject.object != 0x0)
		{
			classNamePtr = OMG::KERNAL::read<uint64_t>(lastObject.object + 0x60);
			name = OMG::KERNAL::read<unity_string>(classNamePtr + 0x0).buffer;
			if (strcmp(name, objectName) == 0)
			{
				return lastObject.object;
			}
		}

		return uint64_t();
	}

	Vector3 calculate_angle(const Vector3& origin, const Vector3& dest)
	{
		Vector3 diff = origin - dest;
		Vector3 ret;

		float length = diff.Length();
		ret.y = asinf(diff.y / length);
		ret.x = -atan2f(diff.x, -diff.z);

		return ret * 57.29578f;
	}

	enum item_type_t : int {
		ITEM_TYPE_OTHER,
		ITEM_TYPE_ATTACHMENT,
		ITEM_TYPE_CLOTHING,
		ITEM_TYPE_WEAPON,
		ITEM_TYPE_FOOD,
		ITEM_TYPE_VALUABLE,
		ITEM_TYPE_KEYCARDS,
		ITEM_TYPE_BEST_VALUABLE,
		ITEM_TYPE_MEDICINE,
		ITEM_TYPE_CORPSE
	};

	int playercount;
	VMatrix matrix;
public:
	bool world_to_screen(Vector3 origin, Vector3& out)
	{
		const auto temp = matrix.transpose();

		auto translation_vector = Vector3{ temp[3][0], temp[3][1], temp[3][2] };
		auto up = Vector3{ temp[1][0], temp[1][1], temp[1][2] };
		auto right = Vector3{ temp[0][0], temp[0][1], temp[0][2] };

		float w = translation_vector.dot(origin) + temp[3][3];

		if (w < 0.098f)
			return false;

		float x = up.dot(origin) + temp._24;
		float y = right.dot(origin) + temp._14;

		out.x = (static_cast<float>(ImGui::GetIO().DisplaySize.x) / 2.f) * (1.f + y / w);
		out.y = (static_cast<float>(ImGui::GetIO().DisplaySize.y) / 2.f) * (1.f - x / w);
		out.z = w;

		return true;
	}

	bool init_game() {
		auto active_objects = OMG::KERNAL::read<std::array<uint64_t, 2>>(m_gom + offsetof(structs::GameObjectManager, lastActiveObject));
		Log::Value(xorget("Active Object 1 0x%X"), active_objects[0]);
		Log::Value(xorget("Active Object 2 0x%X"), active_objects[1]);
		if (!active_objects[0] || !active_objects[1])
			return false;

		game_world = GetObjectFromList(active_objects[1], active_objects[0], xorget("GameWorld"));
		Log::Value(xorget("GameWorld 0x%X"), game_world);
		if (!game_world)
			return false;

		local_game_world.entity = OMG::KERNAL::read_chain<uintptr_t>(game_world, { 0x30, 0x18, 0x28 });
		Log::Value(xorget("Local GameWorld 0x%X"), local_game_world.entity);
		if (!local_game_world.entity)
			return false;

		auto tagged_objects = OMG::KERNAL::read<std::array<uint64_t, 2>>(m_gom + offsetof(structs::GameObjectManager, lastTaggedObject));
		Log::Value(xorget("Tagged Object 1 0x%X"), tagged_objects[0]);
		Log::Value(xorget("Tagged Object 2 0x%X"), tagged_objects[1]);

		fps_camera = GetObjectFromList(tagged_objects[1], tagged_objects[0], xorget("FPS Camera"));
		Log::Value(xorget("FPS Camera 0x%X"), fps_camera);
		if (!fps_camera)
			return false;

		return true;
	}

	inline bool player_loop()
	{
		std::vector<baseplayer> local_players;
		std::vector<extraction> local_extracts;
		{

			uint64_t onlineusers = OMG::KERNAL::read<uint64_t>(local_game_world.entity + 0x80 );

			if (!onlineusers) {
				init_game();
				return false;
			}

			uint64_t list_base = OMG::KERNAL::read<uint64_t>(onlineusers + offsetof(structs::List, listBase));
			int32_t player_count = OMG::KERNAL::read<int32_t>(onlineusers + 0x18);
			if (player_count < 1)
			{
				init_game();
				return false;
			}

			Log::Value("Player Count %d", player_count);

			for (int i = 0; i < player_count; i++)
			{
				uintptr_t player_buffer = OMG::KERNAL::read<uintptr_t>(list_base + (offsetof(structs::ListInternal, firstEntry) + (i * 8)));

				baseplayer current_player;
				playercount = player_count;

				current_player.entity = player_buffer;
				//uint64_t bone_matrix = current_player.get_bone_matrix();

				if (current_player.is_local_player())
				{
					current_player.is_local = true;
					local_player = current_player;
				}

				/*if (bone_matrix)
				{
					uint64_t bone = ricochet::communication::read_chain<uint64_t>(bone_matrix, { 0x20, 0x10, 0x38 });
					current_player.location = ricochet::communication::read<Vector3>(bone + 0xB0);
				}*/

				current_player.player_name = current_player.is_scav() ? "Scav" : current_player.get_player_name();

				local_players.push_back(current_player);
			}
		}

		{
			uint64_t exit_controller = OMG::KERNAL::read<uint64_t>(local_game_world.entity + 0x18);/*gclass646_0 (type: GClass646)*/

			if (!exit_controller)
				return false;

			uint64_t exit_point = OMG::KERNAL::read<uint64_t>(exit_controller + 0x20);/*exfiltrationPoint_0 (type: EFT.Interactive.ExfiltrationPoint[])*/ //If not emuTarkov then wont appear
			int exit_count = OMG::KERNAL::read<int>(exit_point + 0x18);

			if (exit_count <= 0 || !exit_point)
				return false;

			for (int i = 0; i < exit_count; i++)
			{
				extraction extract;

				uint64_t extract_buffer = OMG::KERNAL::read<uint64_t>(exit_point + (offsetof(structs::ListInternal, firstEntry) + (i * 8)));

				extract.entity = extract_buffer;

				if (!extract.open_extract())
					continue;

				extract.transform = OMG::KERNAL::read_chain<uint64_t>(extract.entity, { 0x10, 0x30, 0x30, 0x8, 0x28 });
				extract.location = extract.GetBonePosition();

				uint64_t extract_name = OMG::KERNAL::read_chain<uint64_t>(extract.entity, { 0x58, 0x10 });

				if (extract_name)
				{
					extract.name = extract.read_unicode(extract_name);
				}

				local_extracts.push_back(extract);
			}
		}

		std::lock_guard<std::mutex> lk(entity_mutex);
		players = std::move(local_players);
		extracts = std::move(local_extracts);

		return true;
	}

	bool main_thread(Vector2 res) {
		/*ESP/Iterations*/
		if (!fps_camera || !local_game_world.entity || !game_world) {
			if (!init_game()) {
				Sleep(1000);
				return false;
			}
		}
		else {
			auto end = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() >= 20) {
				begin = std::chrono::steady_clock::now();
				player_loop();
			}
		}

		{
			std::lock_guard<std::mutex> lk(entity_mutex);

			baseplayer closestTarget, cached_target;
			float closestDist = FLT_MAX;

			{
				uint64_t temp = fps_camera;
				temp = OMG::KERNAL::read<uint64_t>(temp + 0x30);
				Log::Value("Temp 1 0x%X", temp);

				temp = OMG::KERNAL::read<uint64_t>(temp + 0x18);
				Log::Value("Temp 2 0x%X", temp);

				matrix = OMG::KERNAL::read<VMatrix>(temp + 0xD8);
				Log::Value("View Matrix 0x%X", OMG::KERNAL::read<uint64_t>(temp + 0xD8));
				printf(xorget("\n"));
			}

			for (auto player : players) {
				if (player.is_local)
					continue;

				player.transform = player.get_bone_transform(structs::Bones::HumanHead);
				player.head_pos = player.GetBonePosition();

				local_player.transform = local_player.get_bone_transform(structs::Bones::HumanHead);
				local_player.head_pos = local_player.GetBonePosition();

				auto dist = (int)(player.head_pos - local_player.head_pos).Length();

				auto is_scav = player.is_scav();

				bool is_dead = OMG::KERNAL::read<bool>(player.entity + 0x658);

				if (is_dead)
					continue;

				if ((!is_scav && dist > settings.esp_players_distance) ||
					(is_scav && dist > settings.esp_scav_distance))
					continue;

				if (is_scav && !settings.scav_esp)
					continue;

				if (!is_scav && !settings.player_esp)
					continue;

				auto clr = settings.m_player_color.get_color_bydist(
					dist, is_scav ? settings.esp_scav_distance : settings.esp_players_distance);

				struct bone_t {
					Vector3 screen;
					int        index;
					bool       on_screen;
				};

				static std::array<bone_t, 23> bones = {
					bone_t{ Vector3{}, 133, false }, bone_t{ Vector3{}, 132, false },
					bone_t{ Vector3{}, 37, false },  bone_t{ Vector3{}, 36, false },
					bone_t{ Vector3{}, 29, false },  bone_t{ Vector3{}, 14, false },
					bone_t{ Vector3{}, 19, false },  bone_t{ Vector3{}, 18, false },
					bone_t{ Vector3{}, 17, false },  bone_t{ Vector3{}, 16, false },
					bone_t{ Vector3{}, 15, false },  bone_t{ Vector3{}, 14, false },
					bone_t{ Vector3{}, 20, false },  bone_t{ Vector3{}, 21, false },
					bone_t{ Vector3{}, 22, false },  bone_t{ Vector3{}, 23, false },
					bone_t{ Vector3{}, 24, false },  bone_t{ Vector3{}, 94, false },
					bone_t{ Vector3{}, 91, false },  bone_t{ Vector3{}, 90, false },
					bone_t{ Vector3{}, 111, false }, bone_t{ Vector3{}, 112, false },
					bone_t{ Vector3{}, 115, false }
				};

				struct bounds_t {
					float left, right, top, bottom;
				};

				const auto get_bounds = [&](bounds_t& out, float expand = 0.f) -> bool {
					bounds_t bounds = { FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN };

					auto screen_size = ImGui::GetIO().DisplaySize;

					for (auto& bone : bones) {
						player.transform = player.get_bone_transform(bone.index);
						auto bone_pos = player.GetBonePosition();
						if (world_to_screen(bone_pos, bone.screen)) {
							// w2s validity check
							if (bone.screen.x <= 0 || bone.screen.x >= screen_size.x ||
								bone.screen.y <= 0 || bone.screen.y >= screen_size.y)
								continue;
							if (bone.screen.x < bounds.left)
								bounds.left = bone.screen.x;
							else if (bone.screen.x > bounds.right)
								bounds.right = bone.screen.x;
							if (bone.screen.y < bounds.top)
								bounds.top = bone.screen.y;
							else if (bone.screen.y > bounds.bottom)
								bounds.bottom = bone.screen.y;
							bone.on_screen = true;
						}
					}

					if (bounds.left == FLT_MAX)
						return false;
					if (bounds.right == FLT_MIN)
						return false;
					if (bounds.top == FLT_MAX)
						return false;
					if (bounds.bottom == FLT_MIN)
						return false;

					bounds.left -= expand;
					bounds.right += expand;
					bounds.top -= expand;
					bounds.bottom += expand;

					out = bounds;

					return true;
				};

				const auto draw_bone_to_bone = [&](uint32_t idx_a, uint32_t idx_b) {
					auto& bone_a = bones[idx_a];
					auto& bone_b = bones[idx_b];

					if (!bone_a.on_screen || !bone_b.on_screen)
						return;

					auto screen_pos_a = bone_a.screen;
					auto screen_pos_b = bone_b.screen;

					// Draw shadow
					Renderer::GetInstance()->draw_line(screen_pos_a.x + 1.f,
						screen_pos_a.y + 1.f,
						screen_pos_b.x + 1.f,
						screen_pos_b.y + 1.f,
						1.f,
						ImColor{ 0.f, 0.f, 0.f, 0.7f * clr.Value.w });

					// Then the actual skeleton
					Renderer::GetInstance()->draw_line(
						screen_pos_a.x, screen_pos_a.y, screen_pos_b.x, screen_pos_b.y, 1.f, clr);
				};

				Vector3 screen;
				if (world_to_screen(player.head_pos, screen)) {
					bounds_t bounds;
					if (player.head_pos.z == 0)
						continue;

					if (get_bounds(bounds, 4.f)) {

						auto fov = distance_cursor(screen);
						int MaxAimFOV = settings.aimbot_max_fov * 10;
						double this_dist = sqrt(pow((res.x / 2) - screen.x, 2) + pow((res.y / 2) - screen.y, 2));
						if (fov < MaxAimFOV) {
							if (this_dist < closestDist)
							{
								closestDist = this_dist;
								closestTarget = player;
							}
						}


						if (settings.esp_players_box) {
							/*Renderer::GetInstance()->draw_rectangle(bounds.left,
								bounds.top,
								bounds.right,
								bounds.bottom,
								4.f,
								false,
								ImColor{ 0, 0, 0, (int)(255 * clr.Value.w) });

							Renderer::GetInstance()->draw_rectangle(bounds.left,
								bounds.top,
								bounds.right,
								bounds.bottom,
								2.f,
								false,
								clr);*/
							float X = bounds.left, Y = bounds.top, W = bounds.right - bounds.left, H = bounds.bottom - bounds.top;
							float lineW = (W / 5);
							float lineH = (H / 6);
							float lineT = 1;
							Renderer::GetInstance()->draw_line(X, Y, X, Y + lineH, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X, Y, X + lineW, Y, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X + W - lineW, Y, X + W, Y, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X + W, Y, X + W, Y + lineH, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X, Y + H - lineH, X, Y + H, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X, Y + H, X + lineW, Y + H, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X + W - lineW, Y + H, X + W, Y + H, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));
							Renderer::GetInstance()->draw_line(X + W, Y + H - lineH, X + W, Y + H, 4, ImColor(0.f, 0.f, 0.f, 1.f * clr.Value.w));

							Renderer::GetInstance()->draw_line(X, Y, X, Y + lineH, 2, clr);
							Renderer::GetInstance()->draw_line(X, Y, X + lineW, Y, 2, clr);
							Renderer::GetInstance()->draw_line(X + W - lineW, Y, X + W, Y, 2, clr);
							Renderer::GetInstance()->draw_line(X + W, Y, X + W, Y + lineH, 2, clr);
							Renderer::GetInstance()->draw_line(X, Y + H - lineH, X, Y + H, 2, clr);
							Renderer::GetInstance()->draw_line(X, Y + H, X + lineW, Y + H, 2, clr);
							Renderer::GetInstance()->draw_line(X + W - lineW, Y + H, X + W, Y + H, 2, clr);
							Renderer::GetInstance()->draw_line(X + W, Y + H - lineH, X + W, Y + H, 2, clr);
						}

						const auto info_x_center = bounds.left + (bounds.right - bounds.left) / 2.f;

						// start info y value here, increment when info is added
						constexpr auto text_y = 10.f;
						auto           info_y = bounds.bottom + 1.f;

						info_y += text_y / 2.f;

						if (settings.esp_players_show_name) {
							const auto _name = player.player_name;
							if (!_name.empty()) {
								Renderer::GetInstance()->draw_text({ info_x_center, info_y, 0 },
									_name.c_str(),
									clr,
									true,
									true,
									Renderer::GetInstance()->m_pFont);

								info_y += text_y;
							}
						}

						// player distance
						if (settings.esp_players_show_distance) {
							const auto dist_text = string::format(xorget("%dm"), dist);

							Renderer::GetInstance()->draw_text({ info_x_center, info_y, 0 },
								dist_text,
								clr,
								true,
								true,
								Renderer::GetInstance()->m_pFont);

							info_y += text_y;
						}

						// player distance end
						if (settings.esp_skeleton) {
							draw_bone_to_bone(0, 1);
							draw_bone_to_bone(1, 2);
							draw_bone_to_bone(2, 3);
							draw_bone_to_bone(3, 4);
							draw_bone_to_bone(4, 5);

							draw_bone_to_bone(6, 7);
							draw_bone_to_bone(7, 8);
							draw_bone_to_bone(8, 9);
							draw_bone_to_bone(9, 10);
							draw_bone_to_bone(10, 11);
							draw_bone_to_bone(11, 12);
							draw_bone_to_bone(12, 13);
							draw_bone_to_bone(13, 14);
							draw_bone_to_bone(14, 15);
							draw_bone_to_bone(15, 16);

							draw_bone_to_bone(17, 18);
							draw_bone_to_bone(18, 19);
							draw_bone_to_bone(19, 20);
							draw_bone_to_bone(20, 21);
							draw_bone_to_bone(21, 22);
						}
					}
				}

				Log::Value(xorget("Player Name %s"), player.player_name.c_str());
			}

			baseplayer null_target;
			if (closestTarget.entity && local_player.entity && settings.aimbot) {
				if (GetAsyncKeyState(settings.aimbot_key) & 0x8000) {
					Vector3 screen;
					if (world_to_screen(closestTarget.head_pos, screen)) {

						auto fov = distance_cursor(screen);
						int MaxAimFOV = settings.aimbot_max_fov * 10;
						if (fov < MaxAimFOV) {
							cached_target = closestTarget;
							Vector3 aimspot;
							switch (settings.aimbot_aimspot) {
							case 0:
								aimspot = cached_target.head_pos;
								break;
							case 1:
								cached_target.transform = cached_target.get_bone_transform(37);
								aimspot = cached_target.GetBonePosition();
								break;
							case 2:
								cached_target.transform = cached_target.get_bone_transform(20);
								aimspot = cached_target.GetBonePosition();
								break;
							}

							local_player.set_view_angles(calculate_angle(local_player.head_pos, aimspot));
						}
						else
							cached_target = null_target;
					}
				}
				else
					cached_target = null_target;
			}

			printf(xorget("\n"));
			for (auto extract : extracts) {

				Log::Value(xorget("Extract Name %s"), extract.name.c_str());
			}
		}

		/*Misc Functions*/
		if(local_player.entity)
		{
			//if (settings.fast_firerate)
			//	local_player.set_fast_fire_rate(settings.fire_rate);

			if (settings.day_time_modifier)
				local_game_world.set_time(settings.day_time);

			//if(settings.set_automatic)
			//	local_player.set_weapon_automatic();

			if (settings.no_recoil)
				local_player.remove_recoil();

			//if (settings.no_sway)
			//	local_player.remove_sway();

			if (settings.no_spread)
				local_player.remove_spread();

			//if (settings.instant_bullet)
			//	local_player.instant_bullet();

			//if (settings.timescale)
			//	local_player.set_time_scale(settings.timescale_speed, settings.timescale_key);

			//auto physical = [&]() {
			//	auto physical = local_player.get_physical();

			//	if (!physical)
			//		return;

			//	/*if (settings.no_fall_damage)
			//		physical->disable_fall_damage();*/

			//	auto sprinting = physical->get_sprinting();

			//	if (!sprinting)
			//		return;

			//	if (settings.infinite_stamine)
			//		sprinting->set_stamina(99.9f);
			//}; physical();

			enum EInteractionType : int32_t { Open, Close, Unlock, Breach, Lock };

			//auto worldinteractiveobject = local_player.get_worldinteractiveobject();
			//if (worldinteractiveobject) {
			//	if (GetAsyncKeyState(settings.unlock_all_doors_key))
			//	{
			//		local_player.open_door(worldinteractiveobject, Unlock);
			//	}
			//}

			auto movement = local_player.get_movement();
			if (movement) {
				if (settings.slow_fall) {
					movement->fall_speed(0.05f);
				}
				if (GetAsyncKeyState(settings.fly_key) && settings.fly) {
					movement->fall_speed(-(settings.fly_speed / 10.f));
				}

				//if (settings.run_damaged) {
				//	movement->fix_physical_condition();
				//}
			}
		}
		return true;
	}

	inline bool init() {
        pid = OMG::KERNAL::get_pid(xorget("EscapeFromTarkov.exe"));
        Log::Value(xorget("Process ID %d"), pid);
		if (!pid)
			return false;

	    //eft_base = ricochet::communication::get_process_base(pid);
		//Log::Value(xorget("EscapeFromTarkov.exe Base 0x%X"), eft_base);
		//if (!eft_base)
		//	return false;

        unityplayer_base = OMG::KERNAL::find(_("UnityPlayer.dll"));
        Log::Value(xorget("UnityPlayer.dll Base 0x%X"), unityplayer_base);
		if (!unityplayer_base)
			return false;

        mono_module_base = OMG::KERNAL::find(_("mono-2.0-bdwgc.dll"));
        Log::Value(xorget("mono-2.0-bdwgc.dll Base 0x%X"), mono_module_base);

        m_gom = OMG::KERNAL::read<uintptr_t>(unityplayer_base + 0x156C698 );
        Log::Value(xorget("GameObjectManager 0x%X"), m_gom);
		if (!m_gom)
			return false;
		return true;
    }
};
#endif