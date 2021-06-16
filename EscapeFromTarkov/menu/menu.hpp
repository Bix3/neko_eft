#ifndef menu_hpp
#define menu_hpp
#pragma once

#include "../renderer/renderer.hpp"
#include "../string.hpp"
#include "../utils/xorstr.hpp"
#include "../settings/settings.hpp"
#include "../eft/eft.hpp"

#include <imgui_impl_dx11.h>
#include <imgui_tabs.h>
#include <imgui.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include <thread>
#include <tchar.h>

class menu {
private:
	bool has_intialized = false;

	void crosshair();
public:

	void build_menu();
	void fov_circle();
	void Style();
	void render();

	menu();
};
#endif