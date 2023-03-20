#pragma once

#include <d3d11_1.h>
#include <directxmath.h>

#include "Quaternion.h"

#include <vector>
#include <memory>

using namespace  std;
using namespace DirectX;

struct BoneProperties
{
	XMFLOAT3 bonePosition;
	XMFLOAT3 boneRotation;
	XMFLOAT3 boneScale;

	XMMATRIX boneMatrix;
};

class Bone
{
public:
	Bone() {}
	~Bone();

	BoneProperties boneProps;

	vector<unique_ptr<Bone>> boneChildren;
	Bone* parent = nullptr;
};

class Skeleton
{
	Skeleton();
	~Skeleton();
};