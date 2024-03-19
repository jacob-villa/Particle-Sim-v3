#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <ctime>
#include <cmath>
#include <random>
#include <future>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include "stb_image.h"

using namespace std;

enum Mode {
	DEVELOPER,
	EXPLORER
};

static GLFWwindow* window = nullptr;
float PI = 3.14159265359;

ImVec4 wallColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
ImVec4 particleColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

Mode currentMode = DEVELOPER; // Default mode

bool isSpriteImageAvailable = false;

//adjust sprite size here
float spriteWidth = 5.0f; 
float spriteHeight = 5.0f;

float zoomFactor = 1.0f;

ImVec2 focusPoint = ImVec2(640, 360);

static bool LoadTexture(const char* path, GLuint& textureID) {
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // ignore format warning

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		return true;
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return false;
	}
}

static float getDistance(float x1, float y1, float x2, float y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

static float pointLineDistance(float px, float py, float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
	float closestX = x1 + t * dx;
	float closestY = y1 + t * dy;

	return sqrt((closestX - px) * (closestX - px) + (closestY - py) * (closestY - py));
}

class Particle {
public:
	float x, y;
	float angle;
	float velocity;

	Particle(float x, float y, float angle, float velocity)
		: x(x), y(y), angle(angle), velocity(velocity) {}

	void UpdatePosition(float deltaTime) {
		float radians = angle * PI / 180.0;

		float dx = cos(radians) * velocity * deltaTime;
		float dy = sin(radians) * velocity * deltaTime;

		float newX = x + dx;
		float newY = y + dy;

		// Threshold for collision detection
		float threshold = velocity > 500 ? 10.0f : 3.0f;

		bool collisionDetected = false;

		x = newX;
		y = newY;

		if (x < 0) {
			x = 0;
			angle = 180 - angle;
		}
		else if (x > 1280) {
			x = 1280;
			angle = 180 - angle;
		}

		if (y < 0) {
			y = 0;
			angle = -angle;
		}
		else if (y > 720) {
			y = 720;
			angle = -angle;
		}
	}
};

std::vector<Particle> particles;

class Sprite {
public:
	float x, y;
	float speed;
	GLuint textureID;

	Sprite(float x, float y, float speed, GLuint textureID) : x(x), y(y), speed(speed), textureID(textureID) {}

	void Move(float dx, float dy) {
		x += dx;
		y -= dy;

		// sprite border check
		if (x < 0) x = 0;
		if (x > 1280) x = 1280;
		if (y < 0) y = 0;
		if (y > 720) y = 720;

		//std::cout << "Sprite position: (" << x << ", " << y << ")" << std::endl; // for debugging
	}
};

Sprite* explorerSprite = nullptr; // Global pointer to the explorer sprite

static void SpawnRandomParticle() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> disX(0, 1280);
	std::uniform_real_distribution<> disY(0, 720);
	std::uniform_real_distribution<> disAngle(0, 360);
	std::uniform_real_distribution<> disVelocity(10, 300);

	float x, y;
	bool validPosition = false;
	x = disX(gen);
	y = disY(gen);
	float angle = disAngle(gen);
	float velocity = disVelocity(gen);

	particles.emplace_back(x, y, angle, velocity);
}

static void GLFWErrorCallback(int error, const char* description) {
	std::cout << "GLFW Error " << description << " code: " << error << std::endl;
}

static float clampSpriteDimension(float dimension, float min, float max) {
	if (dimension > max) {
		return max;
	}
	else if (dimension < min) {
		return min;
	}
	else {
		return dimension;
	}
}

