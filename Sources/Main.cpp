// ----------------------------------------------
// Base code for practical computer graphics
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "LightSource.h"
#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "MeshLoader.h"

static const std::string SHADER_PATH ("../Resources/Shaders/");

static const std::string MATERIAL_PATH ("../Resources/Materials/");

static const std::string MATERIAL_NAME ("Brick/");

static const std::string DEFAULT_MESH_FILENAME ("../Resources/Models/face.off");

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram> shaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader

// Specifies the number of light to use :
// 1 means there is only a key light
// 2 means there is also a fill light
// 3 means there is also a back light
static int numberLightUsed = 1;

// Specifies which render have to be use
// 0 means PBR rendering
#define SHADER_MODE_PBR 0
// 1 means a basic toon rendering
#define SHADER_BASIC_TOON 1
// 2 means a depth-of-field-based X-toon rendering
#define SHADER_DEPTH_X_TOON 2
// 3 means a perspective-based X-toon rendering
#define SHADER_PERSEPECTIVE_X_TOON 3
// 4 means a orientation-based X-toon rendering
#define SHADER_ORIENTATION 4
// 5 means a orientation-based X-toon rendering
#define SHADER_DEPTH_MAPPING 5

static int shaderMode = SHADER_MODE_PBR;

static GLuint FramebufferName;
// Specificies if the normal is computed thanks to the normal map
// defaultly no
static int normalMapUsed = 0;

static std::vector<std::shared_ptr<LightSource>> lightSources(3);

static std::shared_ptr<Material> materialPtr;

static float zMin = -350.0;

static float r = 1.0;

static int textureUsing = 0;

static float zFocus = 0.0;
// Camera control variables
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

void exitOnCriticalError (const std::string & message);

void render();

void initScene(const std::string & meshFilename);

void init (const std::string & meshFilename);

void clear ();

void printHelp ()
{
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
   			  << "    * ESC: quit the program" << std::endl
			  << "    * F5: load shader" << std::endl
			  << "    * T: switch between PBR mode and TSM (Toon Shading Mode)" << std::endl
			  << "    * 1: basic toon shading (default mode of TSM)" << std::endl
			  << "    * 2: X-Toon shading depth and view-point based (once in TSM)" << std::endl
			  << "    * 3: X-Toon shading depth and axis based (once in TSM)" << std::endl
			  << "    * 4: X-Toon shading orientation based (once in TSM)" << std::endl
			  << "    * UP: increment the number of lights to use (max 3)" << std::endl
			  << "    * DOWN: decrement the number of lights to use (min 1)" << std::endl
			  << "    * I: run a laplacian filtering with alpha = 0.1" << std::endl
			  << "    * O: run a laplacian filtering with alpha = 0.5" << std::endl
			  << "    * P: run a laplacian filtering with alpha = 1.0" << std::endl
			  << "    * S: run the simplification with a predefined resolution" << std::endl
			  << "    * A: run the simplification using an octree" << std::endl;
}

