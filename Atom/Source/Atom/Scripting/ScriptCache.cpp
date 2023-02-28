#include "ATPCH.h"
#include "ScriptCache.h"
#include "ScriptEngine.h"

#include "Atom/Core/Hash.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace Atom
{
	
	struct Cache
	{
		std::unordered_map<uint32_t, ManagedClass> Classes;
	};

	static Cache* s_Cache = nullptr;

	void ScriptCache::Initialize()
	{
		s_Cache = new Cache();

		CacheCoreClasses();
	}

	void ScriptCache::Shutdown()
	{
		ClearCache();

		delete s_Cache;
		s_Cache = nullptr;
	}

	void ScriptCache::ClearCache()
	{
		if(s_Cache == nullptr)
		{
			return;
		}

		s_Cache->Classes.clear();
	}

	void ScriptCache::CacheCoreClasses()
	{
		if(s_Cache == nullptr)
		{
			return;
		}

#define CACHE_ATOM_CORE_CLASS(name) CacheClass("Atom." ##name, mono_class_from_name(ScriptEngine::GetCoreAssemblyInfo()->AssemblyImage, "Atom", name))

		CACHE_ATOM_CORE_CLASS("Entity");
	}

	ManagedClass* ScriptCache::GetManagedClassByName(const std::string& className)
	{
		if(s_Cache == nullptr)
		{
			return nullptr;
		}

		uint32_t classId = Hash::GenerateFNVHash(className);
		if(s_Cache->Classes.find(classId) == s_Cache->Classes.end())
		{
			return nullptr;
		}

		return &s_Cache->Classes[classId];
	}

	void ScriptCache::CacheClass(std::string_view className, MonoClass* monoClass)
	{
		MonoType* monoType = mono_class_get_type(monoClass);

		ManagedClass managedClass;
		managedClass.Id = Hash::GenerateFNVHash(className);
		managedClass.FullName = className;

		int32_t alignment = 0;
		managedClass.Size = mono_type_size(monoType, &alignment);
		managedClass.Class = monoClass;
		s_Cache->Classes[managedClass.Id] = managedClass;
	}

}