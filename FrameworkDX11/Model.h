#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <map>

struct BoneInfo
{
	int boneID;
	XMFLOAT4X4 offset;
};

class Model
{
public:
	Model(const char* path, ID3D11Device* pd3dDevice)
	{
		LoadModel(path, pd3dDevice);
	}
	~Model();

	void Draw(ID3D11DeviceContext* deviceContext);

	void LoadModel(string path, ID3D11Device* pd3dDevice);
	void ProcessNode(aiNode* node, const aiScene* scene, ID3D11Device* pd3dDevice);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, ID3D11Device* pd3dDevice);

	// Bone Extraction
	std::map<std::string, BoneInfo> boneInfoMap;
	int boneCounter = 0;

	std::map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

	XMFLOAT4X4 ConvertMatrixToDirectXFormat(aiMatrix4x4 from);
	void SetVertexBoneDataToDefault(Vertex& vertex);
	void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
	void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

	// Model Import
	const aiScene* pScene;
	Assimp::Importer Importer;

	vector<Mesh> meshes;
	string directory;
};

