#pragma once

namespace Atom
{

	class ATOM_API ScriptEngine
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		void Initialize();
		void Shutdown();

		inline static ScriptEngine& Get() { return *s_Instance; }
	private:
		void InitializeMono();
		void ShutdownMono();
	private:
		static ScriptEngine* s_Instance;
	};

}