void switchShaderMode(int mode)
{
	if(mode!=SHADER_MODE_PBR)
	{
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	}

	shaderMode = mode;
	shaderProgramPtr->use();
	shaderProgramPtr->set("shaderMode",shaderMode);
	shaderProgramPtr->set("zMin",zMin);
	shaderProgramPtr->set("r",r);
	shaderProgramPtr->set("zFocus",zFocus);
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height)
{
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region withminin the window
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_H)
	{
		printHelp ();
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F1)
	{
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_T)
	{
		if(shaderMode==SHADER_MODE_PBR)
		{
			switchShaderMode(SHADER_BASIC_TOON);
		} else
		{
			switchShaderMode(SHADER_MODE_PBR);
		}
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_1 && shaderMode != SHADER_MODE_PBR)
	{
		switchShaderMode(SHADER_BASIC_TOON);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_2 && shaderMode != SHADER_MODE_PBR)
	{
		switchShaderMode(SHADER_DEPTH_X_TOON);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_3 && shaderMode != SHADER_MODE_PBR)
	{
		switchShaderMode(SHADER_PERSEPECTIVE_X_TOON);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_4 && shaderMode != SHADER_MODE_PBR)
	{
		switchShaderMode(SHADER_ORIENTATION);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_5 && shaderMode != SHADER_MODE_PBR)
	{
		switchShaderMode(SHADER_DEPTH_MAPPING);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F5)
	{
		try
		{
			shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl",
														         	 SHADER_PATH + "FragmentShader.glsl");
		} catch (std::exception & e)
		{
			exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
		}

		initScene(DEFAULT_MESH_FILENAME);
		switchShaderMode(shaderMode);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_UP)
	{
		numberLightUsed = min(numberLightUsed+1,3);
		shaderProgramPtr->use();
		shaderProgramPtr->set("numberLightUsed",numberLightUsed);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_DOWN)
	{
		numberLightUsed = max(numberLightUsed-1,1);
		shaderProgramPtr->use();
		shaderProgramPtr->set("numberLightUsed",numberLightUsed);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_Q)
	{
		zMin = zMin - meshScale/30;
		std::cout<<"zMin : "<<zMin<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("zMin",zMin);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_W)
	{
		zMin = min(zMin + meshScale/30,meshPtr->getZMin());
		std::cout<<"zMin : "<<zMin<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("zMin",zMin);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_E)
	{
		r = max(r - float(0.05),float(1.001));
		std::cout<<"r value : "<<r<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("r",r);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_R)
	{
		r = r + 0.05;
		std::cout<<"r value : "<<r<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("r",r);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_D)
	{
		zFocus = min(zFocus - meshScale/10,0.0f);
		std::cout<<"z of focus point : "<<zFocus<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("zFocus",zFocus);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F)
{
		zFocus = zFocus + meshScale/10;
		std::cout<<"z of focus point : "<<zFocus<<std::endl;
		shaderProgramPtr->use();
		shaderProgramPtr->set("zFocus",zFocus);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_N)
	{
		std::cout << "normal mapping"<< std::endl;
		normalMapUsed = 1-normalMapUsed;
		shaderProgramPtr->use();
		shaderProgramPtr->set("normalMapUsed",normalMapUsed);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_X)
{
		std::cout << "texture using" <<std::endl;
		textureUsing = 1-textureUsing;
		shaderProgramPtr->use();
		shaderProgramPtr->set("textureUsing",textureUsing);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_I)
{
		std::cout << "laplacian filter with an alpha of 0.1";
		meshPtr->laplacianFilter(0.1, true);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_O)
{
		std::cout << "laplacian filter with an alpha of 0.5";
		meshPtr->laplacianFilter(0.5, true);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_P)
	{
		std::cout << "laplacian filter with an alpha of 1.0";
		meshPtr->laplacianFilter(1.0, true);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_S)
	{
		int resolution = 32;
		std::cout << "simplification with a resolution of : " << resolution << std::endl;
		meshPtr->simplify(resolution);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_A)
	{
		unsigned int numberOfVertexPerLeaf = 10;
		std::cout << "octree simplification with a maximum number of vertex per cell of : " << numberOfVertexPerLeaf << std::endl;
		meshPtr->adaptiveSimplify(numberOfVertexPerLeaf);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_L)
	{
		std::cout << "run a subdivision according loop scheme" << std::endl;
		meshPtr->subdivide();
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating)
	{
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning)
	{
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	}
	else if (isZooming)
	{
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
    	if (!isRotating)
		{
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        }
    }
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
    	isRotating = false;
    }
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    }
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
    	isPanning = false;
    }
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
	{
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    }
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
	{
    	isZooming = false;
    }
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ())
	{
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Computer Graphics - Practical Assignment", nullptr, nullptr);
	if (!windowPtr)
	{
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}

void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress))
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
    glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// Loads and compile the programmable shader pipeline

	try
	{
		shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl",
													         	 SHADER_PATH + "FragmentShader.glsl");
	}
	catch (std::exception & e)
	{
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
}

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));

	// Mesh
	meshPtr = std::make_shared<Mesh> ();

	try
	{
		MeshLoader::loadOFF (meshFilename, meshPtr);
	}
	catch (std::exception & e)
	{
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}
	meshPtr->init ();

	glm::vec3 center;
	meshPtr->computeBoundingSphere (center, meshScale);

	shaderProgramPtr->set("meshCenter", center);

	// Lighting

	// Key light
	lightSources.at(0) = std::make_shared<LightSource> ();
	lightSources.at(0)->setColor(glm::vec3(1.0,1.0,1.0));
	lightSources.at(0)->setIntensity(10.f);
	lightSources.at(0)->setConeAngle(M_PI/3);
	lightSources.at(0)->setRadialAttenuation(1.f);
	lightSources.at(0)->setDistanceAttenuation(glm::vec3(1,0.000001,0.0000001));
	lightSources.at(0)->setTranslation(center+glm::vec3(7.0*meshScale, 0.0, 7.0*meshScale));
	shaderProgramPtr->set ("keyLight.color", lightSources.at(0)->getColor());
	shaderProgramPtr->set ("keyLight.intensity", lightSources.at(0)->getIntensity());
	shaderProgramPtr->set ("keyLight.coneAngle", lightSources.at(0)->getConeAngle());
	shaderProgramPtr->set ("keyLight.radialAttenuation", lightSources.at(0)->getRadialAttenuation());
	shaderProgramPtr->set ("keyLight.distanceAttenuation", lightSources.at(0)->getDistanceAttenuation());


	// Fill light

	lightSources.at(1) = std::make_shared<LightSource> ();
	lightSources.at(1)->setColor(glm::vec3(1.0,1.0,1.0));
	lightSources.at(1)->setIntensity(6.f);
	lightSources.at(1)->setConeAngle(M_PI/3);
	lightSources.at(1)->setRadialAttenuation(1.f);
	lightSources.at(1)->setDistanceAttenuation(glm::vec3(1,0.000001,0.0000001));
	lightSources.at(1)->setTranslation(center+glm::vec3(-7.0*meshScale, 0.0, 7.0*meshScale));
	shaderProgramPtr->set ("fillLight.color", lightSources.at(1)->getColor());
	shaderProgramPtr->set ("fillLight.intensity", lightSources.at(1)->getIntensity());
	shaderProgramPtr->set ("fillLight.coneAngle", lightSources.at(1)->getConeAngle());
	shaderProgramPtr->set ("fillLight.radialAttenuation", lightSources.at(1)->getRadialAttenuation());
	shaderProgramPtr->set ("fillLight.distanceAttenuation", lightSources.at(1)->getDistanceAttenuation());

	// Back light

	lightSources.at(2) = std::make_shared<LightSource> ();
	lightSources.at(2)->setColor(glm::vec3(1.0,1.0,1.0));
	lightSources.at(2)->setIntensity(3.f);
	lightSources.at(2)->setConeAngle(M_PI/3);
	lightSources.at(2)->setRadialAttenuation(1.f);
	lightSources.at(2)->setDistanceAttenuation(glm::vec3(1,0.000001,0.0000001));
	lightSources.at(2)->setTranslation(center+glm::vec3(7.0*meshScale, 0.0, -7.0*meshScale));
	shaderProgramPtr->set ("backLight.color", lightSources.at(2)->getColor());
	shaderProgramPtr->set ("backLight.intensity", lightSources.at(2)->getIntensity());
	shaderProgramPtr->set ("backLight.coneAngle", lightSources.at(2)->getConeAngle());
	shaderProgramPtr->set ("backLight.radialAttenuation", lightSources.at(2)->getRadialAttenuation());
	shaderProgramPtr->set ("backLight.distanceAttenuation", lightSources.at(2)->getDistanceAttenuation());

	shaderProgramPtr->set ("numberLightUsed", numberLightUsed);

	// Material
	materialPtr = std::make_shared<Material> ();
	materialPtr->setAlbedo(glm::vec3(0.4,0.6,0.2));
	materialPtr->setKd(0.2);
	materialPtr->setMetallic(0.9);
	materialPtr->setRoughness(0.1);
	shaderProgramPtr->set ("material.kd", materialPtr->getKd());
	shaderProgramPtr->set ("material.albedo", materialPtr->getAlbedo());
	shaderProgramPtr->set ("material.metallic", materialPtr->getMetallic());
	shaderProgramPtr->set ("material.roughness", materialPtr->getRoughness());


	GLuint albedoTex = loadTextureFromFileToGPU(MATERIAL_PATH+MATERIAL_NAME+"Base_Color.png",false);
	GLuint roughnessTex = loadTextureFromFileToGPU(MATERIAL_PATH+MATERIAL_NAME+"Roughness.png",false);
	GLuint metallicTex = loadTextureFromFileToGPU(MATERIAL_PATH+MATERIAL_NAME+"Metallic.png",false);
	GLuint ambientTex = loadTextureFromFileToGPU(MATERIAL_PATH+MATERIAL_NAME+"Ambient_Occlusion.png",false);
	GLuint normalTex = loadTextureFromFileToGPU(MATERIAL_PATH+MATERIAL_NAME+"Normal.png",true);
	GLuint toneTex = loadTextureFromFileToGPU(MATERIAL_PATH+"Style.png",false);

	shaderProgramPtr->set("material.albedoTex",0);
	shaderProgramPtr->set("material.roughnessTex",1);
	shaderProgramPtr->set("material.metallicTex",2);
	shaderProgramPtr->set("material.ambientTex",3);
	shaderProgramPtr->set("material.normalTex",4);
	shaderProgramPtr->set("material.toneTex",5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,albedoTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,roughnessTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,metallicTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,ambientTex);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D,normalTex);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D,toneTex);


	// Adjust the camera to the actual mesh
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);

	shaderProgramPtr->set("textureUsing",textureUsing);
	zMin = -300.0f;
	zFocus = -80.0f;
	r = 1.6f;
}

