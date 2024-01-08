#pragma once
#include "math/Matrix4x4.h"

struct TransformationMatrix {
	Matrix4x4 matWorld;
	Matrix4x4 WorldInverseTrasnpose;
};