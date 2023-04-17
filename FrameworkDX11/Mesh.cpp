#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned> indices, vector<Texture> Textures, ID3D11Device* pd3dDevice)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = Textures;

	SetupMesh(pd3dDevice);
}

void Mesh::SetupMesh(ID3D11Device* pd3dDevice)
{
	// Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices.data();

	pd3dDevice->CreateBuffer(&bd, &InitData, &vertexBuffer);

	// Index Buffer
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(DWORD) * indices.size();
	bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData2;
	ZeroMemory(&InitData2, sizeof(InitData2));
	InitData2.pSysMem = indices.data();
	pd3dDevice->CreateBuffer(&bd2, &InitData2, &indicesBuffer);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pd3dDevice->CreateSamplerState(&sampDesc, &samplerState);

}

void Mesh::Draw(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indicesBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetSamplers(0, 1, &samplerState);

	deviceContext->DrawIndexed(indices.size() , 0, 0);
}
