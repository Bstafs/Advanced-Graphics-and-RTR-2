#pragma once

#include "Animation.h"

class Animator
{
public:
	Animator(Animation* Animation);
	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* pAnimation);
	void CalculateBoneTransform(const AssimpNodeData* node, XMMATRIX parentTransform);

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