void initTextureBuffer()
{
	FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		exitOnCriticalError(std::string("[Error creating framebuffer]"));

	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	// The fullscreen quad's FBO
	GLuint quad_VertexArrayID;
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
}

void init (const std::string & meshFilename)
{
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initScene (meshFilename); // Actual scene to render
	initTextureBuffer();
}

void clear ()
{
	cameraPtr.reset ();
	meshPtr.reset ();
	shaderProgramPtr.reset ();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}

// The main rendering call
void render ()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	shaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	shaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	shaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	shaderProgramPtr->set ("normalMat", normalMatrix);
	shaderProgramPtr->set ("keyLightPosition", lightSources.at(0)->getTranslation());
	shaderProgramPtr->set ("fillLightPosition", lightSources.at(1)->getTranslation());
	shaderProgramPtr->set ("backLightPosition", lightSources.at(2)->getTranslation());
	meshPtr->render ();
	// Render to the screen
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1024, 768, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
	shaderProgramPtr->stop ();

}

// Update any accessible variable based on the current time
void update (float currentTime)
{
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->
	//TODO// To animate light color
	//lightPtr->setColor(glm::vec3(sin(currentTime*2.0f)+0.1f,cos(currentTime*2.0f)+0.4f,sin(currentTime*1.3f+3.14159)+0.1f));
	//shaderProgramPtr->set("keyLight.color",lightSources.at(0)->getColor());
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
}

void usage (const char * command)
{
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv)
{
	if (argc > 2)
		usage (argv[0]);

	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]);

	while (!glfwWindowShouldClose (windowPtr))
	{
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;
}
