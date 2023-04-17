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
		XMVECTOR newPos = XMLoadFloat3(&pos);
		data.Position = newPos;

		data.timeStamp = timeStamp;
		m_positions.push_back(data);
	}

	m_numRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_numRotations; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation data;

		XMFLOAT3 pos = XMFLOAT3(aiOrientation.x, aiOrientation.y, aiOrientation.z);
		XMVECTOR newPos = XMLoadFloat3(&pos);
		data.orientation = newPos;
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
		XMVECTOR newPos = XMLoadFloat3(&pos);
		data.scale = newPos;

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

XMMATRIX Bone::InterpolatePosition(float animationTime)
{
	if (1 == m_numPositions)
	{
		XMFLOAT3 pos = XMFLOAT3();
		XMStoreFloat3(&pos, m_positions[0].Position);
		return XMMatrixTranslation(pos.x, pos.y, pos.z);
	}

	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_positions[p0Index].timeStamp, m_positions[p1Index].timeStamp, animationTime);

	XMVECTOR tempLerp = XMVectorLerp(m_positions[p0Index].Position, m_positions[p1Index].Position, scaleFactor);

	XMFLOAT3 pos = XMFLOAT3();
	XMStoreFloat3(&pos, tempLerp);

	return XMMatrixTranslation(pos.x, pos.y, pos.z);
}

XMMATRIX Bone::InterpolateRotation(float animationTime)
{
	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_rotations[p0Index].timeStamp, m_rotations[p1Index].timeStamp, animationTime);

	if (1 == m_numRotations)
	{
		XMVECTOR xmVec = XMQuaternionSlerp(m_rotations[0].orientation, m_rotations[0].orientation, scaleFactor);
		XMFLOAT4 quatFloat = XMFLOAT4();
		XMStoreFloat4(&quatFloat, xmVec);

		Quaternion mRotation = Quaternion(quatFloat.x, quatFloat.y, quatFloat.z, quatFloat.w);
		mRotation.normalise();

		XMMATRIX matrixRotation;
		CalculateTransformMatrixRowMajor(matrixRotation, XMFLOAT3(), mRotation);

		return matrixRotation;
	}

	XMVECTOR xmVec = XMQuaternionSlerp(m_rotations[p0Index].orientation, m_rotations[p1Index].orientation, scaleFactor);
	XMFLOAT4 quatFloat = XMFLOAT4();
	XMStoreFloat4(&quatFloat, xmVec);

	Quaternion mRotation = Quaternion(quatFloat.x, quatFloat.y, quatFloat.z, quatFloat.w);
	mRotation.normalise();

	XMMATRIX matrixRotation;
	CalculateTransformMatrixRowMajor(matrixRotation, XMFLOAT3(), mRotation);

	return matrixRotation;
}

XMMATRIX Bone::InterpolateScaling(float animationTime)
{
	if (1 == m_numScalings)
	{
		XMFLOAT3 pos = XMFLOAT3();
		XMStoreFloat3(&pos, m_scales[0].scale);
		return XMMatrixScaling(pos.x, pos.y, pos.z);
	}

	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_scales[p0Index].timeStamp,m_scales[p1Index].timeStamp, animationTime);

	XMVECTOR tempLerp = XMVectorLerp(m_scales[p0Index].scale, m_scales[p1Index].scale, scaleFactor);
	XMFLOAT3 pos = XMFLOAT3();
	XMStoreFloat3(&pos, tempLerp);

	return XMMatrixScaling(pos.x, pos.y, pos.z);
}


void Bone::Update(float animationTime)
{
	XMMATRIX translation = InterpolatePosition(animationTime);

	XMMATRIX rotation = InterpolateRotation(animationTime);

	XMMATRIX scale = InterpolateScaling(animationTime);

	XMStoreFloat4x4(&m_LocalTransform, translation * rotation * scale);
}