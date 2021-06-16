#pragma once

#include "renderer.hpp"

Renderer* Renderer::m_pInstance;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	ImGuiIO& io = ImGui::GetIO();

	m_pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 12.f);
	m_rowland_26 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuisl.ttf", 26.f);
}

void Renderer::BeginScene()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("BUFFERNAME", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiSetCond_Always);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
}

void Renderer::DrawScene()
{
	ImGuiIO& io = ImGui::GetIO();
}

void Renderer::EndScene()
{

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
}

void Renderer::draw_rectangle(float x, float y, float x2, float y2, float thickness, bool filled, ImColor color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (filled)
		window->DrawList->AddRectFilled({ x, y }, { x2, y2 }, color, 0.f, 15); // Round All
	else
		window->DrawList->AddRect({ x, y }, { x2, y2 }, color, 0.f, 15, thickness);
}

void Renderer::draw_line(float x, float y, float x1, float y1, float thickness, ImColor color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine({ x, y }, { x1, y1 }, color, thickness);
}

void Renderer::draw_text(Vector3 pos, const char* text, ImColor color, bool centered, bool stroked, ImFont* font, ImColor outline_col) {
	const auto old_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (centered) {
		const auto size = ImGui::CalcTextSize(text);
		pos.x -= size.x / 2.f;
		pos.y -= size.y / 2.f;
	}

	//if (font) ImGui::PushFont(font);

	if (stroked) {
		outline_col.Value.w *= color.Value.w;
		window->DrawList->AddText({ pos.x + 1, pos.y + 1 }, outline_col, text);
	}

	ImGui::GetStyle().Colors[ImGuiCol_Text] = color;
	window->DrawList->AddText({ pos.x, pos.y }, color, text);
	ImGui::GetStyle().Colors[ImGuiCol_Text] = old_color;

	//if (font) ImGui::PopFont();
}

void Renderer::draw_circle(float x, float y, float radius, bool filled, float thickness, ImColor color, int segments) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto segs = static_cast<int>(radius * (ImGui::GetIO().DisplaySize.y / 1920.f)); // Yeh the proper formula is pi * radius^2 but our pixel density isn't high enough to need that.
	if (filled)
		window->DrawList->AddCircleFilled({ x, y }, radius, color, segments != 0 ? segments : segs);
	else
		window->DrawList->AddCircle({ x, y }, radius, color, segments != 0 ? segments : segs, thickness);
}

Renderer* Renderer::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Renderer();

	return m_pInstance;
}