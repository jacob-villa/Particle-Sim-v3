#define _CRT_SECURE_NO_WARNINGS
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

#include <boost/asio.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include "stb_image.h"
#include "json.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

static boost::asio::io_context io_context;

short port = 69696;

enum Mode {
	DEVELOPER,
	EXPLORER
};

static std::vector<boost::shared_ptr<tcp::socket>> clients;

static GLFWwindow* window = nullptr;
float PI = 3.14159265359;

ImVec4 wallColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
ImVec4 particleColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

Mode currentMode = DEVELOPER; // Default mode

bool isSpriteImageAvailable = false;

bool keyW = false, keyA = false, keyS = false, keyD = false;

//adjust sprite size here
float spriteWidth = 5.0f; 
float spriteHeight = 5.0f;

float zoomFactor = 1.0f;

ImVec2 focusPoint = ImVec2(640, 360);

class Semaphore {
private:
	std::mutex mutex_;
	std::condition_variable condition_;
	size_t count_;

public:
	Semaphore(size_t count = 0) : count_(count) {}

	void notify() {
		std::unique_lock<std::mutex> lock(mutex_);
		++count_;
		condition_.notify_one();
	}

	void wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ == 0) {
			condition_.wait(lock);
		}
		--count_;
	}

	bool try_wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (count_ > 0) {
			--count_;
			return true;
		}
		return false;
	}
};

Semaphore clientSemaphore = Semaphore(0);

std::string getTimestamp() {
	auto now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	std::tm* timeInfo = std::localtime(&currentTime);

	char buffer[29];

	std::strftime(buffer, 29, "%Y-%m-%d %H:%M:%S", timeInfo);


	return std::string(buffer);
}

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

	void UpdatePosition(ImGuiIO& io) {
		float frameRate = io.Framerate;
		float radians = angle * PI / 180.0;

		float dx = cos(radians) * velocity / frameRate;
		float dy = sin(radians) * velocity / frameRate;

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

		// Ensure particles stay within the 1280x720 black panel
		x = std::max(0.0f, std::min(1280.0f, x));
		y = std::max(0.0f, std::min(720.0f, y));
	}

	json toJSON() const {
		json j;
		j["x"] = x;
		j["y"] = y;
		j["angle"] = angle;
		j["velocity"] = velocity;
		return j;
	}

	static Particle fromJSON(const json& j) {
		return Particle(j["x"].get<float>(),
			j["y"].get<float>(),
			j["angle"].get<float>(),
			j["velocity"].get<float>());
	}

	bool equals(Particle& other) {
		return x == other.x && y == other.y && angle == other.angle && velocity == other.velocity;
	}
};

int clientCtr = 0;
std::vector<Particle> particles;


class Sprite {
public:
	int clientID;
	float x, y;
	float speed;
	GLuint textureID;

	Sprite(int clientID, float x, float y, float speed, GLuint textureID) :clientID(clientID), x(x),y(y), speed(speed), textureID(textureID) {}

	void Move(float dx, float dy) {
		x += dx;
		y -= dy;

		if (x < 0) x = 0;
		if (x > 1280) x = 1280;
		if (y < 0) y = 0;
		if (y > 720) y = 720;

		x = std::max(0.0f, std::min(1280.0f, x));
		y = std::max(0.0f, std::min(720.0f, y));

		//std::cout << "Sprite position: (" << x << ", " << y << ")" << std::endl; // for debugging
	}

	json toJSON() const {
		json j;
    
		j["clientID"] = clientID;
		j["x"] = x;
		j["y"] = y;
		j["speed"] = speed;
		//dk where texture ID goes, but i assume here too in case
		return j;
	}

	static Sprite fromJSON(const json& j) {
		GLuint textureID;

		return Sprite(j["clientID"].get<int>(),
			j["x"].get<float>(),
			j["y"].get<float>(),
			j["speed"].get<float>(),
			textureID);
	}

	void setPos(float newX, float newY) {
		x = newX;
		y = newY;
	}
};

std::vector<Sprite> clientSprites;

