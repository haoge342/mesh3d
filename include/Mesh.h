#pragma once
#include <vector>
#include "Particle.h"
#include "Spring.h"
#include <string>
#include <unordered_map>

namespace mesh3d{
	enum class MeshType { Regular, Irregular };

	std::string MeshTypeToString(MeshType type);
	MeshType StringToMeshType(const std::string& type);

    struct Config {
        int width = 10;
        int height = 10;
        float spacing = 1.0f;
        float stiffness = 10.0f;
        float particleMass = 1.0f;
		float dampingFactor = 0.1f;
		float airResistanceFactor = 0.001f;
		MeshType mesh_type = MeshType::Regular;
    };

	Config LoadMeshConfig(const std::string& filename);
	void WriteConfig(const std::string& filename, const Config& config);
    
    class Mesh {
    private:
        int width, height;
        std::vector<Particle> particles;
        std::vector<Spring> springs;
		float springStiffness = 20.0f;
		float dampingFactor = 10.0f;
		float airResistanceFactor = 0.001f;
    public:
        Mesh(const Config& config);
		Mesh(const std::vector<Particle>& particles, const std::vector<Spring>& springs, const Config& config);
        bool Update(float dt);
        void Draw();
        void SetStiffness(float stiff);
		void SetDampingFactor(float dFactor);
		void SetAirResistanceFactor(float arFactor);
    };
} // namespace mesh3d
