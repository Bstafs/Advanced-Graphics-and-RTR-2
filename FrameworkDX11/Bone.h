#pragma once
#include "Quaternion.h"

#include "Model.h"

struct KeyPosition
{
	XMFLOAT3 Position;
	float timeStamp;
};

struct KeyRotation
{
	XMFLOAT4 orientation;
	float timeStamp;
};

struct KeyScale
{
	XMFLOAT3 scale;
	float timeStamp;
};

class Bone
{
public:
	Bone(const std::string& name, int ID, const aiNodeAnim* channel);

	XMVECTOR InterpolatePosition(float animationTime);
	XMVECTOR InterpolateRotation(float animationTime);
	XMVECTOR InterpolateScaling(float animationTime);

	XMFLOAT4X4* GetLocalTransform() { return &m_LocalTransform; }
	std::string GetBoneName() const { return m_name; }
	int GetBoneID() { return m_ID; }

	int GetPositionIndex(float animationTime);
	int GetRotationIndex(float animationTime);
	int GetScaleIndex(float animationTime);

	void Update(float animationTime);

private:
	std::vector<KeyPosition> m_positions;
	std::vector<KeyRotation> m_rotations;
	std::vector<KeyScale> m_scales;

	int m_numPositions;
	int m_numRotations;
	int m_numScalings;

	XMFLOAT4X4 m_LocalTransform;
	std::string m_name;
	int m_ID;

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}
};

