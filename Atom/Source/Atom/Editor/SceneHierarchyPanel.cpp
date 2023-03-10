#include "ATPCH.h"
#include "SceneHierarchyPanel.h"
#include "Atom/Scene/Components.h"

#include "Atom/Physics/2D/Physics2DBodyTypes.h"

#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Scripting/ScriptCache.h"

#include "Atom/ImGui/UICore.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Atom/ImGui/ImGuiUtillities.h"

#include <glm/gtc/type_ptr.hpp>

namespace Atom
{

#define ADD_COMPONENT_POP_UP_IDENTIFIER "AddComponent"
#define COMPONENT_SETTINGS_POP_UP_IDENTIFIER "ComponentSettings"

	namespace Utils
	{

		template<typename Component>
		static void DrawComponent(const std::string& name, Entity entity, std::function<void(Component&)> uiFunction)
		{
			if(!entity.HasComponent<Component>())
			{
				return;
			}

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			auto& component = entity.GetComponent<Component>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(Component).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if(ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup(COMPONENT_SETTINGS_POP_UP_IDENTIFIER);
			}

			bool removeComponent = false;
			if(ImGui::BeginPopup(COMPONENT_SETTINGS_POP_UP_IDENTIFIER))
			{
				if(ImGui::MenuItem("Remove component"))
				{
					removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if(open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if(removeComponent)
			{
				entity.RemoveComponent<Component>();
			}
		}

		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			UI::Column2(label, [&]()
			{
				ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if(ImGui::Button("X", buttonSize))
				values.x = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if(ImGui::Button("Y", buttonSize))
				values.y = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if(ImGui::Button("Z", buttonSize))
				values.z = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();
			});
		}

	}

	SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene)
	{
		SetSceneContext(scene);
	}

	void SceneHierarchyPanel::OnImGuiRender(bool& isOpen)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene Hierarchy", &isOpen);
		ImGui::PopStyleVar();

		if(m_Scene)
		{
			auto& view = m_Scene->GetAllEntitiesWith<Component::Identifier>();
			for(auto e : view)
			{
				Entity entity{ e, m_Scene };
				DrawEntityNode(entity);
			}

			if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectedEntity = {};
			}

			// Right-click on blank space
			if(ImGui::BeginPopupContextWindow("Create", 1))
			{
				if(ImGui::MenuItem("Create Empty Entity"))
				{
					m_Scene->CreateEntity("Empty Entity");
				}

				if(ImGui::MenuItem("Create Camera"))
				{
					auto& entity = m_Scene->CreateEntity("Camera");
					entity.AddComponent<Component::Camera>();
				}

				ImGui::Separator();

				if(ImGui::BeginMenu("2D"))
				{
					if(ImGui::MenuItem("Create Basic Sprite"))
					{
						auto& entity = m_Scene->CreateEntity("Basic Sprite");
						entity.AddComponent<Component::BasicRenderer>();
					}
					
					if(ImGui::MenuItem("Create Circle"))
					{
						auto& entity = m_Scene->CreateEntity("Circle");
						entity.AddComponent<Component::CircleRenderer>();
					}
					
					ImGui::EndMenu();
				}
				
				ImGui::EndPopup();
			}
		}

		ImGui::End();

		ImGui::Begin("Entity Inspector");

		if(m_SelectedEntity)
		{
			DrawComponents(m_SelectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::SetSceneContext(Scene* scene)
	{
		m_Scene = scene;
		m_SelectedEntity = { };
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& name = entity.GetComponent<Component::Identifier>().Name;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if(ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		bool entityDeleted = false;
		if(ImGui::BeginPopupContextItem("Delete"))
		{
			if(ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}

		if(opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, name.c_str());
			if(opened)
			{
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if(entityDeleted)
		{
			m_Scene->DestroyEntity(entity);
			if(m_SelectedEntity == entity)
			{
				m_SelectedEntity = {};
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if(entity.HasComponent<Component::Identifier>())
		{
			auto& name = entity.GetComponent<Component::Identifier>().Name;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), name.c_str(), sizeof(buffer));
			if(ImGui::InputText("##Name", buffer, sizeof(buffer)))
			{
				name = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if(ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup(ADD_COMPONENT_POP_UP_IDENTIFIER);
		}
		DrawAddComponentPopup();

		ImGui::PopItemWidth();

		Utils::DrawComponent<Component::Transform>("Transform", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawTransformComponent));

		Utils::DrawComponent<Component::Camera>("Camera", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawCameraComponent));

		Utils::DrawComponent<Component::TextRenderer>("Text Renderer", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawTextRendererComponent));

		Utils::DrawComponent<Component::BasicRenderer>("Basic Renderer", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawBasicRendererComponent));

		Utils::DrawComponent<Component::CircleRenderer>("Circle Renderer", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawCircleRendererComponent));

		Utils::DrawComponent<Component::Script>("C# Script", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawScriptComponent));

		Utils::DrawComponent<Component::Rigidbody2D>("Rigidbody 2D", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawRigidbody2DComponent));

		Utils::DrawComponent<Component::BoxCollider2D>("Box Collider 2D", entity, AT_BIND_EVENT_FN(SceneHierarchyPanel::DrawBoxCollider2DComponent));
	}

