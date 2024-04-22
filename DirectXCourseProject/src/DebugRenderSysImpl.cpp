#include "pch.h"
#include "DebugRenderSysImpl.h"
#include "Application.h"
#include "Camera.h"

using namespace DirectX::SimpleMath;

DebugRenderSysImpl::DebugRenderSysImpl(Engine::Application* inGame) : game(inGame)
{
	D3D11_BUFFER_DESC constDesc = {};
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = 0;
	constDesc.MiscFlags = 0;
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	constDesc.ByteWidth = sizeof(Matrix);

	game->getDevice()->CreateBuffer(&constDesc, nullptr, &constBuf);
	
	InitPrimitives();
	InitQuads();
	InitMeshes();
}


void DebugRenderSysImpl::InitPrimitives()
{
	HRESULT res;
	ID3DBlob* errorCode = nullptr;
	
	res = D3DCompileFromFile(L"./Shaders/Simple.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
		0,
		&vertexPrimCompResult,
		&errorCode);

	if(errorCode) errorCode->Release();
	
	res = D3DCompileFromFile(L"./Shaders/Simple.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
		0,
		&pixelPrimCompResult,
		&errorCode);

	if (errorCode) errorCode->Release();

	game->getDevice()->CreateVertexShader(vertexPrimCompResult->GetBufferPointer(), vertexPrimCompResult->GetBufferSize(), nullptr, &vertexPrimShader);
	game->getDevice()->CreatePixelShader(pixelPrimCompResult->GetBufferPointer(), pixelPrimCompResult->GetBufferSize(), nullptr, &pixelPrimShader);

	layoutPrim = VertexPositionColor::GetLayout(vertexPrimCompResult);


	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags = 0;
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufDesc.ByteWidth = MaxPointsCount * VertexPositionColor::Stride;

	game->getDevice()->CreateBuffer(&bufDesc, nullptr, &verticesPrim);
	bufPrimStride = VertexPositionColor::Stride;
	
	
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;

	res = game->getDevice()->CreateRasterizerState(&rastDesc, &rastState);

	
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = false;
	
	game->getDevice()->CreateDepthStencilState(&depthDesc, &depthState);

	lines.reserve(100);
}


