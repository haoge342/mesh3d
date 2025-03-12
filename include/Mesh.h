#pragma once
#include <vector>
#include "Particle.h"
#include "Spring.h"

namespace mesh3d{
    class Mesh {
    public:
        int width, height;
        std::vector<Particle> particles;
        std::vector<Spring> springs;
		float springStiffness = 10.0f;

        Mesh(int w, int h, float spacing, float stiff, float pMass);
        bool Update(float dt, float currStiffness, float dampingFactor);
        void Draw();
    };
} // namespace mesh3d