	void SceneHierarchyPanel::DrawTextRendererComponent(Component::TextRenderer& component)
	{
		static char buffer[1024 * 10];
		strcpy_s(buffer, sizeof(buffer), component.Text.c_str());

		if(ImGui::InputTextMultiline("Text", buffer, sizeof(buffer)))
		{
			component.Text = buffer;
		}
		ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		ImGui::DragFloat("Kerning", &component.Kerning, 0.025f);
		ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.025f);
	}

	void SceneHierarchyPanel::DrawTransformComponent(Component::Transform& component)
	{
		Utils::DrawVec3Control("Translation", component.Position);
		glm::vec3 rotation = glm::degrees(component.Rotation);
		Utils::DrawVec3Control("Rotation", rotation);
		component.Rotation = glm::radians(rotation);
		Utils::DrawVec3Control("Scale", component.Scale, 1.0f);
	}

	void SceneHierarchyPanel::DrawCameraComponent(Component::Camera& component)
	{
		auto& camera = component.SceneCamera;

		ImGui::Checkbox("Primary", &component.Primary);

		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
		if(ImGui::BeginCombo("Projection", currentProjectionTypeString))
		{
			for(int i = 0; i < 2; i++)
			{
				bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
				if(ImGui::Selectable(projectionTypeStrings[i], isSelected))
				{
					currentProjectionTypeString = projectionTypeStrings[i];
					camera.SetProjectionType((SceneCamera::ProjectionType)i);
				}

				if(isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
		{
			float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
			if(ImGui::DragFloat("Vertical FOV", &verticalFOV))
			{
				camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));
			}

			float perspectiveNear = camera.GetPerspectiveNearClip();
			if(ImGui::DragFloat("Near", &perspectiveNear))
			{
				camera.SetPerspectiveNearClip(perspectiveNear);
			}

			float perspectiveFar = camera.GetPerspectiveFarClip();
			if(ImGui::DragFloat("Far", &perspectiveFar))
			{
				camera.SetPerspectiveFarClip(perspectiveFar);
			}
		}

		if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
		{
			float orthographicSize = camera.GetOrthographicSize();
			if(ImGui::DragFloat("Size", &orthographicSize))
			{
				camera.SetOrthographicSize(orthographicSize);
			}

			float orthographicNear = camera.GetOrthographicNearClip();
			if(ImGui::DragFloat("Near", &orthographicNear))
			{
				camera.SetOrthographicNearClip(orthographicNear);
			}

			float orthographicFar = camera.GetOrthographicFarClip();
			if(ImGui::DragFloat("Far", &orthographicFar))
			{
				camera.SetOrthographicFarClip(orthographicFar);
			}

			ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
		}
	}

	void SceneHierarchyPanel::DrawBasicRendererComponent(Component::BasicRenderer& component)
	{
		UI::Column2("Color", [&]()
		{
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::PushItemWidth(width);
			ImGui::ColorEdit4("##Color", glm::value_ptr(component.Color));
			ImGui::PopItemWidth();
		});
	}

	void SceneHierarchyPanel::DrawCircleRendererComponent(Component::CircleRenderer& component)
	{
		UI::BeginPropertyGrid();

		ImGui::SetColumnWidth(0, 100.0f);
		ImGui::Text("Color");
		{
			ImGui::NextColumn();
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::PushItemWidth(width);

			ImGui::ColorEdit4("##Color", glm::value_ptr(component.Color));

			ImGui::PopItemWidth();
		}

		ImGui::NextColumn();
		
		ImGui::Text("Thickness");
		{
			ImGui::NextColumn();
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::PushItemWidth(width);

			ImGui::DragFloat("##Thickness", &component.Thickness, 0.05f, 0.0f, 1.0f);

			ImGui::PopItemWidth();
		}

		ImGui::NextColumn();

		ImGui::Text("Fade");
		{
			ImGui::NextColumn();
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::PushItemWidth(width);

			ImGui::DragFloat("##Fade", &component.Fade, 0.001f, 0.0f, 1.0f);

			ImGui::PopItemWidth();
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::DrawScriptComponent(Component::Script& component)
	{
		//bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

		ManagedClass* managedClass = AT_CACHED_ENTITY_CLASS(component.ClassName);

		static char buffer[64];
		strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

		if(managedClass == nullptr)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
		}

		if(ImGui::InputText("Class", buffer, sizeof(buffer)))
		{
			component.ClassName = buffer;
		}

		// Fields
		bool sceneRunning = m_Scene->IsRunning();
		if(sceneRunning)
		{
			if(managedClass)
			{
				const auto& classFields = managedClass->GetFields();

				for(const auto& [fieldName, field] : classFields)
				{
					ManagedClassField& classField = managedClass->GetField(fieldName);

#define AT_CASE_RUNTIME_SCRIPT_FIELD(managedType, type, func) case managedType: { type data = ScriptEngine::GetEntityInstanceFieldValue<type>(m_SelectedEntity, &classField); if(func) { ScriptEngine::SetEntityInstanceFieldValue<type>(m_SelectedEntity, &classField, data); } } break

					switch(field.GetType())
					{
						AT_CASE_RUNTIME_SCRIPT_FIELD(ManagedFieldType::Bool, bool, ImGui::Checkbox(fieldName.c_str(), &data));
						AT_CASE_RUNTIME_SCRIPT_FIELD(ManagedFieldType::Char, char, ImGui::InputText(fieldName.c_str(), &data, sizeof(char)));
						AT_CASE_RUNTIME_SCRIPT_FIELD(ManagedFieldType::Float, float, ImGui::DragFloat(fieldName.c_str(), &data));
						default: break;
					}

#undef AT_CASE_RUNTIME_SCRIPT_FIELD
				}
			}
		}
		else
		{
			if(managedClass)
			{
				const auto& classFields = managedClass->GetFields();

				for(const auto& [fieldName, field] : classFields)
				{
					ManagedClassField& classField = managedClass->GetField(fieldName);

#define AT_SCRIPT_FIELD_TYPE_CASE(managedType, type, func) case managedType: { type data = classField.GetValue<type>(); if(func) { classField.SetValue<type>(data); } } break

					switch(field.GetType())
					{
						AT_SCRIPT_FIELD_TYPE_CASE(ManagedFieldType::Bool, bool, ImGui::Checkbox(fieldName.c_str(), &data));
						AT_SCRIPT_FIELD_TYPE_CASE(ManagedFieldType::Char, char, ImGui::InputText(fieldName.c_str(), &data, sizeof(char)));
						AT_SCRIPT_FIELD_TYPE_CASE(ManagedFieldType::Float, float, ImGui::DragFloat(fieldName.c_str(), &data));
						default: break;
					}

#undef AT_SCRIPT_FIELD_TYPE_CASE
				}
			}
		}


		if(managedClass == nullptr)
		{
			ImGui::PopStyleColor();
		}
	}

	void SceneHierarchyPanel::DrawRigidbody2DComponent(Component::Rigidbody2D& component)
	{
		const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
		const char* currentBodyTypeString = bodyTypeStrings[(int)component.BodyType];
		if(ImGui::BeginCombo("Body Type", currentBodyTypeString))
		{
			for(int i = 0; i < 3; i++)
			{
				bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
				if(ImGui::Selectable(bodyTypeStrings[i], isSelected))
				{
					currentBodyTypeString = bodyTypeStrings[i];
					component.BodyType = (PhysicsBodyType)i;
				}

				if(isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		ImGui::Checkbox("Affected by gravity", &component.AffectedByGravity);
		if (component.AffectedByGravity) 
		{
			ImGui::DragFloat("Gravity Scale", &component.GravityScale, 0.1f, 0.0f, 1.0f);
		}
	}

	void SceneHierarchyPanel::DrawBoxCollider2DComponent(Component::BoxCollider2D& component)
	{
		ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.1f);
		ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.1f);
		ImGui::Checkbox("Is Trigger", &component.IsTrigger);

		ImGui::Text("Physics Material:");
		ImGui::DragFloat("Density", &component.Density, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
	}

	void SceneHierarchyPanel::DrawAddComponentPopup()
	{
		if(ImGui::BeginPopup(ADD_COMPONENT_POP_UP_IDENTIFIER))
		{
			DrawAddComponentPopupItem<Component::Camera>("Camera");
			DrawAddComponentPopupItem<Component::Script>("C# Script");
			DrawAddComponentPopupItem<Component::TextRenderer>("Text Renderer");
			DrawAddComponentPopupItem<Component::BasicRenderer>("Basic Renderer");
			DrawAddComponentPopupItem<Component::CircleRenderer>("Circle Renderer");
			DrawAddComponentPopupItem<Component::Rigidbody2D>("Rigidbody 2D");
			DrawAddComponentPopupItem<Component::BoxCollider2D>("Box Collider 2D");

			ImGui::EndPopup();
		}
	}

	template<typename Component>
	inline void SceneHierarchyPanel::DrawAddComponentPopupItem(const std::string& text)
	{
		if(m_SelectedEntity.HasComponent<Component>())
		{
			return;
		}

		if(ImGui::MenuItem(text.c_str()))
		{
			m_SelectedEntity.AddComponent<Component>();
			ImGui::CloseCurrentPopup();
		}
	}

}