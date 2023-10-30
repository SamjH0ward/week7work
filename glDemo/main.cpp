
#include "core.h"
#include "TextureLoader.h"
#include "ArcballCamera.h"
#include "PrincipleAxes.h"
#include "AIMesh.h"
#include "Cube.h"
#include "2DExamples.h"
#include "tetrahedron.h"


using namespace std;
using namespace glm;


#pragma region Global variables

// Main camera
ArcballCamera* mainCamera = nullptr;

// Mouse tracking
bool				mouseDown = false;
double				prevMouseX, prevMouseY;

// Scene objects
CGPrincipleAxes* principleAxes = nullptr;
Cube* cube = nullptr;
Tetrahedron* tetrahedron = nullptr;
AIMesh* creatureMesh = nullptr;
AIMesh* planetMesh = nullptr;
AIMesh* duckMesh = nullptr;

// Window size
const unsigned int initWidth = 512;
const unsigned int initHeight = 512;

#pragma endregion


// Function prototypes
void renderScene();
void updateScene();
void resizeWindow(GLFWwindow* window, int width, int height);
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos);
void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
void mouseEnterHandler(GLFWwindow* window, int entered);


int main() {

	//
	// 1. Initialisation
	//
	
#pragma region OpenGL and window setup

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
	glfwSetCursorPosCallback(window, mouseMoveHandler);
	glfwSetMouseButtonCallback(window, mouseButtonHandler);
	glfwSetScrollCallback(window, mouseScrollHandler);
	glfwSetCursorEnterCallback(window, mouseEnterHandler);

	// Initialise glew
	glewInit();

	
	// Setup window's initial size
	resizeWindow(window, initWidth, initHeight);

#pragma endregion


	// Initialise scene - geometry and shaders etc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // setup background colour to be black
	glClearDepth(1.0f);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//
	// Setup Textures, VBOs and other scene objects
	//
	mainCamera = new ArcballCamera(0.0f, 0.0f, 1.98595f, 55.0f, 1.0f, 0.1f, 500.0f);
	
	principleAxes = new CGPrincipleAxes();

	cube = new Cube();
	tetrahedron = new Tetrahedron();

	creatureMesh = new AIMesh(string("Assets\\beast\\beast.obj"));
	if (creatureMesh) {
		creatureMesh->addTexture(string("Assets\\beast\\beast_texture.bmp"), FIF_BMP);
	}

	duckMesh = new AIMesh(string("Assets\\duck\\rubber_duck_toy_4k.obj"));
	if (duckMesh) {
		duckMesh->addTexture(string("Assets\\duck\\rubber_duck_toy_diff_4k.jpg"), FIF_JPEG);
	}


	planetMesh = new AIMesh(string("Assets\\gsphere.obj"));
	if (planetMesh) {
		planetMesh->addTexture(string("Assets\\Textures\\Hodges_G_MountainRock1.jpg"), FIF_JPEG);
	}


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


// Demo code for studio - used to show 2D content and expand view into 3D

bool showViewplaneQuad = false;
bool showStar = true; // if false render triangle
bool showPrincipleAxes = false;
bool showZAxis = false;

void demo_render2DStuff(mat4 cameraTransform) {

	glLoadMatrixf((GLfloat*)&cameraTransform);

	if (showStar)
		render2D_star();
	else
		render2D_triangle();
	
	if (showViewplaneQuad) {

		render2D_quadOutline();
	}
}

// renderScene - function to render the current scene
void renderScene()
{
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 cameraTransform = mainCamera->projectionTransform() * mainCamera->viewTransform();

#if 0

	demo_render2DStuff(cameraTransform);

	// Render principle axes - no modelling transforms so just use cameraTransform
	if (showPrincipleAxes) {

		mat4 paTransform = cameraTransform * glm::scale(identity<mat4>(), vec3(0.8f, 0.8f, 0.8f));
		glLoadMatrixf((GLfloat*)&paTransform);
		principleAxes->render(showZAxis);
	}

#endif

#if 0

	// Render cube - no modelling transform so leave cameraTransform set in OpenGL and render
	glLoadMatrixf((GLfloat*)&cameraTransform);
	glDisable(GL_CULL_FACE);
	cube->render();

#endif

#if 0

	// Render cube - no modelling transform so leave cameraTransform set in OpenGL and render
	glLoadMatrixf((GLfloat*)&cameraTransform);
	glDisable(GL_CULL_FACE);
	tetrahedron->render();

#endif

#if 0
	
	if (creatureMesh) {

		// Setup transforms
		glLoadMatrixf((GLfloat*)&cameraTransform);

		creatureMesh->preRender();
		creatureMesh->render();
		creatureMesh->postRender();
	}
	
	if (planetMesh) {

		// Setup transforms
		mat4 planetTranslate = translate(identity<mat4>(), vec3(2.0f, 2.0f, 2.0f));
		mat4 T = cameraTransform * planetTranslate;
		glLoadMatrixf((GLfloat*)&T);

		planetMesh->preRender();
		planetMesh->render();
		planetMesh->postRender();
	}
#endif

#if 1

	if (duckMesh) {

		// Setup transforms
		glLoadMatrixf((GLfloat*)&cameraTransform);

		duckMesh->preRender();
		duckMesh->render();
		duckMesh->postRender();
	}


	
#endif

}


// Function called to animate elements in the scene
void updateScene() {
}


#pragma region Event handler functions

// Function to call when window resized
void resizeWindow(GLFWwindow* window, int width, int height)
{
	if (mainCamera) {

		mainCamera->setAspect((float)width / (float)height);
	}

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
			
			case GLFW_KEY_S:
				showStar = !showStar;
				break;

			case GLFW_KEY_V:
				showViewplaneQuad = !showViewplaneQuad;
				break;

			case GLFW_KEY_A:
				showPrincipleAxes = !showPrincipleAxes;
				break;

			case GLFW_KEY_Z:
				showZAxis = !showZAxis;
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
			default:
			{
			}
		}
	}
}


void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos) {

	if (mouseDown) {

		//cout << "x = " << xpos << ", y = " << ypos << endl;
		double dx = xpos - prevMouseX;
		double dy = ypos - prevMouseY;

		if (mainCamera)
			mainCamera->rotateCamera((float)-dy, (float)-dx);

		prevMouseX = xpos;
		prevMouseY = ypos;
	}

}

void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT) {

		if (action == GLFW_PRESS) {

			mouseDown = true;
			glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
		}
		else if (action == GLFW_RELEASE) {

			mouseDown = false;
		}
	}
}

void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset) {

	if (mainCamera) {

		if (yoffset < 0.0)
			mainCamera->scaleRadius(1.1f);
		else if (yoffset > 0.0)
			mainCamera->scaleRadius(0.9f);

		//cout << mainCamera->getRadius() << endl;
	}
}

void mouseEnterHandler(GLFWwindow* window, int entered) {
}

#pragma endregion