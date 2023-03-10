#include "ATPCH.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"

#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Physics/2D/Physics2D.h"

#include "Atom/Renderer/Renderer2D.h"

namespace Atom
{

#if 0
	static void OnAddRigidbody2DComponent(entt::registry& registry, entt::entity e, Scene* scene)
	{
		AT_CORE_TRACE("Added Rigidbody2D Component at Scene runtime!");
		Physics2D::CreatePhysicsBody({ e, scene });
	}

	void Scene::OnRigidbody2DComponent(entt::registry& registry, entt::entity e)
	{
		Physics2D::CreatePhysicsBody({ e, this });
	}
#endif

	static void Test(entt::registry& registry, entt::entity entity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene) {
			return;
		}

		Physics2D::CreatePhysicsBody({ entity, scene });
	}

	Scene::Scene()
	{
		// TODO: Move to OnRuntimeStart
		//m_Registry.on_construct<Component::Rigidbody2D>().connect<&Scene::OnRigidbody2DComponent>();
		m_Registry.on_construct<Component::Rigidbody2D>().connect<&Test>();
	}

	Scene::~Scene()
	{
		// TODO: Move to OnRuntimeStop
		//m_Registry.on_construct<Component::Rigidbody2D>().disconnect<&OnAddRigidbody2DComponent>();
		m_Registry.on_construct<Component::Rigidbody2D>().disconnect<&Test>();
	}

	template<typename TComponent>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<TComponent>();
		for (auto e : view)
		{
			UUID uuid = src.get<Component::Identifier>(e).ID;
			AT_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
			entt::entity dstEnttId = enttMap.at(uuid);

			auto& component = src.get<TComponent>(e);
			dst.emplace_or_replace<TComponent>(dstEnttId, component);
		}
	}

	template<typename TComponent>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<TComponent>())
		{
			dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());
		}
	}

	Scene* Scene::Copy(Scene* other)
	{
		Scene* newScene = new Scene();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		// Creat entities in new scene
		auto identifierView = srcSceneRegistry.view<Component::Identifier>();
		for (auto e : identifierView)
		{
			UUID uuid = srcSceneRegistry.get<Component::Identifier>(e).ID;
			const auto& name = srcSceneRegistry.get<Component::Identifier>(e).Name;
			Entity newEntity = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except for Identifier)
		CopyComponent<Component::Transform>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::Camera>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::BasicRenderer>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::CircleRenderer>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::Script>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::Rigidbody2D>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::BoxCollider2D>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Component::TextRenderer>(dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntity(UUID(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		auto& identifier = entity.AddComponent<Component::Identifier>(uuid);
		identifier.Name = name.empty() ? "Entity" : name;

		entity.AddComponent<Component::Transform>();

		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		Physics2D::DestroyPhysicsBody(entity);

		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<Component::Transform>(newEntity, entity);
		CopyComponentIfExists<Component::Camera>(newEntity, entity);
		CopyComponentIfExists<Component::BasicRenderer>(newEntity, entity);
		CopyComponentIfExists<Component::CircleRenderer>(newEntity, entity);
		CopyComponentIfExists<Component::Script>(newEntity, entity);
		CopyComponentIfExists<Component::Rigidbody2D>(newEntity, entity);
		CopyComponentIfExists<Component::BoxCollider2D>(newEntity, entity);
		CopyComponentIfExists<Component::TextRenderer>(newEntity, entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
		{
			return;
		}

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<Component::Camera>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<Component::Camera>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.SceneCamera.SetViewportSize(width, height);
			}
		}
	}

	void Scene::OnEditorUpdate(float deltaTime, EditorCamera& editorCamera)
	{
		Renderer2D::BeginScene(editorCamera);

		{
			auto group = m_Registry.group<Component::Transform>(entt::get<Component::BasicRenderer>);
			for (auto entity : group)
			{
				auto [transform, basic] = group.get<Component::Transform, Component::BasicRenderer>(entity);

				Renderer2D::RenderQuad(transform.GetTransform(), basic.Color, (int32_t)entity);
			}
		}

		{
			auto view = m_Registry.view<Component::Transform, Component::CircleRenderer>();
			for (auto entity : view)
			{
				auto [transform, circleRenderer] = view.get<Component::Transform, Component::CircleRenderer>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circleRenderer.Color, circleRenderer.Thickness, circleRenderer.Fade, (int32_t)entity);
			}
		}

		{
			auto view = m_Registry.view<Component::Transform, Component::TextRenderer>();
			for (auto entity : view)
			{
				auto [transform, textRenderer] = view.get<Component::Transform, Component::TextRenderer>(entity);

				textRenderer.FontAsset = Font::GetDefaultFont();
				Renderer2D::DrawString(textRenderer.Text, textRenderer.FontAsset, transform.GetTransform(), { textRenderer.Color, textRenderer.Kerning, textRenderer.LineSpacing }, (int32_t)entity);
			}
		}

		Renderer2D::EndScene();
	}

	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		// Physics 2D
		{
			Physics2D::OnRuntimeStart();

			auto view = m_Registry.view<Component::Rigidbody2D>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				Physics2D::CreatePhysicsBody(entity);
			}
		}

		// Script
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = m_Registry.view<Component::Script>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;

		// Physics 2D
		{
			Physics2D::OnRuntimeStop();
		}

		// Script
		{
			// TODO: Maybe call OnDestroyEntity?
			auto view = m_Registry.view<Component::Script>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnDestroyEntity(entity);
			}

			ScriptEngine::OnRuntimeStop();
		}
	}

	void Scene::OnRuntimeUpdate(float deltaTime)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			// Script 
			{
				auto view = m_Registry.view<Component::Script>();
				for (auto e : view)
				{
					Entity entity{ e, this };
					ScriptEngine::OnUpdateEntity(entity, deltaTime);
				}
			}

			// Physics 2D
			{
				Physics2D::Step(deltaTime);
				auto view = m_Registry.view<Component::Rigidbody2D>();
				for (auto e : view)
				{
					Entity entity{ e, this };

					Physics2D::OnRuntimeUpdate(entity);
				}
			}
		}

		// Renderer 2D
		{
			Camera* mainCamera = nullptr;
			glm::mat4 cameraTransform;
			{
				auto view = m_Registry.view<Component::Transform, Component::Camera>();
				for (auto entity : view)
				{
					auto [transform, camera] = view.get<Component::Transform, Component::Camera>(entity);

					if (camera.Primary)
					{
						mainCamera = &camera.SceneCamera;
						cameraTransform = transform.GetTransform();
						break;
					}
				}
			}

			if (mainCamera)
			{
				Renderer2D::BeginScene(*mainCamera, cameraTransform);

				{
					auto group = m_Registry.group<Component::Transform>(entt::get<Component::BasicRenderer>);
					for (auto entity : group)
					{
						auto [transform, basic] = group.get<Component::Transform, Component::BasicRenderer>(entity);

						Renderer2D::RenderQuad(transform.GetTransform(), basic.Color);
					}
				}

				{
					auto view = m_Registry.view<Component::Transform, Component::CircleRenderer>();
					for (auto e : view)
					{
						auto [transform, circleRenderer] = view.get<Component::Transform, Component::CircleRenderer>(e);

						Renderer2D::DrawCircle(transform.GetTransform(), circleRenderer.Color, circleRenderer.Thickness, circleRenderer.Fade);
					}
				}

				{
					auto view = m_Registry.view<Component::Transform, Component::TextRenderer>();
					for (auto e : view)
					{
						auto [transform, textRenderer] = view.get<Component::Transform, Component::TextRenderer>(e);

						textRenderer.FontAsset = Font::GetDefaultFont();
						Renderer2D::DrawString(textRenderer.Text, textRenderer.FontAsset, transform.GetTransform(), { textRenderer.Color, textRenderer.Kerning, textRenderer.LineSpacing });
					}
				}

				Renderer2D::EndScene();
			}
		}

		ExecutePostRuntimeUpdateQueue();
	}

	void Scene::SubmitToPostRuntimeUpdateQueue(const std::function<void()>& function)
	{
		m_PostRuntimeUpdateQueue.emplace_back(function);
	}

	void Scene::ExecutePostRuntimeUpdateQueue()
	{
		for (auto& function : m_PostRuntimeUpdateQueue)
		{
			function();
		}

		m_PostRuntimeUpdateQueue.clear();
	}

	void Scene::OnSimulationStart()
	{
		// Physics 2D
		{
			Physics2D::OnRuntimeStart();

			auto view = m_Registry.view<Component::Rigidbody2D>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				Physics2D::CreatePhysicsBody(entity);
			}
		}
	}

	void Scene::OnSimulationStop()
	{
		// Physics 2D
		{
			Physics2D::OnRuntimeStop();
		}
	}

	void Scene::OnSimulationUpdate(float deltaTime, EditorCamera& editorCamera)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			// Physics 2D
			{
				Physics2D::Step(deltaTime);
				auto view = m_Registry.view<Component::Rigidbody2D>();
				for (auto e : view)
				{
					Entity entity{ e, this };

					Physics2D::OnRuntimeUpdate(entity);
				}
			}
		}

		// Renderer 2D
		Renderer2D::BeginScene(editorCamera);

		{
			auto group = m_Registry.group<Component::Transform>(entt::get<Component::BasicRenderer>);
			for (auto entity : group)
			{
				auto [transform, basic] = group.get<Component::Transform, Component::BasicRenderer>(entity);

				Renderer2D::RenderQuad(transform.GetTransform(), basic.Color, (int32_t)entity);
			}

			{
				auto view = m_Registry.view<Component::Transform, Component::CircleRenderer>();
				for (auto entity : view)
				{
					auto [transform, circleRenderer] = view.get<Component::Transform, Component::CircleRenderer>(entity);

					Renderer2D::DrawCircle(transform.GetTransform(), circleRenderer.Color, circleRenderer.Thickness, circleRenderer.Fade, (int32_t)entity);
				}
			}

			{
				auto view = m_Registry.view<Component::Transform, Component::TextRenderer>();
				for (auto entity : view)
				{
					auto [transform, textRenderer] = view.get<Component::Transform, Component::TextRenderer>(entity);

					textRenderer.FontAsset = Font::GetDefaultFont();
					Renderer2D::DrawString(textRenderer.Text, textRenderer.FontAsset, transform.GetTransform(), { textRenderer.Color, textRenderer.Kerning, textRenderer.LineSpacing }, (int32_t)entity);
				}
			}
		}

		Renderer2D::EndScene();
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<Component::Identifier>();
		for (auto entity : view)
		{
			const auto& identifier = view.get<Component::Identifier>(entity);
			if (identifier.Name == name)
			{
				return Entity{ entity, this };
			}
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
		{
			return { m_EntityMap[uuid], this };
		}
		return {};
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<Component::Camera>();
		for (auto entity : view)
		{
			const auto& camera = view.get<Component::Camera>(entity);
			if (camera.Primary)
			{
				return Entity{ entity, this };
			}
		}
		return {};
	}

	bool Scene::IsEntityValid(Entity entity) const
	{
		UUID entityId = entity.GetUUID();
		return IsEntityValid(entityId);
	}

	bool Scene::IsEntityValid(UUID entityId) const
	{
		auto iterator = m_EntityMap.find(entityId);
		if (iterator == m_EntityMap.end())
		{
			return false;
		}

		entt::entity enttEntity = m_EntityMap.at(entityId);
		return m_Registry.valid(enttEntity);
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

}