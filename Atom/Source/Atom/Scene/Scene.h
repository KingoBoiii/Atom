#pragma once
#include "Atom/Core/UUID.h"
#include "Atom/Graphics/Renderer2D.h"

#include "entt.hpp"

namespace Atom
{

	class ATOM_API Entity;

	class ATOM_API Scene
	{
	public:
		Scene(Renderer2D* renderer2D);
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntity(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnViewportResize(uint32_t width, uint32_t height);

		void OnUpdateRuntime();

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
	private:
		entt::registry m_Registry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Renderer2D* m_Renderer2D = nullptr;

		friend class Entity;
	};

}