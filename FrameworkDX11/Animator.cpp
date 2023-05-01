#include "Animator.h"

Animator::Animator(Animation* Animation)
{
	m_CurrentTime = 0.0;
	m_CurrentAnimation = Animation;

	m_FinalBoneMatrices.reserve(100);

	for (int i = 0; i < 100; i++)
		m_FinalBoneMatrices.push_back(XMFLOAT4X4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		));
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
	{
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}

void Animator::BlendAnimation(Animation* pBaseAnimation, Animation* pLayeredAnimation, float blendFactor, float deltaTime)
{
	// Speed multipliers to correctly transition from one animation to another
	float a = 1.0f;
	float b = pBaseAnimation->GetDuration() / pLayeredAnimation->GetDuration();
	const float animSpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor;

	a = pLayeredAnimation->GetDuration() / pBaseAnimation->GetDuration();
	b = 1.0f;
	const float animSpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor;

	// Current time of each animation, "scaled" by the above speed multiplier variables
	static float currentTimeBase = 0.0f;
	currentTimeBase += pBaseAnimation->GetTicksPerSecond() * deltaTime * animSpeedMultiplierUp;
	currentTimeBase = fmod(currentTimeBase, pBaseAnimation->GetDuration());

	static float currentTimeLayered = 0.0f;
	currentTimeLayered += pLayeredAnimation->GetTicksPerSecond() * deltaTime * animSpeedMultiplierDown;
	currentTimeLayered = fmod(currentTimeLayered, pLayeredAnimation->GetDuration());

	CalculateBlendedBoneTransform(pBaseAnimation, &pBaseAnimation->GetRootNode(), pLayeredAnimation, &pLayeredAnimation->GetRootNode(), currentTimeBase, currentTimeLayered, XMMatrixIdentity(), blendFactor);
}

void Animator::CalculateBlendedBoneTransform(Animation* pAnimationBase, const AssimpNodeData* node, Animation* pAnimationLayer, const AssimpNodeData* nodeLayered, const float currentBaseTime, const float currentTimeLayered, XMMATRIX parentTransform, const float blendFactor)
{
	const std::string& nodeName = node->name;

	XMMATRIX nodeTransform = XMLoadFloat4x4(&node->transformation);

	Bone* Bone = pAnimationBase->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(currentBaseTime);
		nodeTransform = XMLoadFloat4x4(Bone->GetLocalTransform());
	}

	XMMATRIX layeredNodeTransform = XMLoadFloat4x4(&nodeLayered->transformation);
	Bone = pAnimationLayer->FindBone(nodeName);
	if (Bone)
	{
		Bone->Update(currentTimeLayered);
		layeredNodeTransform = XMLoadFloat4x4(Bone->GetLocalTransform());
	}

	// Blending Matrices
	XMVECTOR rot0 = XMQuaternionRotationMatrix(nodeTransform);
	XMVECTOR rot1 = XMQuaternionRotationMatrix(layeredNodeTransform);
	XMVECTOR finalRot = XMQuaternionNormalize(XMQuaternionSlerp(rot0, rot1, blendFactor));
	XMMATRIX blendedMat = XMMatrixRotationQuaternion(finalRot);

	blendedMat.r[3] = (1.0f - blendFactor) * nodeTransform.r[3] + layeredNodeTransform.r[3] * blendFactor;
	blendedMat.r[2] = (1.0f - blendFactor) * nodeTransform.r[2] + layeredNodeTransform.r[2] * blendFactor;
	blendedMat.r[1] = (1.0f - blendFactor) * nodeTransform.r[1] + layeredNodeTransform.r[1] * blendFactor;
	blendedMat.r[0] = (1.0f - blendFactor) * nodeTransform.r[0] + layeredNodeTransform.r[0] * blendFactor;


	XMMATRIX globalTransformation = blendedMat * parentTransform;

	auto boneInfoMap = pAnimationBase->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].boneID;
		XMMATRIX offset = XMLoadFloat4x4(&boneInfoMap[nodeName].offset);
		XMStoreFloat4x4(&m_FinalBoneMatrices[index], offset * globalTransformation);
	}

	for (int i = 0; i < node->childrenCount; i++)
	{
		CalculateBlendedBoneTransform(pAnimationBase, &node->children[i], pAnimationLayer, &nodeLayered->children[i], currentBaseTime, currentTimeLayered, globalTransformation, blendFactor);
	}
}
