#include "Mesh.h"
#include "raylib.h"
#include <iostream>
#include <future>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

const bool IS_MULTITHREADED = true;

const float HEIGHT = 0.0f;

namespace mesh3d {
	Config LoadMeshConfig(const std::string& filename) {
		Config config;
		std::ifstream file(filename);
        std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
            std::string key;
			if (std::getline(iss, key, '=')) {
				std::string value;
				if (std::getline(iss, value)) {
					if (key == "width") config.width = std::stoi(value);
					else if (key == "height") config.height = std::stoi(value);
					else if (key == "spacing") config.spacing = std::stof(value);
					else if (key == "stiffness") config.stiffness = std::stof(value);
					else if (key == "particleMass") config.particleMass = std::stof(value);
					else if (key == "dampingFactor") config.dampingFactor = std::stof(value);
				}
			}
		}
		return config;
	}

    void WriteConfig(const std::string& filename, const Config& config) {
        std::ofstream file(filename);
		file << "width=" << config.width << "\n";
        file << "height=" << config.height << "\n";
        file << "spacing=" << config.spacing << "\n";
        file << "stiffness=" << config.stiffness << "\n";
        file << "particleMass=" << config.particleMass << "\n";
		file << "dampingFactor=" << config.dampingFactor << "\n";
    }

    Mesh::Mesh(int w, int h, float spacing, float stiff, float pMass, float dFactor) : width(w), height(h), springStiffness(stiff), dampingFactor(dFactor) {
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

    bool Mesh::Update(float dt) {
        if (dt <= 0.0f) return true;

        if (IS_MULTITHREADED) {
            // apply forces to particles in parallel
            std::vector<std::future<void>> futures;
            for (auto& particle : particles) {
                futures.push_back(std::async(std::launch::async, [&particle, dt]() {
                    particle.ApplyForce(Vector3{ 0, -9.8f, 0 });  // Gravity

                    // air resistance
                    const float AIR_RESISTANCE = -0.001f;
                    particle.ApplyForce(Vector3{
                        AIR_RESISTANCE * particle.velocity.x * particle.velocity.x,
                        AIR_RESISTANCE * particle.velocity.y * particle.velocity.y,
                        AIR_RESISTANCE * particle.velocity.z * particle.velocity.z
                        });
                    particle.Update(dt);
                    }));
            }
        }
        else {
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
        }

        for (auto& spring : springs) {
			spring.stiffness = springStiffness; // listen on the stiffness changes
            spring.ApplySpringForce(dampingFactor); // hook law
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

	void Mesh::SetStiffness(float stiff) {
		springStiffness = stiff;
	}

	void Mesh::SetDampingFactor(float dFactor) {
		dampingFactor = dFactor;
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
