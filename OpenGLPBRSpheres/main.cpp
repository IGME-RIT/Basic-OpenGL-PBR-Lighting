
#include <iostream>
#include <vector>

// We are using the glew32s.lib
// Thus we have a define statement
// If we do not want to use the static library, we can include glew32.lib in the project properties
// If we do use the non static glew32.lib, we need to include the glew32.dll in the solution folder
// The glew32.dll can be found here $(SolutionDir)\..\External Libraries\GLEW\bin\Release\Win32
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "Entity.h"
#include "FirstPersonDebug.h"


// Variables for the Height and width of the window
const GLint WIDTH = 800, HEIGHT = 600;

const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

glm::vec2 mousePosition = glm::vec2();

std::vector<Entity*> gameEntities;
void mouseMoveCallback(GLFWwindow *window, GLdouble mouseX, GLdouble mouseY)
{
	mousePosition = glm::vec2(mouseX, mouseY);
}

//Helper method for uploading a vector3 to the shader
void UpdateVec3(char * name, glm::vec3 data, ShaderProgram* program)
{
	GLint uniform = glGetUniformLocation(program->GetGLShaderProgram(), name);
	if (uniform == -1)
	{
		std::cout << "Uniform: " << name << " not found in shader program." << std::endl;
		return;
	}


	glUniform3fv(uniform, 1, &data[0]);
}

//Helper method for uploading a float to the shader
void UpdateFloat(char * name, GLfloat data, ShaderProgram* program)
{
	GLint uniform = glGetUniformLocation(program->GetGLShaderProgram(), name);
	if (uniform == -1)
	{
		std::cout << "Uniform: " << name << " not found in shader program." << std::endl;
		return;
	}


	glUniform1f(uniform, data);
}



