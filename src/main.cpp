#include "raylib.h"
#include "Mesh.h"
#include <string>
#include <iostream>

const float ANIMATION_SPEED_STEP = 0.05f;
const int GRID_SIZE = 31;
const float MASS = 0.1f;

void DrawCoordSystem(void);

std::string formateFloat(float f) {
	char buffer[20];
	std::sprintf(buffer, "%.2f", f);
	return std::string(buffer);
}

int main() {
    const int screenWidth = 1024;
    const int screenHeight = 768;
	float animationSpeed = 1.0f;
	float userStiffness = 20.0f;
	float userDampingFactor = 0.1f;
	std::string msg = "Press r to restart simulation";

	bool isRunning = false;

    InitWindow(screenWidth, screenHeight, "3D Cloth Simulation");
	SetMousePosition(screenWidth / 2, screenHeight / 2);

    Camera camera = { { 15.0f, 15.0f, 15.0f }, { 0.0f, -2.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 60.0f, CAMERA_PERSPECTIVE };

    mesh3d::Mesh cloth(GRID_SIZE, GRID_SIZE, 0.5f, userStiffness, MASS);  // 10x10 cloth

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) { isRunning = !isRunning; };

		// increase or decrease animation speed
		if (IsKeyPressed(KEY_UP)) { animationSpeed += ANIMATION_SPEED_STEP; }
		if (IsKeyPressed(KEY_DOWN)) {
			if (animationSpeed > ANIMATION_SPEED_STEP) animationSpeed -= ANIMATION_SPEED_STEP;
		}

		// increase or decrease stiffness
		if (IsKeyPressed(KEY_M)) { userStiffness += 1.0f; }
		if (IsKeyPressed(KEY_N)) {
			if (userStiffness > 1.0f) userStiffness -= 1.0f;
		}
		
		// increase or decrease damping factor
		if (IsKeyPressed(KEY_P)) { userDampingFactor += 0.1f; }
		if (IsKeyPressed(KEY_O)) {
			if (userDampingFactor > 0.1f) userDampingFactor -= 0.1f;
		}

		// let mouse control camera
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);

		// if window moves, then stop simulation
		if (IsWindowResized() || !IsWindowFocused) { isRunning = false; }


		// restart simulation
		if (IsKeyPressed(KEY_R)) { 
			cloth = mesh3d::Mesh(GRID_SIZE, GRID_SIZE, 1.0f, userStiffness, MASS); 
			msg = "Reseted!";
		}

		if (isRunning) {
			//UpdateCamera(&camera, CAMERA_FREE);
			float dt = GetFrameTime() * animationSpeed;
			if (!cloth.Update(dt, userStiffness, userDampingFactor)) {
				isRunning = false;
				msg = "Simulation failed!";
			}
		}

		//#region Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
		DrawCoordSystem();
        cloth.Draw();
        EndMode3D();

        // Text
		DrawText(isRunning ? "Running..." : "Paused (Press Enter/Space to continue)", 20, 20, 20, BLACK);
		DrawText(msg.c_str(), 20, 40, 20, BLACK);
		DrawText(("FPS: " + formateFloat(GetFPS())).c_str(), 20, 60, 20, BLACK);
		DrawText(("Animation Speed: " + formateFloat(animationSpeed) + " | Up & Down").c_str(), 20, 80, 20, BLACK);
		DrawText(("Stiffness: " + formateFloat(userStiffness) + " | N -> M").c_str(), 20, 100, 20, BLACK);
		DrawText(("Damping Factor: " + formateFloat(userDampingFactor) + " | O -> P").c_str(), 20, 120, 20, BLACK);

		EndDrawing();
		//#endregion
    }

    CloseWindow();
    return 0;
}

void DrawCoordSystem() {
    const float  AXIS_LENGTH = 10.0f;

	DrawGrid(10, 1.0f);

	DrawLine3D({ 0, 0, 0 }, { AXIS_LENGTH, 0, 0 }, MAROON); // x
    DrawLine3D({ 0, 0, 0 }, { -AXIS_LENGTH, 0, 0 }, LIGHTGRAY); // -x
	DrawLine3D({ 0, 0, 0 }, { 0, AXIS_LENGTH, 0 }, DARKGREEN); // y
	DrawLine3D({ 0, 0, 0 }, { 0, -AXIS_LENGTH, 0 }, LIGHTGRAY); // -y
	DrawLine3D({ 0, 0, 0 }, { 0, 0, AXIS_LENGTH }, DARKBLUE); // z
	DrawLine3D({ 0, 0, 0 }, { 0, 0, -AXIS_LENGTH }, LIGHTGRAY); // -z
}