void DebugRenderSysImpl::InitQuads()
{
	quadProjMatrix = Matrix::CreateOrthographicOffCenter(0, static_cast<float>(game->getDisplay()->getWidth()), static_cast<float>(game->getDisplay()->getHeight()), 0, 0.1f, 1000.0f);

	ID3DBlob* errorCode = nullptr;
	
	D3DCompileFromFile(L"Shaders/TexturedShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &vertexQuadCompResult, &errorCode);
	game->getDevice()->CreateVertexShader(vertexQuadCompResult->GetBufferPointer(), vertexQuadCompResult->GetBufferSize(), nullptr, &vertexQuadShader);

	if (errorCode)errorCode->Release();
	
	D3DCompileFromFile(L"Shaders/TexturedShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &pixelQuadCompResult, &errorCode);
	game->getDevice()->CreatePixelShader(pixelQuadCompResult->GetBufferPointer(), pixelQuadCompResult->GetBufferSize(), nullptr, &pixelQuadShader);

	if (errorCode)errorCode->Release();


	quadLayout = VertexPositionTex::GetLayout(vertexQuadCompResult);
	quadBindingStride = VertexPositionTex::Stride;

	quads.reserve(10);

	auto points = new Vector4[8] {
		Vector4(1, 1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 0.0f, 0.0f),
		Vector4(0, 1, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4(1, 0, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f),
		Vector4(0, 0, 0.0f, 1.0f), Vector4(0.0f, 0.0f, 0.0f, 0.0f),
	};

	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.ByteWidth = sizeof(float) * 4 * 8;
	}

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = points;
	
	game->getDevice()->CreateBuffer(&bufDesc, &subData, &quadBuf);

	delete[] points;

	float borderCol[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	
	D3D11_SAMPLER_DESC samplDesc = {};
	{
		samplDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplDesc.BorderColor[0] = 1.0f;
		samplDesc.BorderColor[1] = 0.0f;
		samplDesc.BorderColor[2] = 0.0f;
		samplDesc.BorderColor[3] = 1.0f;
		samplDesc.MaxLOD = static_cast<float>(INT_MAX);
	}
	game->getDevice()->CreateSamplerState(&samplDesc, &quadSampler);

	D3D11_RASTERIZER_DESC rastDesc = {};
	{
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;
	}

	game->getDevice()->CreateRasterizerState(&rastDesc, &quadRastState);
}


void DebugRenderSysImpl::InitMeshes()
{
	ID3DBlob* errorCode;

	D3DCompileFromFile(L"Shaders/Simple.hlsl", nullptr, nullptr, "VSMainMesh", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &vertexMeshCompResult, &errorCode);
	game->getDevice()->CreateVertexShader(vertexMeshCompResult->GetBufferPointer(), vertexMeshCompResult->GetBufferSize(), nullptr, &vertexMeshShader);

	if(errorCode) errorCode->Release();

	D3DCompileFromFile(L"Shaders/Simple.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &pixelMeshCompResult, &errorCode);
	game->getDevice()->CreatePixelShader(pixelMeshCompResult->GetBufferPointer(), pixelMeshCompResult->GetBufferSize(), nullptr, &pixelMeshShader);

	if (errorCode) errorCode->Release();

	meshLayout = VertexPositionNormalBinormalTangentColorTex::GetLayout(vertexMeshCompResult);

	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.ByteWidth = sizeof(MeshConstData);
	}

	game->getDevice()->CreateBuffer(&bufDesc, nullptr, &meshBuf);
}


void DebugRenderSysImpl::DrawPrimitives()
{
	if (isPrimitivesDirty) {
		UpdateLinesBuffer();
		pointsCount = lines.size();

		isPrimitivesDirty = false;
	}

	auto mat = camera->GetCameraMatrix();

	game->getContext()->UpdateSubresource(constBuf, 0, nullptr, &mat, 0, 0);

	game->getContext()->OMSetDepthStencilState(depthState, 0);
	game->getContext()->RSSetState(rastState);

	game->getContext()->VSSetShader(vertexPrimShader, nullptr, 0);
	game->getContext()->PSSetShader(pixelPrimShader, nullptr, 0);

	game->getContext()->IASetInputLayout(layoutPrim);
	game->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	const UINT offset = 0;
	game->getContext()->IASetVertexBuffers(0, 1, &verticesPrim, &bufPrimStride, &offset);

	game->getContext()->VSSetConstantBuffers(0, 1, &constBuf);

	game->getContext()->Draw(pointsCount, 0);
}


void DebugRenderSysImpl::DrawQuads()
{
	if (quads.empty()) return;

	game->getContext()->OMSetDepthStencilState(depthState, 0);
	game->getContext()->RSSetState(quadRastState);
	
	game->getContext()->VSSetShader(vertexQuadShader, nullptr, 0);
	game->getContext()->PSSetShader(pixelQuadShader, nullptr, 0);

	game->getContext()->IASetInputLayout(quadLayout);
	game->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	const UINT offset = 0;
	game->getContext()->IASetVertexBuffers(0, 1, &quadBuf, &quadBindingStride, &offset);

	game->getContext()->VSSetConstantBuffers(0, 1, &constBuf);

	for(auto& quad : quads) {
		auto mat = quad.TransformMat * quadProjMatrix;
		game->getContext()->UpdateSubresource(constBuf, 0, nullptr, &mat, 0, 0);

		game->getContext()->PSSetShaderResources(0, 1, &quad.Srv);
		game->getContext()->PSSetSamplers(0, 1, &quadSampler);

		game->getContext()->Draw(4, 0);
	}
}


void DebugRenderSysImpl::DrawMeshes()
{
	if (meshes.empty()) return;

	game->getContext()->OMSetDepthStencilState(depthState, 0);
	game->getContext()->RSSetState(rastState);

	game->getContext()->VSSetShader(vertexMeshShader, nullptr, 0);
	game->getContext()->PSSetShader(pixelMeshShader, nullptr, 0);

	game->getContext()->IASetInputLayout(meshLayout);
	game->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	game->getContext()->VSSetConstantBuffers(1, 1, &meshBuf);
	
	const UINT offset = 0;
	for(auto& mesh : meshes) {
		game->getContext()->IASetVertexBuffers(0, 1, &mesh.Mesh->VertexBuffer, &mesh.Mesh->Stride, &offset);
		game->getContext()->IASetIndexBuffer(mesh.Mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		MeshConstData data = MeshConstData {
			mesh.Transform * camera->ViewMatrix * camera->ProjMatrix,
			mesh.Color
		};

		game->getContext()->UpdateSubresource(meshBuf, 0, nullptr, &data, 0, 0);
		
		game->getContext()->DrawIndexed(mesh.Mesh->IndexCount, 0, 0);
	}
}


void DebugRenderSysImpl::UpdateLinesBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	game->getContext()->Map(verticesPrim, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, lines.data(), lines.size() * VertexPositionColor::Stride);

	game->getContext()->Unmap(verticesPrim, 0);
}


DebugRenderSysImpl::~DebugRenderSysImpl()
{
}


void DebugRenderSysImpl::SetCamera(Camera* inCamera)
{
	camera = inCamera;
}


void DebugRenderSysImpl::Draw(float dTime)
{
	if (camera == nullptr) return;

	game->getContext()->ClearState();
	game->RestoreTargets();

	D3D11_VIEWPORT viewport = {
		0,
		0,
		static_cast<float>(game->Display->ClientWidth),
		static_cast<float>(game->Display->ClientHeight),
		0,
		1
	};
	
	game->getContext()->RSSetViewports(1, &viewport);

	DrawPrimitives();
	DrawQuads();
	DrawMeshes();
}


void DebugRenderSysImpl::Clear()
{
	lines.clear();
	quads.clear();
	meshes.clear();
}


void DebugRenderSysImpl::DrawBoundingBox(const DirectX::BoundingBox& box)
{
	Vector3 corners[8];
	
	box.GetCorners(&corners[0]);
	
	DrawLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	DrawLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	DrawLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}


void DebugRenderSysImpl::DrawBoundingBox(const DirectX::BoundingBox& box, const DirectX::SimpleMath::Matrix& transform)
{
	Vector3 corners[8];
	box.GetCorners(&corners[0]);

	for(auto& corner : corners) {
		corner = Vector3::Transform(corner, transform);
	}

	DrawLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	DrawLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	DrawLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugRenderSysImpl::DrawLine(const DirectX::SimpleMath::Vector3& pos0, const DirectX::SimpleMath::Vector3& pos1,
	const DirectX::SimpleMath::Color& color)
{
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos0.x, pos0.y, pos0.z, 1.0f),
			color.ToVector4()
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos1.x, pos1.y, pos1.z, 1.0f),
			color.ToVector4()
		});

	isPrimitivesDirty = true;
}


