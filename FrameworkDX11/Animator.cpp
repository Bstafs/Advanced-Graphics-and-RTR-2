#include "Animator.h"

Animator::Animator(Animation* Animation)
{
    m_CurrentTime = 0.0;
    m_CurrentAnimation = Animation;

    m_FinalBoneMatrices.reserve(52);

    for (int i = 0; i < 52; i++)
        m_FinalBoneMatrices.push_back(XMFLOAT4X4());
}

void Animator::UpdateAnimation(float dt)
{
	m_DeltaTime = dt;
	if (m_CurrentAnimation)
	{
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), XMMatrixIdentity());
	}
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}


void Animator::CalculateBoneTransform(const AssimpNodeData* node, XMMATRIX parentTransform)
{
	std::string nodeName = node->name;
	XMMATRIX nodeTransform = XMLoadFloat4x4(&node->transformation);

	Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(m_CurrentTime);
		nodeTransform = XMLoadFloat4x4(Bone->GetLocalTransform());
	}

	XMMATRIX globalTransformation = nodeTransform * parentTransform;

	auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].boneID;
		XMMATRIX offset = XMLoadFloat4x4(&boneInfoMap[nodeName].offset);
		XMStoreFloat4x4(&m_FinalBoneMatrices[index], offset * globalTransformation);
	}

	for (int i = 0; i < node->childrenCount; i++)
		CalculateBoneTransform(&node->children[i], globalTransformation);
}