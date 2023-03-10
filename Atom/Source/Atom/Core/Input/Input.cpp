#include "ATPCH.h"
#include "Input.h"
#include "IInput.h"

#include "Atom/Renderer/Renderer.h"

#include "Atom/Platform/Win32/Win32Input.h"

namespace Atom
{


	static IInput* CreateInput()
	{
		switch(Renderer::Current())
		{
			case RendererAPI::DirectX11: return new Win32Input();
			case RendererAPI::OpenGL:
			case RendererAPI::Unknown:
			default: break;
		}
		
		AT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	static IInput* s_Input = nullptr;

	void Input::Initialize()
	{
		s_Input = CreateInput();
	}

	void Input::Shutdown()
	{
		delete s_Input;
	}

	bool Input::IsKeyDown(KeyCode keycode)
	{
		return s_Input->IsKeyDown(keycode);
	}

	bool Input::IsMouseButtonPressed(MouseButtonCode button)
	{
		return s_Input->IsMouseButtonPressed(button);
	}

	glm::vec2 Input::GetMousePosition()
	{
		return s_Input->GetMousePosition();
	}

	float Input::GetMouseX()
	{
		return s_Input->GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return s_Input->GetMousePosition().y;
	}

}