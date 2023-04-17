#include "Model.h"

Model::~Model()
{
	
}

void Model::LoadModel(string path, ID3D11Device* pd3dDevice)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		OutputDebugStringA(importer.GetErrorString());
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene, pd3dDevice);
}

void Model::SetVertexBoneDataToDefault(Vertex& vertex)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		vertex.BoneIDs[i] = -1;
		vertex.BoneWeights[i] = 0.0f;
	}
}


void Model::ProcessNode(aiNode* node, const aiScene* scene, ID3D11Device* pd3dDevice)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, pd3dDevice));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, pd3dDevice);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, ID3D11Device* pd3dDevice)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		SetVertexBoneDataToDefault(vertex);

		XMFLOAT3 vert;
		vert.x = mesh->mVertices[i].x;
		vert.y = mesh->mVertices[i].y;
		vert.z = mesh->mVertices[i].z;
		vertex.Position = vert;

		XMFLOAT3 norm;
		norm.x = mesh->mNormals[i].x;
		norm.y = mesh->mNormals[i].y;
		norm.z = mesh->mNormals[i].z;
		vertex.Normals = norm;

		if (mesh->mTextureCoords[0])
		{
			XMFLOAT2 tex;
			tex.x = mesh->mTextureCoords[0][i].x;
			tex.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = tex;
		}
		else
		{
			vertex.texCoords = { 0.0f, 0.0f };
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	ExtractBoneWeightForVertices(vertices, mesh, scene);

	return Mesh(vertices, indices, textures, pd3dDevice);
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		if (vertex.BoneIDs[i] < 0)
		{
			vertex.BoneWeights[i] = weight;
			vertex.BoneIDs[i] = boneID;
			break;
		}
	}
}

XMFLOAT4X4 Model::ConvertMatrixToDirectXFormat(aiMatrix4x4 from)
{
	XMFLOAT4X4 myMatrixFloat;

	myMatrixFloat.m[0][0] = from.a1;
	myMatrixFloat.m[0][1] = from.a2;
	myMatrixFloat.m[0][2] = from.a3;
	myMatrixFloat.m[0][3] = from.a4;

	myMatrixFloat.m[1][0] = from.b1;
	myMatrixFloat.m[1][1] = from.b2;
	myMatrixFloat.m[1][2] = from.b3;
	myMatrixFloat.m[1][3] = from.b4;

	myMatrixFloat.m[2][0] = from.c1;
	myMatrixFloat.m[2][1] = from.c2;
	myMatrixFloat.m[2][2] = from.c3;
	myMatrixFloat.m[2][3] = from.c4;

	myMatrixFloat.m[3][0] = from.d1;
	myMatrixFloat.m[3][1] = from.d2;
	myMatrixFloat.m[3][2] = from.d3;
	myMatrixFloat.m[3][3] = from.d4;

	return myMatrixFloat;
}


void Model::ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{

	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.boneID = boneCounter;
			//newBoneInfo.offset = ConvertMatrixToDirectXFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
			newBoneInfo.offset = ConvertMatrixToDirectXFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
			boneInfoMap[boneName] = newBoneInfo;
			boneID = boneCounter;
			boneCounter++;
		}
		else
		{
			boneID = boneInfoMap[boneName].boneID;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

void Model::Draw(ID3D11DeviceContext* deviceContext)
{
	for (int i = 0; i < meshes.size(); ++i)
	{
		meshes[i].Draw(deviceContext);
	}
}
