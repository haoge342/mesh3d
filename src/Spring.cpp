#include "Spring.h"
#include <cmath>

Vector3 SubstractVector3(const Vector3& a, const Vector3& b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vector3 MultiplyVector3(float l, const Vector3& v) {
	return { l * v.x, l * v.y, l * v.z };
}

Spring::Spring(Particle* a, Particle* b, float k): 
	pA(a), 
	pB(b), 
	stiffness(k), 
	initialDiff(*b - *a)
{};

void Spring::ApplySpringForce(float dampingFactor) {
	// HOOK LAW. F = -k * (x - x0)
	const Vector3 currParticalsDiff = *pB - *pA; // p1 is the origin
	const Vector3 delta = SubstractVector3(currParticalsDiff, initialDiff);
	const Vector3 springForce = MultiplyVector3(-stiffness, delta);


	Vector3 velocityDiff = SubstractVector3(pB->velocity, pA->velocity);

	float validatedDampingFactor = 0; // Lower value for stability

	if (validatedDampingFactor < 0.0f || validatedDampingFactor >= 1.0f ) {
		validatedDampingFactor = 0.0f;
	}
	else {
		validatedDampingFactor = dampingFactor;
	}

	// Project velocity difference onto spring direction
	float dampingAmount = (velocityDiff.x * delta.x +
		velocityDiff.y * delta.y +
		velocityDiff.z * delta.z) /
		(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1e-6f); // Avoid div by zero

	Vector3 dampingForce = MultiplyVector3(-validatedDampingFactor * dampingAmount, delta);

	// **Total Force = Spring Force + Damping**
	Vector3 totalForce = { springForce.x + dampingForce.x,
						   springForce.y + dampingForce.y,
						   springForce.z + dampingForce.z };

	pA->ApplyForce(MultiplyVector3(-1, totalForce)); // p1 is origin
    pB->ApplyForce(totalForce); // push/pull side pB
}