//Sprite* explorerSprite = nullptr; // Global pointer to the explorer sprite

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
	// json j;
	// j["x"] = x;
	// j["y"] = y;
	// j["angle"] = angle;
	// j["velocity"]= velocity;

	// std::string serializedParticles = j.dump();
	// sendParticlestoClient(j);
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

	ImVec2 translation = ImVec2(0, 0);
	/*if (currentMode == EXPLORER && explorerSprite) {
		translation.x = 640 - explorerSprite->x;
		translation.y = 360 - explorerSprite->y;
	}*/

	float scaledBorderWidth = 1280 * zoomFactor;
	float scaledBorderHeight = 720 * zoomFactor;

	ImVec2 newBorderPos1 = ImVec2(
		(translation.x - focusPoint.x) * zoomFactor + focusPoint.x,
		(translation.y - focusPoint.y) * zoomFactor + focusPoint.y
	);
	ImVec2 newBorderPos2 = ImVec2(
		(1280 + translation.x - focusPoint.x) * zoomFactor + focusPoint.x,
		(720 + translation.y - focusPoint.y) * zoomFactor + focusPoint.y
	);

	newBorderPos1.y = 720 - newBorderPos1.y;
	newBorderPos2.y = 720 - newBorderPos2.y;

	newBorderPos1.x += 50;
	newBorderPos1.y += 50;

	newBorderPos2.x += 50;
	newBorderPos2.y += 50;

	ImColor purple = ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f);
	draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(1380, 820), purple);
	draw_list->AddRectFilled(newBorderPos1, newBorderPos2, ImColor(0, 0, 0, 255));

	float baseParticleRadius = 1.5f;
	for (const auto& particle : particles) {
		ImVec2 newPos = ImVec2(
			(particle.x + translation.x - focusPoint.x) * zoomFactor + focusPoint.x,
			(particle.y + translation.y - focusPoint.y) * zoomFactor + focusPoint.y
		);

		newPos.y = 720 - newPos.y;

		if (newPos.x >= 0 && newPos.x <= 1280 && newPos.y >= 0 && newPos.y <= 720) {
			newPos.x += 50;
			newPos.y += 50;

			float scaledParticleRadius = baseParticleRadius * zoomFactor;

			draw_list->AddCircleFilled(newPos, scaledParticleRadius, ImColor(particleColor));
		}
	}
	for (const auto& sprite : clientSprites) {
		ImVec2 spritePos = ImVec2(sprite.x, sprite.y);
		// Commented out the adjustments for focus point and zoom factor
		spritePos.x = (spritePos.x - focusPoint.x) * zoomFactor + focusPoint.x;
		spritePos.y = (spritePos.y - focusPoint.y) * zoomFactor + focusPoint.y;
		// Flip Y coordinate because ImGui's coordinate system starts from the top
		spritePos.y = 720 - spritePos.y;

		// Adjust sprite position for the border and offset
		spritePos.x += 50;
		spritePos.y += 50;

		// Calculate scaled sprite dimensions
		float scaledSpriteWidth = spriteWidth * zoomFactor; // This line might need adjustment if zoomFactor is not 1
		float scaledSpriteHeight = spriteHeight * zoomFactor; // This line might need adjustment if zoomFactor is not 1

		// Check if the sprite is within the visible area
		if (spritePos.x >= 0 && spritePos.x <= 1380 && spritePos.y >= 0 && spritePos.y <= 830) {
			//std::cout << "Passed the condition" << std::endl;
			// Draw the sprite
			//if (isSpriteImageAvailable) {
			//	// Draw the sprite using its texture
			//	draw_list->AddImage(reinterpret_cast<void*>(sprite.textureID),
			//		ImVec2(spritePos.x - scaledSpriteWidth / 2, spritePos.y - scaledSpriteHeight / 2),
			//		ImVec2(spritePos.x + scaledSpriteWidth / 2, spritePos.y + scaledSpriteHeight / 2),
			//		ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
			//		std::cout << "Image Added" << std::endl;
			//}
			//else {
				// Draw a placeholder circle if the sprite image is not available
			draw_list->AddCircleFilled(spritePos, scaledSpriteWidth / 2, ImColor(0, 255, 255, 255));
			/*}*/
		}
	}
}
static void UpdateParticlesRange(std::vector<Particle>::iterator begin, std::vector<Particle>::iterator end, ImGuiIO& io) {
	for (auto& it = begin; it != end; ++it) {
		it->UpdatePosition(io);
	}
}

