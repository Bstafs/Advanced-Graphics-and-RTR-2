#pragma once

#include "Animation.h"

class Animator
{
public:
	Animator(Animation* Animation);
	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* pAnimation);
	void CalculateBoneTransform(const AssimpNodeData* node, XMMATRIX parentTransform);
	void CalculateBlendedBoneTransform(Animation* pAnimationBase, const AssimpNodeData* node, Animation* pAnimationLayer, 
		const AssimpNodeData* nodeLayered, const float currentBaseTime, const float currentTimeLayered, 
		XMMATRIX parentTransform, const float blendFactor);

	void BlendAnimation(Animation* pBaseAnimation, Animation* pLayeredAnimation, float blendFactor, float deltaTime);

	std::vector<XMFLOAT4X4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}
private:
	std::vector<XMFLOAT4X4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;
};

