#pragma once
#include "Atom/Core/Input/IInput.h"

namespace Atom
{

	class Win32Input : public IInput
	{
	public:
		Win32Input() = default;
		virtual ~Win32Input() = default;

		virtual bool IsKeyDown(KeyCode keycode) override;

		virtual bool IsMouseButtonPressed(MouseButtonCode button) override;

		virtual glm::vec2 GetMousePosition() override;
	};
	
}
