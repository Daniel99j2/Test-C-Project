//
// Created by dj on 13/07/2025.
//

#pragma once
#include "imgui.h"

struct WorldGameObjectsEditor {
	ImGuiTextFilter Filter;
	shared_ptr<GameObject> VisibleNode = NULL;

	void draw() {
		ImGui::Begin("World Editor");
		ImGui::SeparatorText("Game Objects");
		// Left side
		ImGui::BeginChild("LeftPanel", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
		ImGui::SeparatorText("Game Objects");

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
		ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
		if (ImGui::InputTextWithHint("##Filter", "incl,-excl", Filter.InputBuf, IM_ARRAYSIZE(Filter.InputBuf),
		                             ImGuiInputTextFlags_EscapeClearsAll))
			Filter.Build();
		ImGui::PopItemFlag();

		if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg)) {
			for (std::shared_ptr<GameObject> node: GameConstants::world.getObjects())
				if (Filter.PassFilter(node->type.c_str()))
					DrawTreeNode(node);
			ImGui::EndTable();
		}

		ImGui::EndChild();


		// Right side
		ImGui::SameLine();
		ImGui::BeginChild("RightPanel", ImVec2(0, 0), ImGuiChildFlags_Borders); // width = remaining space
		ImGui::SeparatorText("Demo Properties");

		if (shared_ptr<GameObject> node = VisibleNode) {
			ImGui::Checkbox("Static", &node->isStatic);
			ImGui::Checkbox("Pushable", &node->pushable);

			ImGui::Spacing();
			ImGui::SeparatorText("Dropdowns");
			static int dropdown1 = 0;
			ImGui::Combo("Type", &dropdown1, "None\0Light\0Solid\0Animated\0");

			static int subType = 0;
			if (dropdown1 == 1) {
				ImGui::Combo("Light Type", &subType, "Point\0Spot\0Directional\0");
			} else if (dropdown1 == 2) {
				ImGui::Combo("Material", &subType, "Wood\0Metal\0Stone\0");
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Toggles");
			static bool flagA = false, flagB = true, flagC = false;
			ImGui::Checkbox("Visible", &flagA);
			ImGui::Checkbox("Selectable", &flagB);
			ImGui::Checkbox("Highlight", &flagC);

			ImGui::Spacing();
			if (ImGui::CollapsingHeader("Model")) {
				const Model *model = node->model;
				if (ImGui::TreeNode("Meshes")) {
					int i = 0;
					for (const Mesh &mesh: model->meshes) {
						if (ImGui::TreeNode((void *) (intptr_t) i, "Mesh %d: %s", i, mesh.name.c_str())) {
							ImGui::Text("Vertices: %d", static_cast<int>(mesh.vertices.size()));
							ImGui::Text("Indices: %d", static_cast<int>(mesh.indices.size()));
							ImGui::Text("Material: %s", mesh.material->name.c_str());

							constexpr float fullSize = 256.0f;

							int buttonId;

							auto showPreview = [fullSize](const char* label, GLuint tex, bool& expanded, string name) {
								ImGui::Text("%s (%i):", label, tex);
								ImGui::SameLine();
								if (ImGui::ImageButton(
										("mat_preview_"+name).c_str(),        // string ID - unique per button!
										tex,              // your texture reference
										ImVec2(64, 64),   // size
										ImVec2(0, 0),         // uv0
										ImVec2(1, 1),         // uv1
										ImVec4(0, 0, 0, 0),   // background color
										ImVec4(1, 1, 1, 1)    // tint color
									)) {
									expanded = !expanded;
									}
								if (expanded) {
									ImGui::Text("Expanded:");
									ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(fullSize, fullSize));
								}
							};

							static std::unordered_map<std::string, bool> previewExpanded;
							std::string uid = mesh.name + std::to_string(i); // Unique per mesh

							showPreview("Base", mesh.material->base, previewExpanded[uid + "_base"], uid + "_base");
							showPreview("Metallic", mesh.material->metal, previewExpanded[uid + "_metal"], uid + "metal");
							showPreview("Roughness", mesh.material->rough, previewExpanded[uid + "_rough"], uid + "rough");
							showPreview("Emissive", mesh.material->emissive, previewExpanded[uid + "_emissive"], uid + "emissive");
							showPreview("Normal", mesh.material->normal, previewExpanded[uid + "_normal"], uid + "normal");

							ImGui::TreePop();
						}
						i++;
					}
					if (i == 0)
						ImGui::TextDisabled("No meshes");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Animations")) {
					for (int i = 0; i < model->animations.size(); ++i) {
						ImGui::BulletText("Animation %d", i);
					}
					if (model->animations.empty())
						ImGui::TextDisabled("No animations");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Collisions")) {
					ImGui::Text("Collision parts: %d", (int) model->collisions.size());
					ImGui::TreePop();
				}
			}
		}

		ImGui::EndChild();

		ImGui::End();
	}

	void DrawTreeNode(shared_ptr<GameObject> node) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID(node->id);
		ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
		tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		// Standard opening mode as we are likely to want to add selection afterwards
		tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
		tree_flags |= ImGuiTreeNodeFlags_SpanFullWidth; // Span full width for easier mouse reach
		tree_flags |= ImGuiTreeNodeFlags_DrawLinesToNodes; // Always draw hierarchy outlines
		bool selected = (node == VisibleNode);
		if (selected)
			tree_flags |= ImGuiTreeNodeFlags_Selected;
		if (ImGui::Selectable(node->type.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
			VisibleNode = node;
		ImGui::PopID();
	}
};
