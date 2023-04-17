#include "Animation.h"
Animation::Animation(const std::string& animationPath, Model* model)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	m_Duration = animation->mDuration;
	m_TicksPerSecond = animation->mTicksPerSecond;
	ReadHeirarchyData(m_RootNode, scene->mRootNode);
	ReadMissingBones(animation, model);
}

Animation::~Animation()
{

}

Bone* Animation::FindBone(const std::string& name)
{
	auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
		[&](const Bone& Bone)
		{
			return Bone.GetBoneName() == name;
		}
	);
	if (iter == m_Bones.end()) return nullptr;
	else return &(*iter);
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model* model)
{
	int size = animation->mNumChannels;

	auto& boneInfoMap = model->GetBoneInfoMap();//getting m_BoneInfoMap from Model class
	int& boneCount = model->GetBoneCount(); //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			boneInfoMap[boneName].boneID = boneCount;
			boneCount++;
		}
		m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].boneID, channel));
	}

	m_BoneInfoMap = boneInfoMap;
}

XMFLOAT4X4 Animation::ConvertMatrixToDirectXFormat(aiMatrix4x4 from)
{
	XMFLOAT4X4 myMatrixFloat;

	myMatrixFloat.m[0][0] = from.a1;
	myMatrixFloat.m[1][0] = from.a2;
	myMatrixFloat.m[2][0] = from.a3;
	myMatrixFloat.m[3][0] = from.a4;

	myMatrixFloat.m[0][1] = from.b1;
	myMatrixFloat.m[1][1] = from.b2;
	myMatrixFloat.m[2][1] = from.b3;
	myMatrixFloat.m[3][1] = from.b4;

	myMatrixFloat.m[0][2] = from.c1;
	myMatrixFloat.m[1][2] = from.c2;
	myMatrixFloat.m[2][2] = from.c3;
	myMatrixFloat.m[3][2] = from.c4;

	myMatrixFloat.m[0][3] = from.d1;
	myMatrixFloat.m[1][3] = from.d2;
	myMatrixFloat.m[2][3] = from.d3;
	myMatrixFloat.m[3][3] = from.d4;

	return myMatrixFloat;
}

void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
{
	assert(src);

	dest.name = src->mName.data;
	dest.transformation = ConvertMatrixToDirectXFormat(src->mTransformation);
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		ReadHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}


