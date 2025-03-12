#include "Mesh.h"
#include "raylib.h"
#include <iostream>

const float HEIGHT = 0.0f;

namespace mesh3d {
    Mesh::Mesh(int w, int h, float spacing, float stiff, float pMass) : width(w), height(h), springStiffness(stiff) {
        const Vector3 ORIGIN = { (w - 1) * spacing / 2, HEIGHT, (h - 1) * spacing / 2 };

		particles.reserve(w * h);
		springs.reserve((w - 1) * h + (h - 1) * w);

        // Create particles
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                bool fixed = (y == 0 || x == 0 || y == h - 1 || x == w - 1);  // Fix top row
                particles.emplace_back(Vector3{ x * spacing - ORIGIN.x, 0 - ORIGIN.y, y * spacing - ORIGIN.z }, fixed, pMass);
            }
        }

        // Create springs
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int idx = y * w + x;
                if (x < w - 1) springs.emplace_back(&particles[idx], &particles[idx + 1], stiff); // Horizontal
                if (y < h - 1) springs.emplace_back(&particles[idx], &particles[idx + w], stiff); // Vertical
            }
        }
    }

    bool Mesh::Update(float dt, float currStiffness, float userDampingFactor = 10.0f) {
        if (dt <= 0.0f) return true;

        for (auto& particle : particles) {
            particle.ApplyForce(Vector3{ 0, -9.8f, 0 });  // Gravity

			// air resistance
			const float AIR_RESISTANCE = -0.001f;
			particle.ApplyForce(Vector3{ 
                AIR_RESISTANCE * particle.velocity.x * particle.velocity.x, 
                AIR_RESISTANCE * particle.velocity.y * particle.velocity.y,
                AIR_RESISTANCE * particle.velocity.z * particle.velocity.z
               });
            particle.Update(dt);
        }

        for (auto& spring : springs) {
			spring.stiffness = currStiffness; // listen on the stiffness changes
            spring.ApplySpringForce(userDampingFactor); // hook law
        }

        // debug
        for (auto& particle : particles) {
            if (std::_Is_nan(particle.position.x) || std::_Is_nan(particle.position.y) || std::_Is_nan(particle.position.z)) {
                std::cerr << "Particle position invalid: " << particle.position.x << ", " << particle.position.y << ", " << particle.position.z << std::endl;
				return false;
            }
        }

		return true;
    }

    void Mesh::Draw() {

        for (auto& spring : springs) {
            DrawLine3D(spring.pA->position, spring.pB->position, BLUE);
        }

		for (auto& particle : particles) {
			DrawSphere(particle.position, 0.1f, RED);
		}
    }
}