json particleToJSON(Particle particle) {
	json j;
	j["x"] = particle.x;
	j["y"] = particle.y;
	j["angle"] = particle.angle;
	j["velocity"] = particle.velocity;

	return j;
}

std::string serializeSprites(const std::vector<Sprite>& sprites) {
	std::vector<int> xValues;
	std::vector<int> yValues;
	std::vector<int> speedValues;
	std::vector<int> clientIDValues;

	for (const auto& sprite : sprites) {
		xValues.push_back(sprite.x);
		yValues.push_back(sprite.y);
		speedValues.push_back(sprite.speed);
		clientIDValues.push_back(sprite.clientID);
	}

	json j;
	j["x"] = xValues;
	j["y"] = yValues;
	j["speed"] = speedValues;
	j["clientID"] = clientIDValues;
	j["message_type"] = "Sprites";

	std::string serializedSprites = j.dump();

	// Insert | at end of message
	serializedSprites.push_back('|');

	return serializedSprites;

}

std::string serializeParticles(const std::vector<Particle>& particles) {
	std::vector<int> xValues;
	std::vector<int> yValues;
	std::vector<int> angleValues;
	std::vector<int> velocityValues;

	for (const auto& particle : particles) {
		xValues.push_back(particle.x);
		yValues.push_back(particle.y);
		angleValues.push_back(particle.angle);
		velocityValues.push_back(particle.velocity);
	}

	json j;
	j["x"] = xValues;
	j["y"] = yValues;
	j["angle"] = angleValues;
	j["velocity"] = velocityValues;
	j["message_type"] = "Particles";

	std::string serializedParticles = j.dump();

	// Insert | at end of message
	serializedParticles.push_back('|');

	return serializedParticles;
}

void sendParticles() {
	std::cout << "I'm in sendParticles" << std::endl;
	try {
		std::string particleMessage = serializeParticles(particles);
		std::cout << clients.size() << std::endl;

		for (auto& clientPtr : clients) {
			boost::shared_ptr<boost::asio::ip::tcp::socket> sharedClientPtr = clientPtr;
			boost::asio::async_write(*clientPtr, boost::asio::buffer(particleMessage),
				[sharedClientPtr](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
					std::cout << "before error checking" << std::endl;
					if (!error) {
						// Write completed successfully
						std::cout << "Particle message sent to client" << std::endl;
					}
					else {
						// Handle error
						std::cerr << "Error in async_write: " << error.message() << std::endl;
					}
				}
			);
		}

	}
	catch (std::exception& e) {
		std::cerr << "Exception in server: " << e.what() << "\n";
	}
}

void sendSprites() {
	try {
		// Wait for all clients to send their sprite positions 
		// before broadcasting an update
		/*for (int i = 0; i < clientSprites.size(); i++) {
			std::cout << "I'm in the semaphore waiting, waiting for the " << i + 1 << "th client out of " << clientSprites.size() << std::endl;
			clientSemaphore.wait();
		}*/

		std::string spriteMessage = serializeSprites(clientSprites);

		for (auto& clientPtr : clients) {
			boost::asio::write(*clientPtr, boost::asio::buffer(spriteMessage));
		}

	}
	catch (std::exception& e) {
		std::cerr << "Exception in server: " << e.what() << "\n";
	}
}	

void runPeriodicSend() {
	//boost::asio::io_context io_context;
	boost::asio::steady_timer timer(io_context);
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(io_context);

	// Define function for periodic sending
	using PeriodicTask = std::function<void()>;

	// Initialize the periodic task with a lambda that captures the timer and clients
	PeriodicTask periodicTask = [&]() {
		timer.expires_after(std::chrono::seconds(10));
		timer.async_wait([&](const boost::system::error_code& error) {
			if (!error) {
				std::cout << "about to periodic send.." << std::endl;
				// Send particles to clients
				sendParticles();
				// Send sprites to clients
				//sendSprites();

				// Reschedule the timer
				periodicTask();
			}
			});
		};

	// Start periodic sending
	periodicTask();

	// Run the io_context to start the timer
	io_context.run();
}

