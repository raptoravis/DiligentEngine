#pragma once

#include "BasicMath.h"

inline float SIGN(float x) {
	return (x >= 0.0f) ? +1.0f : -1.0f;
}

inline float NORM(float a, float b, float c, float d) {
	return sqrt(a * a + b * b + c * c + d * d);
}

inline Diligent::Quaternion mRot2Quat(const Diligent::float4x4& m) {
	float r11 = m._11;
	float r12 = m._12;
	float r13 = m._13;
	float r21 = m._21;
	float r22 = m._22;
	float r23 = m._23;
	float r31 = m._31;
	float r32 = m._32;
	float r33 = m._33;
	float q0 = (r11 + r22 + r33 + 1.0f) / 4.0f;
	float q1 = (r11 - r22 - r33 + 1.0f) / 4.0f;
	float q2 = (-r11 + r22 - r33 + 1.0f) / 4.0f;
	float q3 = (-r11 - r22 + r33 + 1.0f) / 4.0f;
	if (q0 < 0.0f) {
		q0 = 0.0f;
	}
	if (q1 < 0.0f) {
		q1 = 0.0f;
	}
	if (q2 < 0.0f) {
		q2 = 0.0f;
	}
	if (q3 < 0.0f) {
		q3 = 0.0f;
	}
	q0 = sqrt(q0);
	q1 = sqrt(q1);
	q2 = sqrt(q2);
	q3 = sqrt(q3);
	if (q0 >= q1 && q0 >= q2 && q0 >= q3) {
		q0 *= +1.0f;
		q1 *= SIGN(r32 - r23);
		q2 *= SIGN(r13 - r31);
		q3 *= SIGN(r21 - r12);
	}
	else if (q1 >= q0 && q1 >= q2 && q1 >= q3) {
		q0 *= SIGN(r32 - r23);
		q1 *= +1.0f;
		q2 *= SIGN(r21 + r12);
		q3 *= SIGN(r13 + r31);
	}
	else if (q2 >= q0 && q2 >= q1 && q2 >= q3) {
		q0 *= SIGN(r13 - r31);
		q1 *= SIGN(r21 + r12);
		q2 *= +1.0f;
		q3 *= SIGN(r32 + r23);
	}
	else if (q3 >= q0 && q3 >= q1 && q3 >= q2) {
		q0 *= SIGN(r21 - r12);
		q1 *= SIGN(r31 + r13);
		q2 *= SIGN(r32 + r23);
		q3 *= +1.0f;
	}
	else {
		printf("coding error\n");
	}
	float r = NORM(q0, q1, q2, q3);
	q0 /= r;
	q1 /= r;
	q2 /= r;
	q3 /= r;

	// quaternion = [w, x, y, z]'
	Diligent::Quaternion res = { q1, q2, q3, q0 };

	return res;
}

inline Diligent::Quaternion calculateRotation(const Diligent::float4x4& m) {
	float r11 = m._11;
	float r12 = m._12;
	float r13 = m._13;
	float r21 = m._21;
	float r22 = m._22;
	float r23 = m._23;
	float r31 = m._31;
	float r32 = m._32;
	float r33 = m._33;

	float qw, qx, qy, qz;

	float trace = r11 + r22 + r33; // I removed + 1.0f; see discussion with Ethan
	if (trace > 0) {// I changed M_EPSILON to 0
		float s = 0.5f / sqrtf(trace + 1.0f);
		qw = 0.25f / s;
		qx = (r32 - r23) * s;
		qy = (r13 - r31) * s;
		qz = (r21 - r12) * s;
	}
	else {
		if (r11 > r22 && r11 > r33) {
			float s = 2.0f * sqrtf(1.0f + r11 - r22 - r33);
			qw = (r32 - r23) / s;
			qx = 0.25f * s;
			qy = (r12 + r21) / s;
			qz = (r13 + r31) / s;
		}
		else if (r22 > r33) {
			float s = 2.0f * sqrtf(1.0f + r22 - r11 - r33);
			qw = (r13 - r31) / s;
			qx = (r12 + r21) / s;
			qy = 0.25f * s;
			qz = (r23 + r32) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + r33 - r11 - r22);
			qw = (r21 - r12) / s;
			qx = (r13 + r31) / s;
			qy = (r23 + r32) / s;
			qz = 0.25f * s;
		}
	}

	Diligent::Quaternion res = { qx, qy, qz, qw };

	return res;
}