int main()
{
	#pragma region GL setup
	//Initializes the glfw
	glfwInit();

	// Setting the required options for GLFW

	// Setting the OpenGL version, in this case 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 99);
	// Setting the Profile for the OpenGL.

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// Setting the forward compatibility of the application to true
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// We don't want the window to resize as of now.
	// Therefore we will set the resizeable window hint to false.
	// To make is resizeable change the value to GL_TRUE.
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create the window object
	// The first and second parameters passed in are WIDTH and HEIGHT of the window we want to create
	// The third parameter is the title of the window we want to create
	// NOTE: Fourth paramter is called monitor of type GLFWmonitor, used for the fullscreen mode.
	//		 Fifth paramter is called share of type GLFWwindow, here we can use the context of another window to create this window
	// Since we won't be using any of these two features for the current tutorial we will pass nullptr in those fields
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Shaders Tutorial", nullptr, nullptr);

	// We call the function glfwGetFramebufferSize to query the actual size of the window and store it in the variables.
	// This is useful for the high density screens and getting the window size when the window has resized.
	// Therefore we will be using these variables when creating the viewport for the window
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	// Check if the window creation was successful by checking if the window object is a null pointer or not
	if (window == nullptr)
	{
		// If the window returns a null pointer, meaning the window creation was not successful
		// we print out the messsage and terminate the glfw using glfwTerminate()
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		// Since the application was not able to create a window we exit the program returning EXIT_FAILURE
		return EXIT_FAILURE;
	}

	// Creating a window does not make it the current context in the windows.
	// As a results if the window is not made the current context we wouldn't be able the perform the operations we want on it
	// So we make the created window to current context using the function glfwMakeContextCurrent() and passing in the Window object
	glfwMakeContextCurrent(window);

	// Enable GLEW, setting glewExperimental to true.
	// This allows GLEW take the modern approach to retrive function pointers and extensions
	glewExperimental = GL_TRUE;

	glfwSetCursorPosCallback(window, mouseMoveCallback);

	// Initialize GLEW to setup OpenGL function pointers
	if (GLEW_OK != glewInit())
	{
		// If the initalization is not successful, print out the message and exit the program with return value EXIT_FAILURE
		std::cout << "Failed to initialize GLEW" << std::endl;

		return EXIT_FAILURE;
	}

	// Setting up the viewport
	// First the parameters are used to set the top left coordinates
	// The next two parameters specify the height and the width of the viewport.
	// We use the variables screenWidth and screenHeight, in which we stored the value of width and height of the window,
	glViewport(0, 0, screenWidth, screenHeight);
	#pragma endregion
	Mesh* sphereMesh = new Mesh("Assets/sphere.obj", true);


	Shader* defaultvShader = new Shader("shaders/defaultvShader.glsl", GL_VERTEX_SHADER);
	Shader* pbrfShader = new Shader("shaders/pbrfShader.glsl", GL_FRAGMENT_SHADER);

	//Core PBR Shader
	ShaderProgram* pbrprogram = new ShaderProgram();
	pbrprogram->AttachShader(defaultvShader);
	pbrprogram->AttachShader(pbrfShader);

	Material* pbrMat = new Material(pbrprogram);
	pbrprogram->LinkProgram();

	//Creating our sphere grid

	int side = 10;

	int index = 0;
	for (float y = 1; y > -2; y-= 3.f/side)
	{
		for (float x = -1; x < 2; x+= 3.f/side)
		{
			gameEntities.push_back(new Entity("sphere" + std::to_string(index)));

			gameEntities[index]->transform->position = glm::vec3(x*side/2.5f, y*side/2.5f, -1.0f);
			gameEntities[index]->transform->recalculateWMatrix = true;
			gameEntities[index]->SetModelMesh(sphereMesh);
			gameEntities[index]->SetModelMaterial(pbrMat);
			index++;
		}
	}

	FirstPersonDebug* fpsCam = new FirstPersonDebug(90, WIDTH, HEIGHT, 0.1f, 100.f);

	
	//Light position
	glm::vec4 lightPositions[4] = {
	   glm::vec4(-10.0f,  10.0f, 10.0f, 1.0),
	   glm::vec4(10.0f,  10.0f, 10.0f, 1.0),
	   glm::vec4(-10.0f, -10.0f, 10.0f, 1.0),
	   glm::vec4(10.0f, -10.0f, 10.0f, 1.0)
	};

	//MSAA
	glEnable(GL_MULTISAMPLE);

	//Fill our depth buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	pbrprogram->Bind();
	//Upload light data
	glUniform4fv(glGetUniformLocation(pbrprogram->GetGLShaderProgram(), "lightPositions"), 4, glm::value_ptr(lightPositions[0]));

	// This is the game loop, the game logic and render part goes in here.
	// It checks if the created window is still open, and keeps performing the specified operations until the window is closed
	while (!glfwWindowShouldClose(window))
	{
		
		
		// Calculate delta time.
		float dt = glfwGetTime();
		// Reset the timer.
		glfwSetTime(0);

		// Specifies the RGBA values which will be used by glClear to clear the color buffer
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WIDTH, HEIGHT);
	#pragma region Updates

		//Update camera and camera matrix
		fpsCam->Update(window, WIDTH, HEIGHT, mousePosition, dt);
		pbrMat->SetMatrix((char*)"cameraMatrix", fpsCam->camera->GetCameraMatrix());

		//Update PBR shader variables
		UpdateFloat((char*)"ao", 1.0f, pbrprogram);
		UpdateVec3((char*)"camPos", fpsCam->transform->position, pbrprogram);
		UpdateVec3((char*)"albedo", glm::vec3(0.5f, 0.0f, 0.0f), pbrprogram);
		index = 0;
		for (int y = 0; y < side; y++)
		{
			UpdateFloat((char*)"metallic", (float)(side - y) / side, pbrprogram);
			for (int x = 0; x < side; x++)
			{
				//Update model matrix
				gameEntities[index]->Update();
				UpdateFloat((char*)"roughness", glm::clamp((float)x / side, 0.025f, 1.0f), pbrprogram);
				
				//Bind material and render
				pbrMat->Bind();
				gameEntities[index]->DrawModel();
				index++;
			}
		}
	#pragma endregion
	
		// Swaps the front and back buffers of the specified window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	pbrMat->Unbind();
	delete fpsCam;
	delete sphereMesh;
	delete pbrMat;

	for (int i = 0; i < gameEntities.size(); i++)
		delete gameEntities[i];

	// Terminate all the stuff related to GLFW and exit the program using the return value EXIT_SUCCESS
	glfwTerminate();

	return EXIT_SUCCESS;
}


