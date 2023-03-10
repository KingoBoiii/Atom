#pragma once
#include "Atom/Renderer/UniformBuffer.h"

#include "DirectX11.h"

namespace Atom
{
	
	class DX11UniformBuffer : public UniformBuffer
	{
	public:
		DX11UniformBuffer(const void* data, uint32_t size, uint32_t slot);
		virtual ~DX11UniformBuffer();

		virtual void SetData(const void* data, uint32_t size) const override;

		virtual void Bind() const override;
	private:
		void Invalidate(const void* data, uint32_t size);
	private:
		uint32_t m_Slot;
		
		ID3D11Device* m_Device;
		ID3D11DeviceContext* m_DeviceContext;

		ID3D11Buffer* m_Buffer;
	};

}