static void DrawElements() {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	if (currentMode == EXPLORER && explorerSprite) {
		ImVec2 newPos = ImVec2(
			(explorerSprite->x - focusPoint.x) * zoomFactor + focusPoint.x,
			(explorerSprite->y - focusPoint.y) * zoomFactor + focusPoint.y
		);

		newPos.x = std::max(0.0f, std::min(1280.0f, newPos.x));
		newPos.y = std::max(0.0f, std::min(720.0f, newPos.y));

		newPos.y = 720 - newPos.y;

		//float scaleFactor = std::min(19.0f / spriteWidth, 33.0f / spriteHeight);

		spriteWidth = clampSpriteDimension(spriteWidth, 3.0f, 15.0f);
		spriteHeight = clampSpriteDimension(spriteHeight, 3.0f, 15.0f);

		float scaledSpriteWidth = spriteWidth * zoomFactor;
		float scaledSpriteHeight = spriteHeight * zoomFactor;

		if (isSpriteImageAvailable) {
			draw_list->AddImage(reinterpret_cast<void*>(explorerSprite->textureID),
				ImVec2(newPos.x - scaledSpriteWidth / 2, newPos.y - scaledSpriteHeight / 2),
				ImVec2(newPos.x + scaledSpriteWidth / 2, newPos.y + scaledSpriteHeight / 2),
				ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
		}
		else {
			draw_list->AddCircleFilled(newPos, scaledSpriteWidth / 2, ImColor(0, 255, 255, 255));
		}

		for (const auto& particle : particles) {
			ImVec2 newPos = ImVec2(
				(particle.x - focusPoint.x) * zoomFactor + focusPoint.x,
				(particle.y - focusPoint.y) * zoomFactor + focusPoint.y
			);

			//newPos.x = std::max(0.0f, std::min(1280.0f, newPos.x));
			//newPos.y = std::max(0.0f, std::min(720.0f, newPos.y));
			newPos.y = 720 - newPos.y;

			if (newPos.x >= 0 && newPos.x <= 1280 && newPos.y >= 0 && newPos.y <= 720) {
				draw_list->AddCircleFilled(newPos, scaledSpriteWidth / 2, ImColor(particleColor));
			}
		}
	}
	else {
		for (const auto& particle : particles) {
			ImVec2 newPos = ImVec2(
				(particle.x - focusPoint.x) * zoomFactor + focusPoint.x,
				(particle.y - focusPoint.y) * zoomFactor + focusPoint.y
			);

			//newPos.x = std::max(0.0f, std::min(1280.0f, newPos.x));
			//newPos.y = std::max(0.0f, std::min(720.0f, newPos.y));
			newPos.y = 720 - newPos.y;

			if (newPos.x >= 0 && newPos.x <= 1280 && newPos.y >= 0 && newPos.y <= 720) {
				draw_list->AddCircleFilled(newPos, 1.5f, ImColor(particleColor));
			}
		}
	}
	
}

static void UpdateParticlesRange(std::vector<Particle>::iterator begin, std::vector<Particle>::iterator end, float deltaTime) {
	for (auto &it = begin; it != end; ++it) {
		it->UpdatePosition(deltaTime);
	}
}

int main(int argc, char *argv) {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		std::cin.get();
	}

	glfwSetErrorCallback(GLFWErrorCallback);

	window = glfwCreateWindow(1380, 820, "Particle Sim", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMaximizeWindow(window);
	glfwMakeContextCurrent(window);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	float newParticleX = 0.0f;
	float newParticleY = 0.0f;
	float newParticleAngle = 0.0f;
	float newParticleVelocity = 0.0f;

	bool showErrorPopup = false;

	char numParticlesStr[16] = "";
	int numParticles = 0;
	int particleVariationType = 0; //  0: Varying X and Y,  1: Varying Angle,  2: Varying Velocity
	float startX = 0.0f, endX = 0.0f;
	float startY = 0.0f, endY = 0.0f;
	float startAngle = 0.0f, endAngle = 0.0f;
	float startVelocity = 0.0f, endVelocity = 0.0f;

	double frameTime = 0.0; // Time since the last frame
	double targetFrameTime = 1.0 / 60.0; // Target time per frame (60 FPS)
	double updateInterval = 0.5; // Interval for updating particles (0.5 seconds)
	double lastUpdateTime = 0.0; // Last time particles were updated
	double lastFPSUpdateTime = 0.0; // Last time the framerate was updated

	const double targetFPS = 60.0;
	const std::chrono::duration<double> targetFrameDuration = std::chrono::duration<double>(1.0 / targetFPS);

	std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();

	const double timeStep = 1.0 / targetFPS; // Time step for updates
	double accumulator = 0.0; // Accumulates elapsed time

	double currentFramerate = 0.0;
	double lastUIUpdateTime = 0.0;

	GLuint explorerTexture;
	isSpriteImageAvailable = LoadTexture("squareman.jpg", explorerTexture); //change sprite image here 

	char imagePath[256] = "";

	std::string loadImageMessage = "";

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		frameTime = currentTime - lastUpdateTime;

		std::chrono::steady_clock::time_point currentTimeForDelta = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsedTime = currentTimeForDelta - prevTime;
		prevTime = currentTimeForDelta;

		accumulator += frameTime;

		if (elapsedTime > std::chrono::seconds(1)) {
			elapsedTime = std::chrono::duration<double>(1.0 / targetFPS);
		}

		double deltaTime = elapsedTime.count();

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

		ImGui::SetNextWindowSize(ImVec2(1380, 820), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::Begin("Black Panel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		DrawElements();

		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(1380, 200), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(0, 820), ImGuiCond_Once);

		ImGui::Begin("Color Pickers", nullptr, ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));

		ImGui::ColorEdit3("Particle Color", (float*)&particleColor);

		ImGui::Dummy(ImVec2(0, 10));
		if (currentMode == DEVELOPER) {
			if (ImGui::Button("Reset Particles")) {
				particles.clear();
			}
		}
		else {
			ImGui::Text("Buttons disabled in Explorer mode");
		}
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Text("Current FPS: %.f", currentFramerate);
		ImGui::Text("Number of Particles: %d", particles.size());

		ImGui::PopStyleColor(4);

		ImGui::Dummy(ImVec2(0, 10));

		if (ImGui::GetIO().Fonts->Fonts.Size > 0) {
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

			ImGui::Text("Particle Physics Simulator");
			ImGui::Text("STDISCM - S12");
			ImGui::Text("Joshua Ejercito, Ryan Go, Anton Morana, Jacob Villa");

			ImGui::PopFont();
		}
		else {
			ImGui::Text("Particle Physics Simulator");
			ImGui::Text("STDISCM - S12");
			ImGui::Text("Joshua Ejercito, Ryan Go, Naton Morana, Jacob Villa");
		}

		ImGui::End();

		ImGui::PopStyleColor();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowSizeConstraints(ImVec2(640, 1080), ImVec2(640, 1080));
		ImGui::SetNextWindowPos(ImVec2(1380, 0), ImGuiCond_Always);
		ImGui::Begin("Button Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::Dummy(ImVec2(0, 25));
		ImGui::Text("Add Individual Particle");
		ImGui::Dummy(ImVec2(0, 10));

		ImGui::PushStyleColor(ImGuiCol_FrameBg, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentMode == EXPLORER ? ImVec4(0.0f, 0.0f, 1.0f, 1.0f) : ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
		ImGui::PushItemWidth(175.0f);

		ImGui::InputFloat("X Coordinate", &newParticleX);
		ImGui::InputFloat("Y Coordinate", &newParticleY);
		ImGui::InputFloat("Angle (degrees)", &newParticleAngle);
		ImGui::InputFloat("Velocity (pixels/sec)", &newParticleVelocity);

		ImGui::Dummy(ImVec2(0, 10));
		if (currentMode == DEVELOPER) {
			if (ImGui::Button("Add Particle")) {

				//std::cout << "New particle velocity: " << newParticleVelocity << std::endl; // Debug output

				if (newParticleX >= 0 && newParticleX <= 1280 &&
					newParticleY >= 0 && newParticleY <= 720 &&
					newParticleAngle >= 0.0 && newParticleAngle <= 360.0) {
					particles.emplace_back(newParticleX, newParticleY, newParticleAngle, newParticleVelocity);
				}
				else {
					showErrorPopup = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Spawn Random Particle")) {
				SpawnRandomParticle();
			}

			if (showErrorPopup) {
				ImGui::OpenPopup("Invalid Input");
				if (ImGui::BeginPopupModal("Invalid Input", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("The input values for X and Y coordinates must be within the dimensions of the black panel (0-1280,   0-720).\nThe angle must be between   0 and   360 degrees.");
					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
						showErrorPopup = false;
					}
					ImGui::EndPopup();
				}
			}
		}
		else {
			ImGui::Text("Buttons disabled in Explorer mode");
		}

		ImGui::Dummy(ImVec2(0, 55));
		ImGui::Text("--------------------------------------------------------------------------------------------------------------------");


		ImGui::Dummy(ImVec2(0, 55));
		ImGui::Text("Add Batch Particle");
		ImGui::Dummy(ImVec2(0, 10));

		ImGui::InputText("Number of Particles", numParticlesStr, sizeof(numParticlesStr));
		numParticles = atoi(numParticlesStr);

		const char* particleVariationTypes[] = { "Varying X and Y", "Varying Angle", "Varying Velocity" };
		ImGui::Combo("Particle Variation Type", &particleVariationType, particleVariationTypes, IM_ARRAYSIZE(particleVariationTypes));
		ImGui::InputFloat("Start X", &startX);
		ImGui::InputFloat("Start Y", &startY);
		ImGui::InputFloat("End X", &endX);
		ImGui::InputFloat("End Y", &endY);
		ImGui::InputFloat("Start Angle", &startAngle);
		ImGui::InputFloat("End Angle", &endAngle);
		ImGui::InputFloat("Start Velocity", &startVelocity);
		ImGui::InputFloat("End Velocity", &endVelocity);

		ImGui::Dummy(ImVec2(0, 10));
		if (currentMode == DEVELOPER) {
			if (ImGui::Button("Add Batch Particles")) {
				float dX = (endX - startX) / (numParticles - 1);
				float dY = (endY - startY) / (numParticles - 1);
				float dAngle = (endAngle - startAngle) / (numParticles - 1);
				float dVelocity = (endVelocity - startVelocity) / (numParticles - 1);

				for (int i = 0; i < numParticles; ++i) {
					float x = startX + i * dX;
					float y = startY + i * dY;
					float angle = startAngle + i * dAngle;
					float velocity = startVelocity + i * dVelocity;

					switch (particleVariationType) {
					case 0: // Varying X and Y
						particles.emplace_back(x, y, startAngle, startVelocity);
						break;
					case 1: // Varying Angle
						angle = fmod(angle, 360.0f);
						particles.emplace_back(startX, startY, angle, startVelocity);
						break;
					case 2: // Varying Velocity
						particles.emplace_back(startX, startY, startAngle, velocity);
						break;
					}
					//std::cout << "Particle position: (" << x << ", " << y << ")" << std::endl;
				}
			}
		}
		else {
			ImGui::Text("Buttons disabled in Explorer mode");
		}
		ImGui::Dummy(ImVec2(0, 55));
		ImGui::Text("--------------------------------------------------------------------------------------------------------------------");
		if (ImGui::Button("Developer mode")) {
			std::cout << "Developer mode" << std::endl;
			currentMode = DEVELOPER;
			zoomFactor = 1.0f;
		}
		if (ImGui::Button("Explorer mode")) {
			std::cout << "Explorer mode" << std::endl;
			currentMode = EXPLORER;
			//zoomFactor = 3.0f;
			float scaleFactorWidth = 1280.0f / 19.0f;
			float scaleFactorHeight = 720.0f / 33.0f;
			zoomFactor = std::min(scaleFactorWidth, scaleFactorHeight);
			if (currentMode == EXPLORER) {
				if (!explorerSprite) {
					explorerSprite = new Sprite(640, 360, 10.0f, explorerTexture); // initial position and speed adjust here n lng
				}
				//explorerSprite->UpdatePosition(deltaTime);
			}
		}

		ImGui::InputFloat("Sprite Width (Max: 15, Min: 3)", &spriteWidth);
		ImGui::InputFloat("Sprite Height (Max: 15, Min: 3)", &spriteHeight);
		//std::cout << "Sprite Width: " << spriteWidth << ", Sprite Height: " << spriteHeight << std::endl;
		ImGui::InputText("Image Path", imagePath, sizeof(imagePath));

		if (ImGui::Button("Load Image")) {
			GLuint newTextureID;
			if (LoadTexture(imagePath, newTextureID)) {
				if (explorerSprite) {
					glDeleteTextures(1, &explorerSprite->textureID);
				}
				explorerSprite->textureID = newTextureID;
				isSpriteImageAvailable = true;
				loadImageMessage = "Sprite successfully loaded.";
			}
			else {
				std::cout << "Failed to load image: " << imagePath << std::endl;
				isSpriteImageAvailable = false;
				loadImageMessage = "Failed to load image. Make sure you are using the right directory and image type.";
			}
		}

		if (!loadImageMessage.empty()) {
			ImGui::Text("%s", loadImageMessage.c_str());
		}

		if (currentMode == EXPLORER && explorerSprite) {
			float moveSpeed = explorerSprite->speed * deltaTime;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				explorerSprite->Move(0, -moveSpeed); // Move up
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				explorerSprite->Move(-moveSpeed, 0); // Move left
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				explorerSprite->Move(0, moveSpeed); // Move down
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				explorerSprite->Move(moveSpeed, 0); // Move right
			}
		}

		ImGui::PopStyleColor(4);

		ImGui::End();

		ImGui::PopStyleVar();

		while (accumulator >= timeStep) {
			std::vector<std::future<void>> futures;
			size_t numParticles = particles.size();
			size_t numThreads = std::thread::hardware_concurrency();
			size_t chunkSize = numParticles / numThreads;

			for (size_t i = 0; i < numThreads; ++i) {
				auto startIter = particles.begin() + i * chunkSize;
				auto endIter = (i == numThreads - 1) ? particles.end() : startIter + chunkSize;
				futures.push_back(std::async(std::launch::async, UpdateParticlesRange, startIter, endIter, timeStep));
			}

			futures.push_back(std::async(std::launch::async, DrawElements));

			for (auto& future : futures) {
				future.wait();
			}
			accumulator -= timeStep;
		}

		if (frameTime >= targetFrameTime) {
			lastUpdateTime = currentTime;
		}

		if (currentTime - lastFPSUpdateTime >= updateInterval) {
			currentFramerate = 1.0 / frameTime;
			std::cout << "Framerate: " << currentFramerate << " FPS" << std::endl;
			lastFPSUpdateTime = currentTime;
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
		glfwSwapInterval(1); // VSync setting here
	}

	if (explorerSprite) {
		glDeleteTextures(1, &explorerSprite->textureID);
		delete explorerSprite;
		explorerSprite = nullptr;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();

	return 0;
}