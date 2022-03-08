#include "StdAfx.h"
#include "RendererVulkan/GUI/TestGUILayer.h"

#include "imgui/imgui.h"

// TEMPORARY
#include "glm/gtc/type_ptr.hpp"

namespace SG
{

	void TestGUILayer::OnUpdate(float deltaTime)
	{
		bool bShowDemoWindow = true;
		ImGui::ShowDemoWindow(&bShowDemoWindow);

		ImGui::Begin("Light");
		SSystem()->GetMainScene()->TraversePointLight([](PointLight& pointLight)
			{
				Vector3f position = pointLight.GetPosition();
				float radius = pointLight.GetRadius();
				Vector3f color = pointLight.GetColor();
				ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f);
				ImGui::DragFloat("Radius", &radius, 0.1f);
				ImGui::ColorEdit3("Color", glm::value_ptr(color));

				pointLight.SetPosition(position);
				pointLight.SetRadius(radius);
				pointLight.SetColor(color);
			});
		ImGui::End();
	}

}