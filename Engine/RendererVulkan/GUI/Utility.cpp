#include "StdAfx.h"
#include "RendererVulkan/GUI/Utility.h"

#include "Reflection/Name.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"

#include "glm/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

namespace SG
{

#define ITEM_SPACING_Y 0.0f

#define RGB255(R, G, B) (R / 255.0f), (G / 255.0f), (B / 255.0f), 1.0f
	static void PushGrayButtonColorStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(RGB255(112, 112, 112)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(RGB255(161, 157, 157)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(RGB255(183, 183, 183)));
	}

	static void PushRedButtonColorStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(RGB255(190, 51, 51)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(RGB255(236, 73, 73)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(RGB255(241, 115, 115)));
	}

	static void PushGreenButtonColorStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(RGB255(51, 155, 69)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(RGB255(23, 194, 53)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(RGB255(80, 220, 104)));
	}

	static void PushBlueButtonColorStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(RGB255(38, 94, 174)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(RGB255(45, 123, 233)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(RGB255(104, 159, 238)));
	}
#undef RGB255

	template <typename TComponent, typename TFunc>
	static void DrawComponent(Scene::Entity& entity, TFunc&& uiFunc)
	{
		if (entity.HasComponent<TComponent>())
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 2, 2 });
			ImGui::Separator();
			const eastl::string componentFullName = Refl::CT_TypeName<TComponent>();
			const Size colonPos = componentFullName.find_last_of(':') + 1;
			const eastl::string componentName = componentFullName.substr(colonPos, componentFullName.size() - colonPos);
			bool bOpened = ImGui::TreeNodeEx(componentName.c_str(), flags);
			ImGui::PopStyleVar();

			auto regionAvailX = ImGui::GetContentRegionAvail().x;
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			//ImGui::SameLine(regionAvailX - lineHeight * 0.5f);
			//if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			//{
			//	ImGui::OpenPopup("ComponentSetting");
			//}

			//bool bRemoveComponent = false;
			//if (ImGui::BeginPopup("ComponentSetting"))
			//{
			//	if (ImGui::MenuItem("Remove Component"))
			//		bRemoveComponent = true;
			//	ImGui::EndPopup();
			//}

			auto& component = entity.GetComponent<TComponent>();
			if (bOpened)
			{
				uiFunc(component);
				ImGui::TreePop();
			}

			//if (bRemoveComponent)
			//	entity.RemoveComponent<TComponent>();
		}
	}

	bool DrawEntityProperty(Scene::Entity& entity)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		auto& tag = entity.GetComponent<TagComponent>();

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, tag.name.c_str());
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			tag = buffer;

		ImGui::PushID(tag.name.c_str());
		DrawComponent<TransformComponent>(entity, [&tag](TransformComponent& comp)
			{
				tag.bDirty |= DrawGUIDragFloat3("Position", comp.position);
				tag.bDirty |= DrawGUIDragFloat3("Rotation", comp.rotation, Vector3f(0.0f), 2.0f);
				tag.bDirty |= DrawGUIDragFloat3("Scale", comp.scale, Vector3f(1.0f));
			});
		DrawComponent<MaterialComponent>(entity, [&tag](MaterialComponent& comp)
			{
				tag.bDirty |= DrawGUIColorEdit3("Color", comp.albedo);
				tag.bDirty |= DrawGUIDragFloat("Metallic", comp.metallic, 0.35f, 0.05f, 0.0f, 1.0f);
				tag.bDirty |= DrawGUIDragFloat("Roughness", comp.roughness, 0.75f, 0.05f, 0.0f, 1.0f);
			});
		DrawComponent<MeshComponent>(entity, [&tag](MeshComponent& comp)
			{
				ImGui::Text("MeshType:   %s", comp.meshType == EMeshType::eOBJ ? ".obj" : "Unknown");
				ImGui::Text("ObjectId:   %d", comp.objectId);
				ImGui::Text("MeshId:     %d", comp.meshId);
				ImGui::Text("InstanceId: %d", comp.instanceId);
			});
		DrawComponent<PointLightComponent>(entity, [&entity, &tag](PointLightComponent& comp)
			{
				tag.bDirty |= DrawGUIColorEdit3("Color", comp.color);
				tag.bDirty |= DrawGUIDragFloat("Radius", comp.radius);
			});
		DrawComponent<DirectionalLightComponent>(entity, [&entity, &tag](DirectionalLightComponent& comp)
			{
				tag.bDirty |= DrawGUIColorEdit3("Color", comp.color);
			});
		ImGui::PopID();

		return false;
	}

	bool DrawGUIDragFloat(const string& label, float& values, float defaultValue, float speed,
		float minValue, float maxValue, float columnWidth)
	{
		ImGuiIO io = ImGui::GetIO();
		auto* pBoldFont = io.Fonts->Fonts[(UInt32)EGUIFontWeight::eBold];
		bool bDirty = false;

		// unique ID
		ImGui::PushID(label.c_str());

		// first column for label
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());

		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, ITEM_SPACING_Y });
		PushGrayButtonColorStyle();

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushFont(pBoldFont);

		if (ImGui::Button("D", buttonSize))
		{
			values = defaultValue;
			bDirty |= true;
		}
		ImGui::SameLine();
		bDirty |= ImGui::DragFloat("##X", &values, speed, minValue, maxValue, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopFont();

		ImGui::PopItemWidth();
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
		return bDirty;
	}

	bool DrawGUIDragFloat3(const string& label, Vector3f& values, const Vector3f& defaultValue, float speed,
		const Vector3f& minValues, const Vector3f& maxValues, float columnWidth)
	{
		ImGuiIO io = ImGui::GetIO();
		auto* pBoldFont = io.Fonts->Fonts[(UInt32)EGUIFontWeight::eBold];
		bool bDirty = false;

		// unique ID
		ImGui::PushID(label.c_str());

		// first column for label
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, ITEM_SPACING_Y });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		// Button X begin
		PushRedButtonColorStyle();

		ImGui::PushFont(pBoldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = defaultValue.x;
			bDirty |= true;
		}
		ImGui::SameLine();
		bDirty |= ImGui::DragFloat("##X", &values.x, speed, minValues.x, maxValues.x, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PopFont();

		ImGui::PopStyleColor(3);
		// Button X end

		// Button Y begin
		PushGreenButtonColorStyle();

		ImGui::PushFont(pBoldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = defaultValue.y;
			bDirty |= true;
		}
		ImGui::SameLine();
		bDirty |= ImGui::DragFloat("##Y", &values.y, speed, minValues.y, maxValues.y, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PopFont();

		ImGui::PopStyleColor(3);
		// Button Y end

		// Button Z begin
		PushBlueButtonColorStyle();

		ImGui::PushFont(pBoldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = defaultValue.z;
			bDirty |= true;
		}
		ImGui::SameLine();
		bDirty |= ImGui::DragFloat("##Z", &values.z, speed, minValues.z, maxValues.z, "%.2f");
		ImGui::PopItemWidth();
		ImGui::PopFont();

		ImGui::PopStyleColor(3);
		// Button Z end

		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
		return bDirty;
	}

	bool DrawGUIColorEdit3(const string& label, Vector3f& values, const Vector3f& defaultValue, float columnWidth)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiIO& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		auto* pBoldFont = io.Fonts->Fonts[(UInt32)EGUIFontWeight::eBold];
		bool bDirty = false;

		// unique ID
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);

		bDirty |= DrawGUIDragFloat3(label, values, defaultValue, 0.01f, Vector3f(0.0f), Vector3f(1.0f));

		ImGui::NextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth(1));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, ITEM_SPACING_Y });
		ImGui::PushFont(pBoldFont);

		/// begin custom color button
		ImVec4 v4 = { values.x, values.y, values.z, 1.0f };
		ImGui::SameLine();
		if (ImGui::ColorButton("##ColorButton", v4))
		{
			GImGui->ColorPickerRef = v4;
			ImGui::OpenPopup("picker");
			ImGui::SetNextWindowPos(GImGui->LastItemData.Rect.GetBL() + ImVec2(0.0f, style.ItemSpacing.y));
		}

		ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

		ImGuiWindow* picker_active_window = NULL;
		const float square_sz = ImGui::GetFrameHeight();
		const char* label_display_end = ImGui::FindRenderedTextEnd("##Color");
		GImGui->NextItemData.ClearFlags();

		const ImVec2 pos = window->DC.CursorPos;

		if (ImGui::BeginPopup("picker"))
		{
			picker_active_window = GImGui->CurrentWindow;
			ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
			ImGuiColorEditFlags picker_flags = (picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
			ImGui::SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
			bDirty |= ImGui::ColorPicker4("##picker", glm::value_ptr(values), picker_flags, &GImGui->ColorPickerRef.x);
			ImGui::EndPopup();
		}
		/// end custom color button

		ImGui::PopItemWidth();
		ImGui::PopFont();

		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
		return bDirty;
	}

}