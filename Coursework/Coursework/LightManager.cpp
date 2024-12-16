#include "LightManager.h"
#include "Shaders/DefaultShader.h"

LightManager::LightManager(D3D* _renderer)
{
	lights[0].type = static_cast<int>(LightType::DIRECTIONAL);
	lights[0].direction = XMFLOAT4(-1.0f, 0.68f, 0.0f, 0.0f);
	lights[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
	lights[0].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	activeLights.push_back(&lights[0]);

	lights[1].type = static_cast<int>(LightType::POINT);
	lights[1].position = XMFLOAT4(15.0f, 10.0f, 12.5f, 0.0f);
	lights[1].diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	activeLights.push_back(&lights[1]);

	lights[2].type = static_cast<int>(LightType::POINT);
	lights[2].position = XMFLOAT4(2.5f, 5.0f, 5.0f, 0.0f);
	lights[2].diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	activeLights.push_back(&lights[2]);

	lights[3].type = static_cast<int>(LightType::POINT);
	lights[3].position = XMFLOAT4(5.0f, 5.0f, 2.5f, 0.0f);
	lights[3].diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	activeLights.push_back(&lights[3]);

	for (size_t light = 0; light < MAX_LIGHTS; light++)
	{		
		SphereMesh* cube = new SphereMesh(_renderer->getDevice(), _renderer->getDeviceContext(), 4);
		lightBoxes.push_back(cube);
	}
}

LightManager::~LightManager()
{
	for (size_t light = 0; light < lightBoxes.size(); light++)
	{
		delete lightBoxes[light];
		lightBoxes[light] = nullptr;
	}

	lightBoxes.clear();
}

void LightManager::render(ID3D11DeviceContext* _deviceContext, DefaultShader* _defaultShader, Matrices _matrices) const
{
	if (renderLightboxes)
	{
		_matrices.worldMatrix *= XMMatrixScaling(0.1f, 0.1f, 0.1f);

		for (size_t light = 0; light < activeLights.size(); light++)
		{		
			if (activeLights[light]->type != static_cast<int>(LightType::POINT) && activeLights[light]->type != static_cast<int>(LightType::SPOT)) continue;

			lightBoxes[light]->setPosition(activeLights[light]->position);
			_defaultShader->renderMesh(lightBoxes[light], _matrices, nullptr);
		}
	}
}

void LightManager::gui()
{
	if (ImGui::CollapsingHeader("Lights"))
	{
		ImGui::Checkbox("Render light boxes", &renderLightboxes);
		for (size_t light = 0; light < activeLights.size(); light++)
		{
			if (ImGui::TreeNode(reinterpret_cast<void*>(static_cast<intptr_t>(light)), "Light: %d", light))
			{
				ImGui::Combo("Light Type", &lights[light].type, lightType, 4);

				switch (lights[light].type)
				{
				case static_cast<int>(LightType::OFF):
					break;

				case static_cast<int>(LightType::DIRECTIONAL):
					ImGui::SliderFloat3("Direction", &lights[light].direction.x, -1, 1);
					ImGui::ColorEdit4("Ambient", &lights[light].ambient.x, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Diffuse", &lights[light].diffuse.x, ImGuiColorEditFlags_NoInputs);
					break;

				case static_cast<int>(LightType::POINT):
				{
					ImGui::SliderFloat3("Position", &lights[light].position.x, -5, 25);
					ImGui::ColorEdit4("Ambient", &lights[light].ambient.x, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Diffuse", &lights[light].diffuse.x, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Specular", &lights[light].specular.x, ImGuiColorEditFlags_NoInputs);
					break;
				}
				case static_cast<int>(LightType::SPOT):
					ImGui::SliderFloat3("Position", &lights[light].position.x, -5, 25);
					ImGui::ColorEdit4("Ambient", &lights[light].ambient.x, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Diffuse", &lights[light].diffuse.x, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Specular", &lights[light].specular.x, ImGuiColorEditFlags_NoInputs);
					break;
				}

				ImGui::TreePop();
			}
		}

		if (activeLights.size() < MAX_LIGHTS)
		{
			if (ImGui::Button("Add Light"))
			{
				for (int light = 0; light < MAX_LIGHTS; light++)
				{
					if (std::find(activeLights.begin(), activeLights.end(), &lights[light]) == activeLights.end())
					{
						activeLights.push_back(&lights[light]);
						break;
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Light"))
			{
				activeLights.back()->type = static_cast<int>(LightType::OFF);
				activeLights.erase(activeLights.begin() + activeLights.size() - 1);
			}
		}
	}
}