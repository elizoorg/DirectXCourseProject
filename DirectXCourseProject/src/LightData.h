#pragma once
#include "Mathtypes.h"

struct LightData {
	Vector4 LightPos;
	Vector4 LightColor;
	Vector4 AmbientSpecularRowType;
	Matrix T;
};