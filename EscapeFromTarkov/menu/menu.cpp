#include "menu.hpp"

namespace DisplayInformation
{
	float FPSLock = 144.f;
	bool login = true;
	bool bMenu = false;
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool WindowStatus = true;
	Vector2 Resolution;

	Vector2 GetDisplayResolution()
	{
		RECT Desktop;
		GetWindowRect(GetDesktopWindow(), &Desktop);
		Resolution.x = Desktop.right;
		Resolution.y = Desktop.bottom;
		return Resolution;
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static ID3D11Device* g_pd3dDevice;
static ID3D11DeviceContext* g_pd3dDeviceContext;
static IDXGISwapChain* g_pSwapChain;
static ID3D11RenderTargetView* g_mainRenderTargetView;

void CreateRenderTarget()
{
	DXGI_SWAP_CHAIN_DESC sd;
	g_pSwapChain->GetDesc(&sd);

	ID3D11Texture2D* pBackBuffer;
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
	ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
	render_target_view_desc.Format = sd.BufferDesc.Format;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &g_mainRenderTargetView);
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void ChangeClickAbility(bool canclick, HWND CHWND)
{
	long style = GetWindowLong(CHWND, GWL_EXSTYLE);
	if (canclick)
	{
		style &= ~WS_EX_LAYERED;
		SetWindowLong(CHWND, GWL_EXSTYLE, style);
		SetForegroundWindow(CHWND);
	}
	else
	{
		style |= WS_EX_LAYERED;
		SetWindowLong(CHWND, GWL_EXSTYLE, style);
	}
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	UINT createDeviceFlags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 1, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return E_FAIL;

	CreateRenderTarget();

	return S_OK;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX11_InvalidateDeviceObjects();
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
static float AVAL = 0;

void menu::build_menu()
{
	auto tab = [&](const char* name, int index, int& selected_tab, ImVec2 size) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.65f, 0.9f));
		if (selected_tab == index)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.95f));
		if (ImGui::Button(name, size)) {
			if (selected_tab == index)
				ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			selected_tab = index;
			return;
		}
		if (selected_tab == index)
			ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	};

	auto Combo = [](const char* name, const char** items, int itemcnt, int& index) {
		ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, ImGui::GetStyle().ChildRounding);

		if (ImGui::BeginCombo(name, items[index], 0)) {
			bool value_changed = false;
			for (int i = 0; i < itemcnt; i++) {
				ImGui::PushID((void*)(intptr_t)i);
				const char* item_text = items[i];
				if (ImGui::Selectable(item_text, i == index)) {
					index = true;
					index = i;
				}
				if (i == index)
					ImGui::SetItemDefaultFocus();
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	};

	auto aimbot = [&]() {
		ImGui::Spacing();

		ImGui::BeginChild(xorget("AimbotChild"), ImVec2(250.f, 144.f), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Checkbox(xorget("Enabled##aimbot"), &settings.aimbot);
			//ImGui::Checkbox(xorget("Ricochet##aimbot"), &settings.ricochet);
			ImGui::Hotkey(xorget("Aimbot Key"), &settings.aimbot_key);

			ImGui::SliderFloat(xorget("##aimbotFovSlider"),
				&settings.aimbot_max_fov,
				1.f,
				100.f,
				xorget("%.0f"));

			const char head_buffer[32] = { 0 };
			const char chest_buffer[32] = { 0 };
			const char body_buffer[32] = { 0 };
			memcpy((void*)head_buffer, xorget("Aim at Head"), sizeof("Aim at Head"));
			memcpy((void*)chest_buffer, xorget("Aim at Thorax"), sizeof("Aim at Thorax"));
			memcpy((void*)body_buffer, xorget("Aim at Body"), sizeof("Aim at Body"));

			volatile const char* aim_items[] = { head_buffer, chest_buffer, body_buffer };
			Combo(xorget("##aimbotAimSpotCombo"),
				(const char**)aim_items,
				IM_ARRAYSIZE(aim_items),
				settings.aimbot_aimspot);

			ImGui::EndChild();
		}
	};

	auto visuals = [&]() {
		const auto distance_format = xorget("%.0fm");

		ImGui::Spacing();

		ImGui::BeginChild(xorget("VisualsChild"), ImVec2(250.f, -1.f), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Checkbox(xorget("Enabled##visuals"), &settings.visuals);

			ImGui::Checkbox(xorget("Players##visuals"), &settings.player_esp);
			ImGui::Checkbox(xorget("Scavs##visuals"), &settings.scav_esp);

			ImGui::Separator();

			ImGui::Checkbox(xorget("Crosshair##visuals"), &settings.crosshair);

			ImGui::EndChild();
		}
		ImGui::SameLine();

		ImGui::BeginChild(xorget("ESPChild"), ImVec2(250.f, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Checkbox(xorget("Name##players"), &settings.esp_players_show_name);
			ImGui::Checkbox(xorget("Distance##show"), &settings.esp_players_show_distance);
			//ImGui::Checkbox(xorget("Weapon##players"), &settings.esp_players_weapon);
			ImGui::Checkbox(xorget("Skeleton##players"), &settings.esp_skeleton);
			ImGui::Checkbox(xorget("Box##visuals"), &settings.esp_players_box);


			ImGui::EndChild();
		}
	};

	auto settings_tab = [&]() {
		ImGui::Spacing();

		static settings_struct::color_var_t* cur_color = &settings.m_player_color;
		static float* cur_dist = &settings.esp_players_distance;
		ImGui::BeginChild(xorget("ColorEntries"),
			ImVec2(250.f, -1.f),
			true,
			ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			static int selected_index = 0;
			int        cur_index = 0;

			auto color_entry = [&](const char* name, settings_struct::color_var_t& color, float& distance = AVAL) {

				if (cur_index == selected_index)
					ImGui::PushStyleColor(ImGuiCol_Header,
						ImVec4(0.4f, 0.77f, 0.4f, 0.35f));
				ImGui::TextColored(color.color.Value, "|");
				ImGui::SameLine();
				if (ImGui::Selectable(
					name, cur_index == selected_index, 0, ImVec2(0.f, 0.f))) {
					if (cur_index == selected_index)
						ImGui::PopStyleColor();

					cur_dist = &distance;
					cur_color = &color;
					selected_index = cur_index;
					cur_index++;
					return;
				}
				if (cur_index == selected_index)
					ImGui::PopStyleColor();
				cur_index++;
			};

			color_entry(xorget("Player"), settings.m_player_color, settings.esp_players_distance);
			color_entry(xorget("Scav"), settings.m_player_color, settings.esp_scav_distance);

			color_entry(xorget("Crosshair"), settings.m_color_crosshair);

			ImGui::EndChild();
		}

		ImGui::SameLine();

		ImGui::BeginChild(xorget("Settings"),
			ImVec2(250.f, 225.f),
			true,
			ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			constexpr auto flags =
				ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs |
				ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview |
				ImGuiColorEditFlags_PickerHueBar | 1 << 9 |
				ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoOptions |
				ImGuiColorEditFlags_NoPicker;

			auto backup_w = ImGui::GetCurrentWindow()->DC.ItemWidth;
			ImGui::GetCurrentWindow()->DC.ItemWidth = 195.f;
			ImGui::ColorPicker4(xorget("##MainPicker"), (float*)(cur_color), flags);
			ImGui::GetCurrentWindow()->DC.ItemWidth = backup_w;

			ImGui::SameLine();

			ImGui::ColorButton(xorget("##ShowClrBtn"),
				cur_color->color.Value,
				flags,
				ImVec2(49.f, 172.f));

			ImGui::Spacing();

			if (cur_dist) {
				ImGui::Text(xorget("Render distance"));
				ImGui::SliderFloat(
					xorget("##dist"), cur_dist, 1.f, 500.f, xorget("%.2f"));
			}

			ImGui::EndChild();
		}
	};

	auto misc = [&]() {
		ImGui::Spacing();
		ImGui::BeginChild(xorget("MiscChild"), ImVec2(250.f, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Checkbox(xorget("No Sway##no_sway"), &settings.no_sway);
			ImGui::Checkbox(xorget("No Recoil##no_recoil"), &settings.no_recoil);
			ImGui::Checkbox(xorget("No Spread##no_spread"), &settings.no_spread);
			ImGui::Checkbox(xorget("Instant Bullet##instant_bullet"), &settings.instant_bullet);
			ImGui::Checkbox(xorget("Set Full Auto##set_automatic"), &settings.set_automatic);

			ImGui::Checkbox(xorget("Firerate##fast_firerate"), &settings.fast_firerate);
			ImGui::SameLine();
			ImGui::SliderFloat(xorget("##fire_rate"),
				&settings.fire_rate,
				1.f,
				1000.f,
				xorget("Firerate: %.2f"));

			ImGui::Separator();
			//ImGui::Checkbox(xorget("No Fall Damage##nofall"), &settings.no_fall_damage);
			ImGui::Checkbox(xorget("Infinite Stamina##infinite_stamine"), &settings.infinite_stamine);
			ImGui::Checkbox(xorget("Slow Fall##slow_fall"), &settings.slow_fall);
			ImGui::Checkbox(xorget("Run While Damage##run_damaged"), &settings.run_damaged);
			ImGui::Separator();

			ImGui::Checkbox(xorget("Time##day_time_modifier"), &settings.day_time_modifier);
			ImGui::SameLine();
			ImGui::SliderFloat(xorget("##time_of_day"),
				&settings.day_time,
				0,
				24.f,
				xorget("Time: %.2f"));

			ImGui::EndChild();
		}

		ImGui::SameLine();

		ImGui::BeginChild(xorget("Misc2Child"), ImVec2(250.f, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Checkbox(xorget("Fly##fly"), &settings.fly);

			ImGui::SliderFloat(xorget("##Fly_speed"),
				&settings.fly_speed,
				0.01f,
				10.f,
				xorget("Fly speed: %.2f"));
			ImGui::Hotkey(xorget("Fly Key"), &settings.fly_key);

			ImGui::Separator();
			ImGui::Checkbox(xorget("Unlock all doors##unlock_all_doors"), &settings.unlock_all_doors);
			ImGui::Hotkey(xorget("Unlock key##unlock_all_doors"), &settings.unlock_all_doors_key);

			ImGui::Separator();

			ImGui::Checkbox(xorget("Speedhack##time"), &settings.timescale);

			ImGui::SliderFloat(xorget("##speed"),
				&settings.timescale_speed,
				0.01f,
				10.f,
				xorget("Speedhack Speed: %.2f"));
			ImGui::Hotkey(xorget("Speedhack Key"), &settings.timescale_key);

			ImGui::EndChild();
		}
	};
#define NO_CONFIG
	auto config_tab = [&]() {
		ImGui::Spacing();

#ifdef NO_CONFIG
		ImGui::PushFont(Renderer::GetInstance()->m_rowland_26);

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.f - ImGui::CalcTextSize(xorget("Coming Soon...")).x / 2.f);
		ImGui::TextColored(ImColor(255, 0, 0), "Coming Soon...");

		ImGui::PopFont();
#else
		static cfg::cfg_t* cfg_selected = nullptr;

		auto reload_cfgs = [&]() {
			cfg_selected = nullptr;
			*g_ctx.cfg_list() = cfg::get_cfgs_from_disk();
			if (!g_ctx.cfg_list()->empty())
				cfg_selected = &*g_ctx.cfg_list()->begin();
		};

		// First time query configs
		static bool init_queried_cfgs = false;
		if (!init_queried_cfgs) {
			reload_cfgs();
			init_queried_cfgs = true;
		}

		// Show all the configs available
		ImGui::BeginChild(xorget("ConfigSelectChild"),
			ImVec2(-1.f, 171.f),
			true,
			ImGuiWindowFlags_AlwaysUseWindowPadding);
		for (auto& cfg : *g_ctx.cfg_list()) {
			bool is_selected = (cfg_selected == &cfg);
			char cfg_name_buffer[40] = { 0 };

			cfg.get_name(cfg_name_buffer);
			memcpy(cfg_name_buffer + cfg.name_len, xorget(".conf"), 5);

			if (ImGui::Selectable(cfg_name_buffer, is_selected))
				cfg_selected = &cfg;

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndChild();

		static char new_configname_buffer[32] = { 0 };

		auto create_new_cfg = [&]() {
			// Make sure the cfg name isn't empty
			if (new_configname_buffer[0] != '\0') {
				// WARNING: Names COULD collide.
				cfg::cfg_t new_cfg(new_configname_buffer);

				// Get current settings
				nlohmann::json cur_cfg;
				settings.config_load(cur_cfg, true);

				cfg::save_to_disk(new_cfg, true, cur_cfg);

				reload_cfgs();

				g_ctx.cheat()->add_message(xorget("Created config"));
			}
		};

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
		if (ImGui::InputTextEx(xorget("##InputConfigName"),
			nullptr,
			new_configname_buffer,
			32,
			ImVec2(367.f, 0.f),
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_CharsNoBlank)) {
			create_new_cfg();
		}
		ImGui::SameLine();
		if (ImGui::Button(xorget("Create New"))) {
			create_new_cfg();
		}
		if (ImGui::Button(xorget("Delete"), ImVec2(100.f, 0.f))) {
			if (cfg_selected) {
				// First delete from disk.
				cfg_selected->delete_from_disk();

				// Find the current cfg's iterator and remove it from the list.
				for (auto it = g_ctx.cfg_list()->begin(); it != g_ctx.cfg_list()->end();
					it++) {
					if (&*it == cfg_selected) {
						g_ctx.cfg_list()->erase(it);
						cfg_selected = nullptr;
						g_ctx.cheat()->add_message(xorget("Deleted config"));
						break;
					}
				}

				if (!g_ctx.cfg_list()->empty()) {
					cfg_selected = &*g_ctx.cfg_list()->begin();
				}
			}
		}
		auto posY = ImGui::GetCursorPosY();
		ImGui::SameLine();

		ImGui::BeginChild(xorget("ConfigInfoChild"),
			ImVec2(-1.f, -1.f),
			true,
			ImGuiWindowFlags_AlwaysUseWindowPadding);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
		if (cfg_selected) {
			char name_buffer[RB_CONFIG_MAXLEN] = { 0 };
			char created_buffer[RB_CONFIG_MAXLEN] = { 0 };
			char modified_buffer[RB_CONFIG_MAXLEN] = { 0 };
			cfg_selected->get_name(name_buffer);
			cfg_selected->get_created_time(created_buffer);
			cfg_selected->get_modified_time(modified_buffer);

			ImGui::Text(xorget("Name: %s.%s"), name_buffer, xorget("conf"));
			ImGui::Text(xorget("Created: %s"), created_buffer);
			ImGui::Text(xorget("Modified: %s"), modified_buffer);
		}
		else {
			ImGui::Text(xorget("No Config Selected."));
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();

		ImGui::SetCursorPosY(posY);
		if (ImGui::Button(xorget("Save"), ImVec2(100.f, 0.f))) {
			if (cfg_selected) {
				// Get current cfg to save.
				nlohmann::json cur_cfg;
				settings.config_load(cur_cfg, true);

				cfg::save_to_disk(*cfg_selected, false, cur_cfg);

				// Reload to get relevant data.
				// Should just load the file itself or update in memory.
				reload_cfgs();

				g_ctx.cheat()->add_message(xorget("Saved config"));
			}
		}
		if (ImGui::Button(xorget("Load"), ImVec2(100.f, 0.f))) {
			if (cfg_selected) {
				// Load config settings into current ones.
				auto config_settings = cfg_selected->container.get_json();
				settings.config_load(config_settings, false);

				g_ctx.cheat()->add_message(xorget("Loaded config"));
			}
		}
		ImGui::PopStyleColor();
#endif
	};

	if (ImGui::Begin(xorget("CheatHWND"), NULL, ImVec2(500, 350), 0.80f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
		//ImGui::PushFont(Renderer::GetInstance()->m_rowland_26);

		//ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.f - ImGui::CalcTextSize(xorget("faggothaq.me")).x / 2.f);
		//ImGui::TextColored(rgba(230, 126, 34, 1.0), xorget("faggothaq.me"));

		//ImGui::PopFont();
		ImGui::GetStyle().WindowPadding = { 5.f, 5.f };

		if (ImGui::BeginChild(xorget("CheatHWNDChild"), ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding)) {

			static int selected_tab = 0;
			tab(xorget("Aimbot"), 0, selected_tab, ImVec2(88.f, 28.f));
			ImGui::SameLine();
			tab(xorget("Misc"), 1, selected_tab, ImVec2(88.f, 28.f));
			ImGui::SameLine();
			tab(xorget("Visuals"), 2, selected_tab, ImVec2(88.f, 28.f));
			ImGui::SameLine();
			tab(xorget("Settings"), 3, selected_tab, ImVec2(89.f, 28.f));
			ImGui::SameLine();
			tab(xorget("Config"), 4, selected_tab, ImVec2(89.f, 28.f));

			switch (selected_tab) {
			case 0:
				aimbot();
				break;
			case 1:
				misc();
				break;
			case 2:
				visuals();
				break;
			case 3:
				settings_tab();
				break;
			case 4:
				//config_tab();
				break;
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

}

void menu::fov_circle() {
	ImVec2 dp = ImGui::GetIO().DisplaySize;
	Renderer::GetInstance()->draw_circle(dp.x / 2.f,
		dp.y / 2.f,
		dp.x * (settings.aimbot_max_fov / 90.f) *
		0.5f,
		false,
		2.f,
		ImColor(240, 240, 240, 220),
		60);
}

void menu::Style() {
	ImGuiStyle& st = ImGui::GetStyle();
	st.FrameBorderSize = 1.0f;
	st.FramePadding = ImVec2(4.0f, 2.0f);
	st.ItemSpacing = ImVec2(8.0f, 2.0f);
	st.WindowBorderSize = 1.0f;
	st.WindowRounding = 1.0f;
	st.ChildRounding = 1.0f;
	st.AntiAliasedFill = true;
	st.AntiAliasedLines = true;
	st.WindowBorderSize = 0.0f;

	// Setup style
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.53f, 0.53f, 0.53f, 0.46f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.53f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
	colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
	colors[ImGuiCol_Header] = ImVec4(0.54f, 0.54f, 0.54f, 0.58f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.65f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.58f, 0.58f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.87f, 0.87f, 0.87f, 0.53f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
}

void menu::crosshair()
{
	float middle_x = ImGui::GetIO().DisplaySize.x / 2.f;
	float middle_y = ImGui::GetIO().DisplaySize.y / 2.f;
	float size = 9.f;

	Renderer::GetInstance()->draw_line(
		middle_x - size,
		middle_y,
		middle_x + size + 1.f,
		middle_y,
		3.f,
		ImColor(0.f, 0.f, 0.f, 0.333f * settings.m_color_crosshair.color.Value.w));

	Renderer::GetInstance()->draw_line(
		middle_x,
		middle_y - size,
		middle_x,
		middle_y + size + 1.f,
		3.f,
		ImColor(0.f, 0.f, 0.f, 0.333f * settings.m_color_crosshair.color.Value.w));

	Renderer::GetInstance()->draw_line(middle_x - size + 1.f,
		middle_y,
		middle_x + size,
		middle_y,
		1.f,
		settings.m_color_crosshair.color);

	Renderer::GetInstance()->draw_line(middle_x,
		middle_y - size + 1.f,
		middle_x,
		middle_y + size,
		1.f,
		settings.m_color_crosshair.color);
}
std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

void menu::render()
{
	Vector2 Resolution = DisplayInformation::GetDisplayResolution();
	WNDCLASSEX WindowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, "TestHWND", NULL };
	RegisterClassEx(&WindowClass);

	HWND MenuWindowHandle = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, _T(xorget("TestHWND")), _T(xorget("TestHWND")), WS_POPUP | WS_CHILD, 0, 0, DisplayInformation::GetDisplayResolution().x - 1, DisplayInformation::GetDisplayResolution().y - 1, NULL, NULL, 0, NULL);

	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(MenuWindowHandle, &margins);

	if (CreateDeviceD3D(MenuWindowHandle) < 0)
	{
		CleanupDeviceD3D();
		UnregisterClass(_T(xorget("TestHWND")), WindowClass.hInstance);
		return;
	}

	ImGui::CreateContext();
	ImGui_ImplDX11_Init(MenuWindowHandle, g_pd3dDevice, g_pd3dDeviceContext);
	ChangeClickAbility(true, MenuWindowHandle);

	Renderer::GetInstance()->Initialize();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* imFont = io.Fonts->AddFontDefault();
	Style();

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	ShowWindow(MenuWindowHandle, SW_SHOWNORMAL);//start esp window.

	eft eft_obj;

	while (msg.message != WM_QUIT) {
		SetWindowPos(MenuWindowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		ImGui_ImplDX11_NewFrame();

		if (GetAsyncKeyState(VK_INSERT) && has_intialized)
		{
			ShowWindow(MenuWindowHandle, SW_SHOWDEFAULT);
			UpdateWindow(MenuWindowHandle);
			DisplayInformation::bMenu = DisplayInformation::bMenu ? false : true;
			DisplayInformation::WindowStatus = DisplayInformation::WindowStatus ? false : true;
			Sleep(150);
		}

		if (DisplayInformation::bMenu && DisplayInformation::login && has_intialized) {
			build_menu();
			ChangeClickAbility(true, MenuWindowHandle);
		}
		else {
			ChangeClickAbility(false, MenuWindowHandle);
		}
		if (!DisplayInformation::WindowStatus) {
			ShowWindow(MenuWindowHandle, SWP_SHOWWINDOW);
		}

		Renderer::GetInstance()->BeginScene();
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, reinterpret_cast<const float*>(&DisplayInformation::clearColor));

		/*Draw Here*/
		if(!has_intialized) {
			if (eft_obj.init()) {
				has_intialized = true;
			}
			else
				Sleep(1000);
		}
		else {
			HWND game = 0;
			if(!game)
				game = FindWindow(0, xorget("EscapeFromTarkov"));
		
			auto end = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() >= 3) {
				begin = std::chrono::steady_clock::now();
				if (!FindWindow(0, xorget("EscapeFromTarkov")))
					has_intialized = false;
			}

			if (game == GetForegroundWindow()) {
				if (settings.crosshair)
					crosshair();

				if (settings.aimbot)
					fov_circle();

				eft_obj.main_thread(Resolution);
			}
		}

		Renderer::GetInstance()->DrawScene();

		Renderer::GetInstance()->EndScene();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		
		g_pSwapChain->Present(0, 0);
		HWND DESKTOP = GetForegroundWindow();
		HWND MOVERDESK = GetWindow(DESKTOP, GW_HWNDPREV);
		
		SetWindowPos(MenuWindowHandle, MOVERDESK, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);
		UpdateWindow(MenuWindowHandle);

		Sleep(1);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();

	UnregisterClass(xorget("TestHWND"), WindowClass.hInstance);
}

menu::menu() {
	render();
}