std::mutex spriteVectorMutex;

void handleClient(int pos) {
	// Will have to change buffer size to accommodate message length (from JSON)
	constexpr size_t bufferSize = 1024;
	auto buffer = std::make_shared<std::array<char, bufferSize>>();

	try {
		boost::shared_ptr<boost::asio::ip::tcp::socket> sharedClientPtr = clients[pos];

		sharedClientPtr->async_read_some(boost::asio::buffer(*buffer),
			[&buffer, &pos](const boost::system::error_code& error, std::size_t length) {
				if (error) {
					throw boost::system::system_error(error);
				}
				else {
					std::string message = std::string(buffer->data(), length);
					//std::cout << message << std::endl;
					std::istringstream iss(message);
					int id = 0;
					float x = 0.0f;
					float y = 0.0f;

					// Attempt to extract the first float
					if (iss >> id) {
						// Read the second word (float1)
						if (iss >> x) {
							// Read the third word (float2)
							if (iss >> y) {
								//std::cout << "ID: " << id << ", Float1: " << x << ", Float2: " << y << std::endl;
							}
							else {
								std::cerr << "Error: Could not parse the third float." << std::endl;
							}
						}
						else {
							std::cerr << "Error: Could not parse the second float." << std::endl;
						}
					}
					else {
						std::cerr << "Error: Could not parse the ID." << std::endl;
					}
					//std::cout << "Received sprite position data from client " << id << ": (" << x << ", " << y << ")" << std::endl;

					//std::unique_lock<std::mutex> spriteVectorLock(spriteVectorMutex);
					/*clientSprites[id - 1].x = x;
					clientSprites[id - 1].y = y;*/
					for (int i = 0; i < clientSprites.size(); i++) {
						if (clientSprites[i].clientID == id) {
							clientSprites[i].x = x;
							clientSprites[i].y = y;
							break;
						}	
					}
					//clientSemaphore.notify();
					//spriteVectorLock.unlock();
					
					message.clear();
					handleClient(pos);
				}

			});

			io_context.run();
			// The \0 appended approach to string reading:
			//size_t length = socket.read_some(boost::asio::buffer(buffer, 1), error);

			//if (error) {
			//	throw boost::system::system_error(error);
			//}

			//// Append received character to the message
			//if (buffer[0] == '\0') {
			//	break; // Null terminator encountered, end of message
			//}
			//else {
			//	message.push_back(buffer[0]);
			//}
	}
	catch (const std::exception& e) {
		std::cerr << "Error receiving message: " << e.what() << std::endl;
	}

}


std::mutex clientCtrMutex;

