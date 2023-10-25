
#include "core.h"
#include "TextureLoader.h"
#include "star-vbo.h"
#include "RandomStars.h"
#include "PlanetSystem.h"
#include "TextureQuad.h"


using namespace std;
using namespace glm;


// global variables
SimplePlanetSystem planets = SimplePlanetSystem();

GLuint backgroundTexture = 0;

// Player variables
vec2 playerPos = vec2(0.0f, 0.0f);
vec2 playerVelocity = vec2(0.0f, 0.0f);
float playerOrientation = 0.0f; // orientation in degrees
GLuint playerSpriteTexture = 0;

// Camera variables
float cameraZoom = 1.0f;
vec2 cameraPos = vec2(0.0f, 0.0f);

// Keyboard input state
bool zoomInPressed = false;
bool zoomOutPressed = false;
bool rotateLeftPressed = false;
bool rotateRightPressed = false;
bool acceleratePressed = false;
bool deceleratePressed = false;

// Window size
const unsigned int initWidth = 512;
const unsigned int initHeight = 512;

// Function prototypes
void renderScene();
void resizeWindow(GLFWwindow* window, int width, int height);
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
void updateScene();


int main() {

	//
	// 1. Initialisation
	//
	

	// Initialise glfw and setup window
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);

	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Real-Time Computer Graphics", NULL, NULL);

	// Check window was created successfully
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	

	// Set callback functions to handle different events
	glfwSetFramebufferSizeCallback(window, resizeWindow); // resize window callback
	glfwSetKeyCallback(window, keyboardHandler); // Keyboard input callback


	// Initialise glew
	glewInit();

	
	// Setup window's initial size
	resizeWindow(window, initWidth, initHeight);

	// Initialise scene - geometry and shaders etc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // setup background colour to be black


	//
	// Setup Textures, VBOs and other scene objects
	//

	setupTextureQuadVBO();

	planets.initialise();

	// Setup sprites
	playerSpriteTexture = loadTexture(string("Assets\\Textures\\player1_ship.png"), FIF_PNG);
	backgroundTexture = loadTexture(string("Assets\\Textures\\stars.jpg"), FIF_JPEG);


	//
	// 2. Main loop
	// 

	while (!glfwWindowShouldClose(window)) {

		updateScene();
		renderScene();						// Render into the current buffer
		glfwSwapBuffers(window);			// Displays what was just rendered (using double buffering).

		glfwPollEvents();					// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}



// renderScene - function to render the current scene
void renderScene()
{
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 orthoProjection = ortho(-cameraZoom, cameraZoom, -cameraZoom, cameraZoom);
	mat4 viewTransform = glm::translate(identity<mat4>(), vec3(-cameraPos.x, -cameraPos.y, 0.0f));
	mat4 cameraTransform = orthoProjection * viewTransform;


	//
	// Render background
	//
	mat4 backgroundScale = glm::scale(identity<mat4>(), vec3(20.0f, 20.0f, 1.0f));
	mat4 T = cameraTransform * backgroundScale;

	glLoadMatrixf((GLfloat*)&T);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);

	textureQuadPreRender();
	textureQuadRender();
	textureQuadPostRender();
	

	//
	// Render plants
	//
	planets.render(cameraTransform);


	//
	// Render player
	//

	mat4 playerScale = glm::scale(identity<mat4>(), vec3(0.15f, 0.15f, 1.0f));
	mat4 playerRotation = glm::rotate(identity<mat4>(), glm::radians(playerOrientation), vec3(0.0f, 0.0f, 1.0f));
	mat4 playerTranslate = glm::translate(identity<mat4>(), vec3(playerPos.x, playerPos.y, 0.0f));

	mat4 playerModelTransform = playerTranslate * playerRotation * playerScale;

	T = cameraTransform * playerModelTransform;

	glLoadMatrixf((GLfloat*)&T);


	// Enable Texturing and alpha blending for player rendering
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, playerSpriteTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	textureQuadPreRender();
	textureQuadRender();
	textureQuadPostRender();

	// Disable texturing and alpha blending
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}




// Function to call when window resized
void resizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);		// Draw into entire window
}


// Function to call to handle keyboard input
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		// check which key was pressed...
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;

			case GLFW_KEY_UP:
				zoomInPressed = true;
				break;
			case GLFW_KEY_DOWN:
				zoomOutPressed = true;
				break;

			case GLFW_KEY_W:
				acceleratePressed = true;
				break;
			case GLFW_KEY_S:
				deceleratePressed = true;
				break;
			case GLFW_KEY_A:
				rotateLeftPressed = true;
				break;
			case GLFW_KEY_D:
				rotateRightPressed = true;
				break;

			default:
			{
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// handle key release events
		switch (key)
		{
			case GLFW_KEY_UP:
				zoomInPressed = false;
				break;
			case GLFW_KEY_DOWN:
				zoomOutPressed = false;
				break;

			case GLFW_KEY_W:
				acceleratePressed = false;
				break;
			case GLFW_KEY_S:
				deceleratePressed = false;
				break;
			case GLFW_KEY_A:
				rotateLeftPressed = false;
				break;
			case GLFW_KEY_D:
				rotateRightPressed = false;
				break;

			default:
			{
			}
		}
	}
}


// Function called to animate elements in the scene
void updateScene() {

	planets.update();

	// Update player
	if (zoomInPressed) {
		cameraZoom *= 0.9995f;
	}
	else if (zoomOutPressed) {
		cameraZoom *= 1.0005f;
	}

	// Update player orientation
	if (rotateLeftPressed) {

		playerOrientation += 0.05f;
	}
	else if (rotateRightPressed) {

		playerOrientation -= 0.05f;
	}

	// Update player velocity
	if (acceleratePressed) {

		// calculate rotation matrix from orientation of player
		mat4 R = rotate(identity<mat4>(), radians(playerOrientation), vec3(0.0f, 0.0f, 1.0f));
		// get the x basis vector (the ship sprite points along the x axis when no rotation, so the x axis is the "forward" direction
		vec4 xBasis = R[0];
		// get the (x, y) elements to represent the direction vector we need to accelerate in (it's the direction the ship is pointing in)
		float dx = xBasis.x;
		float dy = xBasis.y;
		// scale (dx, dy) and add to the velocity vector
		playerVelocity.x += dx * 0.0000001f;
		playerVelocity.y += dy * 0.0000001f;
	}
	else if (deceleratePressed) {

		// calculate rotation matrix from orientation of player
		mat4 R = rotate(identity<mat4>(), radians(playerOrientation), vec3(0.0f, 0.0f, 1.0f));
		// get the x basis vector (the ship sprite points along the x axis when no rotation, so the x axis is the "forward" direction
		vec4 xBasis = R[0];
		// get the (x, y) elements to represent the direction vector we need to accelerate in.  For deceleration we set the vector to point in the opposite direction the ship is facing, so we negate the x, y values
		float dx = -xBasis.x;
		float dy = -xBasis.y;
		// scale (dx, dy) and add to the velocity vector
		playerVelocity.x += dx * 0.0000001f;
		playerVelocity.y += dy * 0.0000001f;
	}

	// Update player position
	playerPos.x += playerVelocity.x;
	playerPos.y += playerVelocity.y;

	// Centre camera on player
	cameraPos.x = playerPos.x;
	cameraPos.y = playerPos.y;
}