void DebugRenderSysImpl::DrawArrow(const DirectX::SimpleMath::Vector3& p0, const DirectX::SimpleMath::Vector3& p1,
	const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Vector3& n)
{
	DrawLine(p0, p1, color);

	auto a = Vector3::Lerp(p0, p1, 0.85f);

	auto diff = p1 - p0;
	auto side = Vector3::Cross(n, diff) * 0.05f;

	DrawLine(a + side, p1, color);
	DrawLine(a - side, p1, color);
}


void DebugRenderSysImpl::DrawPoint(const DirectX::SimpleMath::Vector3& pos, const float& size)
{
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x + size, pos.y, pos.z, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x - size, pos.y, pos.z, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y + size, pos.z, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y - size, pos.z, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y, pos.z + size, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y, pos.z - size, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)
		});

	isPrimitivesDirty = true;
}


void DebugRenderSysImpl::DrawCircle(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density)
{
	double angleStep = DirectX::XM_PI * 2 / density;

	for (int i = 0; i < density; i++)
	{
		auto point0X = radius * cos(angleStep * i);
		auto point0Y = radius * sin(angleStep * i);

		auto point1X = radius * cos(angleStep * (i + 1));
		auto point1Y = radius * sin(angleStep * (i + 1));

		auto p0 = Vector3::Transform(Vector3(static_cast<float>(point0X), static_cast<float>(point0Y), 0), transform);
		auto p1 = Vector3::Transform(Vector3(static_cast<float>(point1X), static_cast<float>(point1Y), 0), transform);

		DrawLine(p0, p1, color);
	}
}


