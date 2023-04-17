#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

class Model
{
public:
	Model(const char* path, ID3D11Device* pd3dDevice)
	{
		LoadModel(path, pd3dDevice);
	}

	void Draw(ID3D11DeviceContext* deviceContext);
private:
	void LoadModel(string path, ID3D11Device* pd3dDevice);
	void ProcessNode(aiNode* node, const aiScene* scene, ID3D11Device* pd3dDevice);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, ID3D11Device* pd3dDevice);
	//vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, string typeName);

	const aiScene* pScene;
	Assimp::Importer Importer;

	vector<Mesh> meshes;
	string directory;
};

