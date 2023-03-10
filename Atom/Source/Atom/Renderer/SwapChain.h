#pragma once

namespace Atom
{

	class Window;

	class SwapChain
	{
	public:
		static SwapChain* Create(Window* window);
	public:
		virtual void Initialize() = 0;
		virtual void Present() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
	};

}