void DebugRenderSysImpl::DrawSphere(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density)
{
	DrawCircle(radius, color, transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationX(DirectX::XM_PIDIV2) * transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationY(DirectX::XM_PIDIV2) * transform, density);
}

void DebugRenderSysImpl::DrawPlane(const DirectX::SimpleMath::Vector4& p, const DirectX::SimpleMath::Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross)
{
	auto dir = Vector3(p.x, p.y, p.z);
	if (dir.Length() == 0.0f) return;
	dir.Normalize();
	
	auto up = Vector3(0, 0, 1);
	auto right = Vector3::Cross(dir, up);
	if(right.Length() < 0.01f) {
		up = Vector3(0, 1, 0);
		right = Vector3::Cross(dir, up);
	}
	right.Normalize();

	up = Vector3::Cross(right, dir);
	
	auto pos = -dir * p.w;

	auto leftPoint	= pos - right * sizeWidth;
	auto rightPoint	= pos + right * sizeWidth;
	auto downPoint	= pos - up * sizeWidth;
	auto upPoint		= pos + up * sizeWidth;

	DrawLine(leftPoint + up * sizeWidth, rightPoint + up * sizeWidth, color);
	DrawLine(leftPoint - up * sizeWidth, rightPoint - up * sizeWidth, color);
	DrawLine(downPoint - right * sizeWidth, upPoint - right * sizeWidth, color);
	DrawLine(downPoint + right * sizeWidth, upPoint + right * sizeWidth, color);
	
	
	if(drawCenterCross) {
		DrawLine(leftPoint, rightPoint, color);
		DrawLine(downPoint, upPoint, color);
	}
	
	DrawPoint(pos, 0.5f);
	DrawArrow(pos, pos + dir*sizeNormal, color, right);
}


void DebugRenderSysImpl::DrawFrustrum(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	const auto corners = MathHelper::GetFrustumCornersWorldSpace(view, proj);

	//for (int i = 0; i < corners.size(); ++i) {
	//	DrawPoint(corners[i].ToVec3(), 2.0f * (i+1));
	//}

	auto invView = view.Invert();
	DrawPoint(invView.Translation(), 1.0f);

	DrawLine(corners[0].ToVec3(), corners[1].ToVec3(), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2].ToVec3(), corners[3].ToVec3(), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[4].ToVec3(), corners[5].ToVec3(), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[6].ToVec3(), corners[7].ToVec3(), Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	DrawLine(corners[0].ToVec3(), corners[2].ToVec3(), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1].ToVec3(), corners[3].ToVec3(), Vector4(0.0f, 0.5f, 0.0f, 1.0f));
	DrawLine(corners[4].ToVec3(), corners[6].ToVec3(), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[5].ToVec3(), corners[7].ToVec3(), Vector4(0.0f, 0.5f, 0.0f, 1.0f));

	DrawLine(corners[0].ToVec3(), corners[4].ToVec3(), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	DrawLine(corners[1].ToVec3(), corners[5].ToVec3(), Vector4(0.5f, 0.0f, 0.0f, 1.0f));
	DrawLine(corners[2].ToVec3(), corners[6].ToVec3(), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	DrawLine(corners[3].ToVec3(), corners[7].ToVec3(), Vector4(0.5f, 0.0f, 0.0f, 1.0f));
}


void DebugRenderSysImpl::DrawTextureOnScreen(ID3D11ShaderResourceView* tex, int x, int y, int width, int height, int zOrder)
{
	if (quads.size() >= QuadMaxDrawCount) return;

	QuadInfo quad = {};
	quad.Srv = tex;
	quad.TransformMat = Matrix::CreateScale(static_cast<float>(width), static_cast<float>(height), 1.0f)
						* Matrix::CreateTranslation(static_cast<float>(x), static_cast<float>(y), static_cast<float>(zOrder));
	
	quads.emplace_back(quad);
}


