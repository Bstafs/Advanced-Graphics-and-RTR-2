#include "Skeleton.h"
Skeleton::Skeleton()
{
	
}
Skeleton::~Skeleton()
{
	
}

void Skeleton::LoadModel(string path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Skeleton::ProcessNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

void Skeleton::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<BoneProperties> Bones;

	for(unsigned int i =0;i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		XMFLOAT3 vert;
		vert.x = mesh->mVertices[i].x;
		vert.y = mesh->mVertices[i].y;
		vert.z = mesh->mVertices[i].z;
		vertex.position = vert;

		XMFLOAT3 norm;
		norm.x = mesh->mNormals[i].x;
		norm.y = mesh->mNormals[i].y;
		norm.z = mesh->mNormals[i].z;
		vertex.normals = norm;

		if(mesh->mTextureCoords[0])
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

	if(mesh->HasBones())
	{
		//LoadBones(index, mesh, Bones);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

}

void Skeleton::LoadBones(unsigned MeshIndex, const aiMesh* pMesh, std::vector<BoneProperties>& Bones)
{
	for (int i = 0; i < pMesh->mNumBones; i++) 
	{
		unsigned int BoneIndex = 0;

		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (boneMapping.find(BoneName) == boneMapping.end())
		{
			BoneIndex = numberOfBones;

			numberOfBones++;

			BoneInfo bi;
			boneInfo.push_back(bi);
		}
		else
		{
			BoneIndex = boneMapping[BoneName];
		}

		boneMapping[BoneName] = BoneIndex;

		boneInfo[BoneIndex].boneOffset = pMesh->mBones[i]->mOffsetMatrix;

		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) 
		{
			unsigned int VertexID = Mesh[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;

			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;

			Bones[VertexID].AddBone(BoneIndex, Weight);
		}
	}
}

