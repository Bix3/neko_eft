#include <vector>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui {
	struct TabsDesc {
		__int32	lableCount;
		float lableWidth;
		__int32 currentidx;

	};

	struct Tabs {
		TabsDesc* tbd;
		ImGuiID ID;
		__int32 selectedIdx;
	};

	static   ImVector<Tabs*> CacheTabs;
	static   Tabs* CurTabs;
	inline void BeginTabs(const char* name, int lablesCount, float tabwidth = 0) {
		//Find exists Tabs
		Tabs* exsisttbs = NULL;
		ImGuiID id = ImHash(name, 0);
		for (int i = 0; i < CacheTabs.Size; i++) {
			if (CacheTabs[i]->ID == id) {
				exsisttbs = CacheTabs[i];
			}
		}

		if (exsisttbs == NULL) {
			Tabs* tbs = (Tabs*)ImGui::MemAlloc(sizeof(Tabs));
			tbs->selectedIdx = 0;
			tbs->ID = id;
			CacheTabs.insert(CacheTabs.begin(), tbs);
			CurTabs = tbs;
		}
		else
		{
			CurTabs = exsisttbs;
		}

		TabsDesc* tbd = (TabsDesc*)ImGui::MemAlloc(sizeof(TabsDesc));
		tbd->lableCount = lablesCount;
		tbd->currentidx = 0;
		ImVec2 windowSize = ImGui::GetWindowSize();
		tbd->lableWidth = windowSize.x / lablesCount;
		CurTabs->tbd = tbd;
	}

	inline void EndTabs() {
		MemFree(CurTabs->tbd);
		CurTabs = NULL;

	}

	inline bool AddTab(const char* label, const char* tooltip)
	{
		TabsDesc* tbs = CurTabs->tbd;
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 itemSpacing = style.ItemSpacing;
		ImVec4 color = style.Colors[ImGuiCol_Button];
		ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
		style.ItemSpacing.x = 0;

		if (tbs->currentidx > 0)
			ImGui::SameLine();

		// push the style
		if (tbs->currentidx == CurTabs->selectedIdx)
		{
			style.Colors[ImGuiCol_Button] = colorActive;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorActive;
		}
		else
		{
			style.Colors[ImGuiCol_Button] = color;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		}

		// Draw the button
		if (ImGui::Button(label, ImVec2(tbs->lableWidth, 0))) {
			CurTabs->selectedIdx = tbs->currentidx;
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%s", tooltip);
			ImGui::EndTooltip();
		}

		// Restore the style
		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		style.ItemSpacing = itemSpacing;

		tbs->currentidx++;
		return CurTabs->selectedIdx == tbs->currentidx;
	}
}