void runServer() {
	try {
		std::cout << "Server listening...on port: " << port << std::endl;
		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

		// Use a loop to continuously accept new clients
		while (true) {

			boost::shared_ptr<tcp::socket> socketPtr(new tcp::socket(io_context));
			acceptor.accept(*socketPtr);
			socketPtr->non_blocking(true);

			std::cout << "New client connected." << std::endl;
			//unique mutex lock
			std::unique_lock<std::mutex> clientCtrLock(clientCtrMutex);
			clients.push_back(socketPtr);
			clientCtr++;
			clientCtrLock.unlock();

			GLuint explorerTexture;
			LoadTexture("squareman.jpg", explorerTexture); //change sprite image here 
			Sprite newSprite = Sprite(clientCtr, 640.0f, 360.0f, 100.0f, explorerTexture);
			//std::cout << explorerTexture << std::endl;
			clientSprites.push_back(newSprite);

			// Send client its ID
			std::string IDMsg = "ID\n" + std::to_string(clientCtr);
			//boost::asio::write(clients.back(), boost::asio::buffer(IDMsg));
			std::string id_message = std::to_string(clientCtr);
			boost::asio::write(*(clients[clientCtr - 1]), boost::asio::buffer(id_message));
			/*sendParticles(clients);
			sendSprites(clients);*/

			// Create a copy of the client socket object
			/*tcp::socket clientCopy(io_context);
			clientCopy = clients.back();*/

			std::thread clientThread(handleClient, clients.size() - 1);
			clientThread.detach();

			
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception in server: " << e.what() << "\n";
	}
}


int main(int argc, char *argv) {
	std::vector<std::thread> clientThreads;
	//std::thread serverThread(runServer, std::ref(clients), std::ref(clientThreads));
	std::thread serverThread(runServer);
	serverThread.detach();

	// Start the periodic sending of particles in a separate thread
	std::thread periodicSendThread(runPeriodicSend);
	periodicSendThread.detach();

	// Start thread for sending sprites
	//std::thread sendSpritesThread(sendSprites);
	//sendSpritesThread.detach();


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
	
	glfwSwapInterval(1); // VSync setting here

	ImGui::CreateContext();
	
    ImGuiIO& io = ImGui::GetIO();

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

	double updateInterval = 0.5; // Interval for updating framerate (0.5 seconds)
	double lastFPSUpdateTime = 0.0; // Last time the framerate was updated

	double currentFramerate = 0.0;

	GLuint explorerTexture;
	isSpriteImageAvailable = LoadTexture("squareman.jpg", explorerTexture); //change sprite image here 

	//explorerSprite = new Sprite(640, 360, 100.0f, explorerTexture); // change sprite speed and initial position here
	char imagePath[256] = "";

	std::string loadImageMessage = "";

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();

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

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));

		ImGui::ColorEdit3("Particle Color", (float*)&particleColor);

		ImGui::Dummy(ImVec2(0, 10));

		ImGui::Dummy(ImVec2(0, 10));
		ImGui::Text("Current FPS: %.f", currentFramerate);
		ImGui::Text("Number of Particles: %d", particles.size());

		ImGui::PopStyleColor(4);

		ImGui::Dummy(ImVec2(0, 10));

		if (ImGui::GetIO().Fonts->Fonts.Size > 0) {
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

			ImGui::Text("Particle Physics Simulator V2");
			ImGui::Text("STDISCM - S12");
			ImGui::Text("Joshua Ejercito, Ryan Go, Anton Morana, Jacob Villa");

			ImGui::PopFont();
		}
		else {
			ImGui::Text("Particle Physics Simulator V2");
			ImGui::Text("STDISCM - S12");
			ImGui::Text("Joshua Ejercito, Ryan Go, Anton Morana, Jacob Villa");
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

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(85.0f / 255.0f, 0.0f, 85.0f / 255.0f, 1.0f));
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
					newParticleAngle >= 0.0 && newParticleAngle <= 360.0) 
				{
					// json j;
					// j["x"] = newParticleX;
					// j["y"] = newParticleY;
					// j["angle"] = newParticleAngle;
					// j["velocity"]= newParticleVelocity;

					// std::string serializedParticles = j.dump();
					// sendParticlestoClient(j);
					particles.emplace_back(newParticleX, newParticleY, newParticleAngle, newParticleVelocity);
					sendParticles();
				}
				else {
					showErrorPopup = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Spawn Random Particle")) {
				SpawnRandomParticle();
				sendParticles();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Reset Particles")) {
				particles.clear();
				sendParticles();
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
				/*
				std::vector<Particle> particlesSent;
				*/
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
						// json j;
						// j["x"] = x;
						// j["y"] = y;
						// j["angle"] = startAngle;
						// j["velocity"]= startVelocity;

						// std::string serializedParticles = j.dump();
						// sendParticlestoClient(j);
						particles.emplace_back(x, y, startAngle, startVelocity);
						break;
					case 1: // Varying Angle
						angle = fmod(angle, 360.0f);
												// json j;
						// j["x"] = startX;
						// j["y"] = startY;
						// j["angle"] = angle;
						// j["velocity"]= startVelocity;

						// std::string serializedParticles = j.dump();
						// sendParticlestoClient(j);
						particles.emplace_back(startX, startY, angle, startVelocity);
						break;
					case 2: // Varying Velocity
						particles.emplace_back(startX, startY, startAngle, velocity);
						break;
					}
					//std::cout << "Particle position: (" << x << ", " << y << ")" << std::endl;
				}
				sendParticles();
			}
		}
		else {
			ImGui::Text("Buttons disabled in Explorer mode");
		}
		ImGui::Dummy(ImVec2(0, 55));
		ImGui::Text("--------------------------------------------------------------------------------------------------------------------");
		//if (ImGui::Button("Developer mode")) {
		//	std::cout << "Developer mode" << std::endl;
		//	currentMode = DEVELOPER;
		//	zoomFactor = 1.0f;
		//}
		//if (ImGui::Button("Explorer mode")) {
		//	std::cout << "Explorer mode" << std::endl;
		//	currentMode = EXPLORER;
		//	//zoomFactor = 3.0f;
		//	float scaleFactorWidth = 1280.0f / 19.0f;
		//	float scaleFactorHeight = 720.0f / 33.0f;
		//	zoomFactor = std::min(scaleFactorWidth, scaleFactorHeight);
		//}

		//ImGui::InputFloat("Sprite Width (Max: 15, Min: 3)", &spriteWidth);
		//ImGui::InputFloat("Sprite Height (Max: 15, Min: 3)", &spriteHeight);
		////std::cout << "Sprite Width: " << spriteWidth << ", Sprite Height: " << spriteHeight << std::endl;
		//ImGui::InputText("Image Path", imagePath, sizeof(imagePath));

		//if (ImGui::Button("Load Image")) {
		//	GLuint newTextureID;
		//	if (LoadTexture(imagePath, newTextureID)) {
		//		if (explorerSprite) {
		//			glDeleteTextures(1, &explorerSprite->textureID);
		//		}
		//		explorerSprite->textureID = newTextureID;
		//		isSpriteImageAvailable = true;
		//		loadImageMessage = "Sprite successfully loaded.";
		//	}
		//	else {
		//		std::cout << "Failed to load image: " << imagePath << std::endl;
		//		isSpriteImageAvailable = false;
		//		loadImageMessage = "Failed to load image. Make sure you are using the right directory and image type.";
		//	}
		//}

		//if (!loadImageMessage.empty()) {
		//	ImGui::Text("%s", loadImageMessage.c_str());
		//}


		//ImGui::Text("Sprite Coordinates: (%.2f, %.2f)", explorerSprite->x, explorerSprite->y);


		// Check for key presses and releases
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) keyW = true;
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) keyW = false;

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) keyA = true;
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) keyA = false;

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) keyS = true;
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) keyS = false;

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) keyD = true;
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) keyD = false;

		ImGui::PopStyleColor(4);

		ImGui::End();

		ImGui::PopStyleVar();

		std::vector<std::future<void>> futures;
		size_t numParticles = particles.size();
		size_t numThreads = std::thread::hardware_concurrency();
		size_t chunkSize = numParticles / numThreads;

		for (size_t i = 0; i < numThreads; ++i) {
			auto startIter = particles.begin() + i * chunkSize;
			auto endIter = (i == numThreads - 1) ? particles.end() : startIter + chunkSize;
			futures.push_back(std::async(std::launch::async, UpdateParticlesRange, startIter, endIter, std::ref(io)));
		}

		futures.push_back(std::async(std::launch::async, DrawElements));

		for (auto& future : futures) {
			future.wait();
		}

		if (currentTime - lastFPSUpdateTime >= updateInterval) {
			currentFramerate = io.Framerate;
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
	}

	serverThread.join();

	/*for (auto& thread : clientThreads) {
		if (thread.joinable()) {
			thread.join();
		}
	}*/

	/*if (explorerSprite) {
		glDeleteTextures(1, &explorerSprite->textureID);
		delete explorerSprite;
		explorerSprite = nullptr;
	}*/

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();

	return 0;
}