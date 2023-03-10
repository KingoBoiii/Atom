#pragma once
#include "Scene.h"

namespace Atom
{

	class SceneSerializer
	{
	public:
		SceneSerializer(Scene* scene);
		~SceneSerializer() = default;

		void Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);

		void SerializeRuntime(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Scene* m_Scene;
	};

}