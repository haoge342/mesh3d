#include "Mesh.h"
#include "raylib.h"
#include <iostream>
#include <future>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

const float HEIGHT = 0.0f;

namespace mesh3d {
	std::string MeshTypeToString(MeshType type) {
		switch (type) {
		case MeshType::Regular: return "regular";
		case MeshType::Irregular: return "irregular";
		default: return "unknown";
		}
	}

	MeshType StringToMeshType(const std::string& type) {
		if (type == "regular") return MeshType::Regular;
		if (type == "irregular") return MeshType::Irregular;
		throw std::invalid_argument("Unknown mesh type: " + type);
	}

	// load particals for irregular mesh
	std::vector<Particle> LoadParticles(const std::string& filename, float pMass) {
		std::vector<Particle> particles;
		std::ifstream file(filename);
		std::string line;

		// Count number of lines first to pre-allocate
		size_t lineCount = 0;
		while (std::getline(file, line)) {
			lineCount++;
		}
		particles.reserve(lineCount);

		// Reset file to beginning
		file.clear();
		file.seekg(0);

		// skip the 1st title line
		std::getline(file, line);

		while (std::getline(file, line)) {
			std::istringstream iss(line);
			float x, y, z;
			std::string fixed;
			char comma;

			iss >> x >> comma >> y >> comma >> z >> comma >> fixed;

			particles.emplace_back(Vector3{ y, z, x }, fixed=="true", pMass);
		}

		std::cout << "Finished loading particles" << std::endl;
		return particles;
	}

	// load springs for irregular mesh
	std::vector<Spring> LoadSprings(const std::string& filename, std::vector<Particle>& particles) {
		std::vector<Spring> springs;
		std::ifstream file(filename);
		std::string line;
		const int SIZE = particles.size();

		// Count number of lines first to pre-allocate
		size_t lineCount = 0;
		while (std::getline(file, line)) {
			lineCount++;
		}
		springs.reserve(lineCount);

		// Reset file to beginning
		file.clear();
		file.seekg(0);

		// skip the 1st line
		std::getline(file, line);

		while (std::getline(file, line)) {
			std::istringstream iss(line);
			int idxA, idxB;
			float stiffness;
			char comma;
			iss >> idxA >> comma >> idxB >> comma >> stiffness;

			if (idxA < 0 || idxA >= SIZE || idxB < 0 || idxB >= SIZE) {
				std::cout << "Invalid arugment " << idxA << "," << idxB << std::endl;
				throw std::invalid_argument("Invalid spring indices: " + std::to_string(idxA) + ", " + std::to_string(idxB));
			}

			Particle* pA = &particles[idxA];
			Particle* pB = &particles[idxB];

			Spring newSpring(pA, pB, stiffness);
			springs.push_back(std::move(newSpring));
		}
		return springs;
	}

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
					else if (key == "airResistanceFactor") config.airResistanceFactor = std::stof(value);
					else if (key == "mesh_type") config.mesh_type = StringToMeshType(value);
				}
			}
		}
		return config;
	}

	void WriteConfig(const std::string& filename, const Config& config) {
		std::ofstream file(filename);
		file << "mesh_type=" << MeshTypeToString(config.mesh_type) << "\n";
		file << "width=" << config.width << "\n";
		file << "height=" << config.height << "\n";
		file << "spacing=" << config.spacing << "\n";
		file << "stiffness=" << config.stiffness << "\n";
		file << "particleMass=" << config.particleMass << "\n";
		file << "dampingFactor=" << config.dampingFactor << "\n";
		file << "airResistanceFactor=" << config.airResistanceFactor << "\n";
	}

	// regular mesh
	Mesh::Mesh(const Config& c) : width(c.width), height(c.height), springStiffness(c.stiffness), dampingFactor(c.dampingFactor), airResistanceFactor(c.airResistanceFactor) {
		const Vector3 ORIGIN = { (c.width - 1) * c.spacing / 2, HEIGHT, (c.height - 1) * c.spacing / 2 };

		particles.reserve(c.width * c.height);
		springs.reserve((c.width - 1) * c.height + (c.height - 1) * c.width);

		// Create particles
		for (int y = 0; y < c.height; y++) {
			for (int x = 0; x < c.width; x++) {
				bool fixed = (y == 0 || x == 0 || y == c.height - 1 || x == c.width - 1);  // Fix top row
				particles.emplace_back(Vector3{ x * c.spacing - ORIGIN.x, 0 - ORIGIN.y, y * c.spacing - ORIGIN.z }, fixed, c.particleMass);
			}
		}

		// Create springs
		for (int y = 0; y < c.height; y++) {
			for (int x = 0; x < c.width; x++) {
				int idx = y * c.width + x;
				if (x < c.width - 1) springs.emplace_back(&particles[idx], &particles[idx + 1], c.stiffness); // Horizontal
				if (y < c.height - 1) springs.emplace_back(&particles[idx], &particles[idx + c.width], c.stiffness); // Vertical
			}
		}
	};

	// irregular mesh
	Mesh::Mesh(const std::string& pFileName, const std::string& sFileName, const Config& c)
	{
		width=0; height = 0; springStiffness = c.stiffness; dampingFactor = c.dampingFactor; airResistanceFactor = c.airResistanceFactor;
		particles = LoadParticles(pFileName, c.particleMass);
		springs = LoadSprings(sFileName, particles);
	};

    bool Mesh::Update(float dt) {
        if (dt <= 0.0f) return true;

		for (auto& particle : particles) {
			particle.ApplyForce(Vector3{ 0, -9.8f, 0 });  // Gravity
			// air resistance
			particle.ApplyForce(Vector3{
				airResistanceFactor * particle.velocity.x * particle.velocity.x,
				airResistanceFactor * particle.velocity.y * particle.velocity.y,
				airResistanceFactor * particle.velocity.z * particle.velocity.z
				});
			particle.Update(dt);
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

	void Mesh::SetAirResistanceFactor(float arFactor) {
		airResistanceFactor = arFactor;
	}

    void Mesh::Draw() {

        for (auto& spring : springs) {
            DrawLine3D(spring.pA->position, spring.pB->position, BLUE);
        }

		for (auto& particle : particles) {
			const auto COLOR = particle.isFixed ? DARKGREEN : ORANGE;
			DrawSphere(particle.position, 0.1f, COLOR);
		}
    }
}
