#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <DirectXMath.h>

#include "../utils/vectors/vectors.hpp"

class Renderer
{
public:
	void Initialize();
	void BeginScene();
	void DrawScene();
	void EndScene();

	static Renderer* GetInstance();

	ImFont* m_pFont;
	ImFont* m_rowland_26;

	Renderer();
	~Renderer();


	void draw_rectangle(float x, float y, float x2, float y2, float thickness, bool filled, ImColor color);
	void draw_line(float x, float y, float x1, float y1, float thickness, ImColor color);
	void draw_text(Vector3 pos, const char* text, ImColor color, bool centered, bool stroked, ImFont* font, ImColor outline_col = ImColor{ 0.f, 0.f, 0.f, 0.8f });
	void draw_circle(float x, float y, float radius, bool filled, float thickness, ImColor color, int segments);
private:
	static Renderer* m_pInstance;
};