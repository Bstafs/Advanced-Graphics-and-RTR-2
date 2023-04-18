#include "Bone.h"

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) : m_name(name), m_ID(ID)
{
	m_numPositions = channel->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_numPositions; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		XMFLOAT3 pos = XMFLOAT3(aiPosition.x, aiPosition.y, aiPosition.z);
		data.Position = pos;
		data.timeStamp = timeStamp;
		m_positions.push_back(data);
	}

	m_numRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_numRotations; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation data;
		XMFLOAT4 pos = XMFLOAT4(aiOrientation.x, aiOrientation.y, aiOrientation.z, aiOrientation.w);
		data.orientation = pos;
		data.timeStamp = timeStamp;
		m_rotations.push_back(data);
	}

	m_numScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_numScalings; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale data;
		XMFLOAT3 pos = XMFLOAT3(scale.x, scale.y, scale.z);
		data.scale = pos;

		data.timeStamp = timeStamp;
		m_scales.push_back(data);
	}
}

int Bone::GetPositionIndex(float animationTime)
{
	for (int index = 0; index < m_numPositions - 1; ++index)
	{
		if (animationTime < m_positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Bone::GetRotationIndex(float animationTime)
{
	for (int index = 0; index < m_numRotations - 1; ++index)
	{
		if (animationTime < m_rotations[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Bone::GetScaleIndex(float animationTime)
{
	for (int index = 0; index < m_numScalings - 1; ++index)
	{
		if (animationTime < m_scales[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

XMVECTOR Bone::InterpolatePosition(float animationTime)
{
	if (m_numPositions == 1)
	{
		return XMLoadFloat3(&m_positions[0].Position);
	}

	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_positions[p0Index].timeStamp, m_positions[p1Index].timeStamp, animationTime);

	XMVECTOR vec1 = XMLoadFloat3(&m_positions[p0Index].Position);
	XMVECTOR vec2 = XMLoadFloat3(&m_positions[p1Index].Position);

	XMVECTOR tempLerp = XMVectorLerp(vec1, vec2, scaleFactor);

	return tempLerp;
}

XMVECTOR Bone::InterpolateRotation(float animationTime)
{
	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_rotations[p0Index].timeStamp, m_rotations[p1Index].timeStamp, animationTime);

	if (m_numRotations == 1)
	{
		XMVector4Normalize(XMLoadFloat4(&m_rotations[0].orientation));
	}

	XMVECTOR orien01 = XMLoadFloat4(&m_rotations[p0Index].orientation);
	XMVECTOR orien02 = XMLoadFloat4(&m_rotations[p1Index].orientation);

	return XMVector4Normalize(XMQuaternionSlerp(orien01, orien02, scaleFactor));
}

XMVECTOR Bone::InterpolateScaling(float animationTime)
{
	if (m_numScalings == 1)
	{
		return XMLoadFloat3(&m_scales[0].scale);
	}

	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_scales[p0Index].timeStamp,m_scales[p1Index].timeStamp, animationTime);

	XMVECTOR vec1 = XMLoadFloat3(&m_scales[p0Index].scale);
	XMVECTOR vec2 = XMLoadFloat3(&m_scales[p1Index].scale);

	XMVECTOR tempLerp = XMVectorLerp(vec1, vec2, scaleFactor);

	return tempLerp;
}


void Bone::Update(float animationTime)
{
	XMVECTOR translation = InterpolatePosition(animationTime);

	XMVECTOR rotation = InterpolateRotation(animationTime);
	XMVECTOR rotationOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR scale = InterpolateScaling(animationTime);

	XMStoreFloat4x4(&m_LocalTransform, XMMatrixAffineTransformation(scale, rotationOrigin, rotation, translation));
}