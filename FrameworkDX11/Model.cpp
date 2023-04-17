#include "Model.h"

void Model::LoadModel(string path, ID3D11Device* pd3dDevice)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		OutputDebugStringA(importer.GetErrorString());
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene, pd3dDevice);
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

	return Mesh(vertices, indices, textures, pd3dDevice);
}

//vector<Texture> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type, string typeName)
//{
//	vector<Texture> textures;
//	for(int i=0; i < material->GetTextureCount(type); ++i)
//	{
//		aiString string;
//		material->GetTexture(type, i, &string);
//		Texture texture;
//	}
//}


void Model::Draw(ID3D11DeviceContext* deviceContext)
{
	for(int i = 0; i< meshes.size();++i)
	{
		meshes[i].Draw(deviceContext);
	}
}
