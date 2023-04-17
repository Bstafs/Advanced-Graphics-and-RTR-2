#pragma once

#include <d3d11_1.h>
#include <directxmath.h>

#include <string>
#include <vector>

using namespace DirectX;
using namespace std;

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normals;
	XMFLOAT2 texCoords;
};

struct Texture
{
	unsigned int id;
	std::string type;
};

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> Textures, ID3D11Device* pd3dDevice);
	void Draw(ID3D11DeviceContext* deviceContext);

private:
	void SetupMesh(ID3D11Device* pd3dDevice);

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indicesBuffer;
	ID3D11SamplerState* samplerState;

};

