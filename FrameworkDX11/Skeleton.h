#pragma once

#include <d3d11_1.h>
#include <directxmath.h>
#include <map>

#include "Quaternion.h"

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace  std;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normals;
	XMFLOAT2 texCoords;
};

struct BoneProperties
{
	unsigned int IDs[4];
	float Weights[4];

	void AddBone(unsigned int BoneID, float Weight)
	{
		for (unsigned i = 0; i < 4; ++i) // 4 bones
		{
			if(Weights[i] == 0)
			{
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}
	}
};

struct BoneInfo
{
	XMMATRIX boneTransform;
	aiMatrix4x4 boneOffset;
};

struct MeshInfo
{
	unsigned int BaseVertex;
	unsigned int BaseIndex;
	unsigned int NumIndices;
	unsigned int MaterialIndex;
};

class Skeleton
{
public:
	Skeleton();
	~Skeleton();

	void LoadModel(string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	void LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<BoneProperties>& Bones);
private:

	const aiScene* pScene;
	Assimp::Importer Importer;

	string directory;

	unsigned int numberOfBones;
	std::map<std::string, unsigned int> boneMapping;
	vector<BoneInfo> boneInfo;

	vector<MeshInfo> Mesh;

};