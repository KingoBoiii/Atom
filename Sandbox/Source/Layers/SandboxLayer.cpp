#include "SandboxLayer.h"

void SandboxLayer::OnAttach()
{
	Atom::Window* window = Atom::Application::Get().GetWindow();

	Atom::RendererOptions rendererOptions{ };
	rendererOptions.ClearColor = new float[4] { 0.1f, 0.1f, 0.1f, 1.0f };
	rendererOptions.SwapChain = window->GetSwapChain();
	m_Renderer = Atom::RendererFactory::Create(rendererOptions);
	m_Renderer->Initialize();

	Atom::VertexBufferOptions vertexBufferOptions{ };
	vertexBufferOptions.Vertices = new float[3 * 3] { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f };
	vertexBufferOptions.Size = 3 * 3 * sizeof(float);
	vertexBufferOptions.Stride = 3 * sizeof(float);
	vertexBufferOptions.Offset = 0;
	m_VertexBuffer = Atom::VertexBufferFactory::Create(vertexBufferOptions);
}

void SandboxLayer::OnDetach()
{
}

void SandboxLayer::OnUpdate()
{
	m_Renderer->Clear();
}
