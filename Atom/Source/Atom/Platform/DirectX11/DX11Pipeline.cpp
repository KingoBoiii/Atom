#include "ATPCH.h"
#include "DX11Pipeline.h"
#include "DX11RendererContext.h"
#include "DX11Shader.h"

namespace Atom
{

	namespace Utils
	{

		DXGI_FORMAT AtomShaderTypeToDirectX(ShaderDataType shaderDataType)
		{
			switch(shaderDataType)
			{
				case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
				case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
				case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
				case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case ShaderDataType::Int:		return DXGI_FORMAT_R32G32B32A32_SINT;
				case ShaderDataType::None:
				default: break;
			}

			return DXGI_FORMAT_UNKNOWN;
		}

		D3D11_PRIMITIVE_TOPOLOGY AtomPrimitiveTopologyToDirectX(PrimitiveTopology primitiveTopology)
		{
			switch(primitiveTopology)
			{
				case Atom::PrimitiveTopology::Points:		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
				case Atom::PrimitiveTopology::Lines:		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
				case Atom::PrimitiveTopology::Triangles:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				case Atom::PrimitiveTopology::None:
				default: break;
			}

			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

	}

	DX11Pipeline::DX11Pipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		DX11RendererContext& context = DX11RendererContext::Get();

		m_Device = context.m_Device;
		m_DeviceContext = context.m_DeviceContext;

		Invalidate();
	}

	DX11Pipeline::~DX11Pipeline()
	{
		ReleaseCOM(m_InputLayout);
		ReleaseCOM(m_RasterizerState);

		m_Device = nullptr;
		m_DeviceContext = nullptr;
	}

	void DX11Pipeline::Bind() const
	{
		m_Specification.Shader->Bind();

		m_DeviceContext->IASetPrimitiveTopology(Utils::AtomPrimitiveTopologyToDirectX(m_Specification.PrimitiveTopology));
		m_DeviceContext->IASetInputLayout(m_InputLayout);
		m_DeviceContext->RSSetState(m_RasterizerState);
	}

	void DX11Pipeline::Invalidate()
	{
		CreateInputLayout();
		CreateRasterizerState();
	}

	void DX11Pipeline::CreateInputLayout()
	{
		std::vector<BufferElement> layout = m_Specification.InputLayout.GetElements();
		D3D11_INPUT_ELEMENT_DESC* desc = new D3D11_INPUT_ELEMENT_DESC[layout.size()];
		for(uint32_t i = 0; i < layout.size(); i++)
		{
			const BufferElement& element = layout[i];
			desc[i] = { element.Name.c_str(), 0, Utils::AtomShaderTypeToDirectX(element.Type), 0, (uint32_t)element.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		}

		DX11Shader* shader = reinterpret_cast<DX11Shader*>(m_Specification.Shader);

		HRESULT hr = m_Device->CreateInputLayout(desc, layout.size(), shader->m_VertexSourceBlob->GetBufferPointer(), shader->m_VertexSourceBlob->GetBufferSize(), &m_InputLayout);
		AT_CORE_ASSERT(SUCCEEDED(hr));
	}

	void DX11Pipeline::CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC desc{};
		desc.FillMode = m_Specification.Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.FrontCounterClockwise = true;
		desc.DepthClipEnable = true;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.AntialiasedLineEnable = false;
		desc.MultisampleEnable = false;
		desc.ScissorEnable = false;
		desc.SlopeScaledDepthBias = 0.0f;

		HRESULT hr = m_Device->CreateRasterizerState(&desc, &m_RasterizerState);
		AT_CORE_ASSERT(SUCCEEDED(hr), "Failed to create Rasterizer State");
	}

}

