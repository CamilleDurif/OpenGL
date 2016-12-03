#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>      

#include <GL/glew.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

#include "NFD/nfd.h"

#include "shaders.h"
#include "helperFunctions.h"

#include "myObject3D.h"
#include "myLight.h"
#include "myMaterial.h"

using namespace std;


// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

int window_width = 640;
int window_height = 480;

int mouse_position[2];
bool mouse_button_pressed = false;
bool quit = false;

// Camera parameters.
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);

// Projection parameters.
float fovy = 45.0f;
float zNear = 0.2f;
float zFar = 4000;

// OpenGL shader variables
GLuint vertexshader, fragmentshader, shaderprogram1;

//Second shader program
GLuint vertexshader2, fragmentshader2, shaderprogram2;

 // Mesh object
myObject3D *obj1;

//second object
myObject3D *obj2;

//timer
int treshold = 0;
int compteur = 0;

// Process the event.  
void processEvents(SDL_Event current_event)
{
	switch (current_event.type)
	{
		// window close button is pressed
		case SDL_QUIT:
		{
			quit = true;
			break;
		}
		case SDL_KEYDOWN:
		{
			if (current_event.key.keysym.sym == SDLK_ESCAPE)
				quit = true;
			else if (current_event.key.keysym.sym == SDLK_UP)
				camera_eye += 0.1f * camera_forward;
			else if (current_event.key.keysym.sym == SDLK_DOWN)
				camera_eye -= 0.1f * camera_forward;
			else if (current_event.key.keysym.sym == SDLK_LEFT)
				rotate(camera_forward, camera_up, 0.05f, true);
			else if (current_event.key.keysym.sym == SDLK_RIGHT)
				rotate(camera_forward, camera_up, -0.05f, true);
			else if (current_event.key.keysym.sym == SDLK_o)
			{
				nfdchar_t *outPath = NULL;
				nfdresult_t result = NFD_OpenDialog("obj", NULL, &outPath);
				if (result != NFD_OKAY) return;
				myObject3D *obj_tmp = new myObject3D();
				if (!obj_tmp->readMesh(outPath))
				{
					delete obj_tmp;
					return;
				}
				delete obj1;
				obj1 = obj_tmp;
				obj1->normalize();
				obj1->computeNormals();
				obj1->createObjectBuffers();
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			mouse_position[0] = current_event.button.x;
			mouse_position[1] = window_height - current_event.button.y;
			mouse_button_pressed = true;
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			mouse_button_pressed = false;
			break;
		}
		case SDL_MOUSEMOTION:
		{
			int x = current_event.motion.x;
			int y = window_height - current_event.motion.y;

			int dx = x - mouse_position[0];
			int dy = y - mouse_position[1];

			mouse_position[0] = x;
			mouse_position[1] = y;

			if ( (dx == 0 && dy == 0) || !mouse_button_pressed ) break;

			float vx = (float)dx / (float)window_width;
			float vy = (float)dy / (float)window_height;
			float theta = 4.0f * (fabs(vx) + fabs(vy));

			glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));
			glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

				rotate(camera_forward, rotation_axis, theta, true);
				rotate(camera_up, rotation_axis, theta, true);
				rotate(camera_eye, rotation_axis, theta, false);
			}
			else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				camera_eye += 1.6f * tomovein_direction;
			}
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if (current_event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				window_width = current_event.window.data1;
				window_height = current_event.window.data2;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			if (current_event.wheel.y < 0)
				camera_eye -= 0.1f * camera_forward;
			else if (current_event.wheel.y > 0)
				camera_eye += 0.1f * camera_forward;
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	// Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Initialize video subsystem
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	// Create window
	window = SDL_CreateWindow("IT-5102E-16", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);

	// Initialize glew
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Setting up OpenGL shaders
	vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl");
	fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl");
	shaderprogram1 = initprogram(vertexshader, fragmentshader);

	//Setting second shaderprogram
	vertexshader2 = initshaders(GL_VERTEX_SHADER, "shaders/light.vert2.glsl");
	fragmentshader2 = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag2.glsl");
	shaderprogram2 = initprogram(vertexshader2, fragmentshader2);
	
	// Read up the scene
	obj1 = new myObject3D();
	if (!obj1->readMesh("hand.obj")) return 0;
	obj1->normalize();
	obj1->computeNormals();
	obj1->computeTexturecoordinates_cylinder();
	obj1->computeTangents();

	obj1->createObjectBuffers();

	//obj2
	obj2 = new myObject3D();
	if (!obj2->readMesh("mountains.obj")) return 0;
	obj2->normalize();
	obj2->computeNormals();
	obj2->computeTexturecoordinates_plane();
	obj2->computeTangents();

	obj2->createObjectBuffers();

	//obj1->texid = readTexture("wood.ppm");
	//obj1->normaltexid = readTexture("243_normal.ppm");
	
	//TEXTURE
	int height, width;
	width = 0;
	height = 0;
	GLubyte *a = readPPMfile("br_diffuse.ppm", width, height);
		std::cout << width << endl;
	GLuint texid = createTextureBuffer(a, width, height);

	//BUMP
	width = 0;
	height = 0;
	a = readPPMfile("br_normal.ppm", width, height);
	std::cout << width << endl;
	GLuint normaltexid = createTextureBuffer(a, width, height);
	
	//SAND
	
	width = 0;
	height = 0;
	GLubyte *b = readPPMfile("br_diffuse.ppm", width, height);
	std::cout << width << endl;
	GLuint sandtexid = createTextureBuffer(b, width, height);

	//CAUSTIC EFFECT
	GLubyte* caustics[15];
	string fname;
	stringstream ss;
	/*for (int i = 0; i++; i < 15) {
		ss << "caust/" << i+1 << ".ppm";
		fname = ss.str();
		std::cout << fname << endl;
		width = 0;
		height = 0;
		caustics[i] = readPPMfile((char*)fname.c_str(), width, height);
		//std::cout << width << endl;
		treshold = 0;
	}*/
	width = 0;
	height = 0;
	caustics[0] = readPPMfile("caust/1.ppm", width, height);
	width = 0;
	height = 0; caustics[1] = readPPMfile("caust/2.ppm", width, height);
	width = 0;
	height = 0;
	caustics[2] = readPPMfile("caust/3.ppm", width, height);
	width = 0;
	height = 0; caustics[3] = readPPMfile("caust/4.ppm", width, height);
	width = 0;
	height = 0;
	caustics[4] = readPPMfile("caust/5.ppm", width, height);
	width = 0;
	height = 0; caustics[5] = readPPMfile("caust/6.ppm", width, height);
	width = 0;
	height = 0;
	caustics[6] = readPPMfile("caust/7.ppm", width, height);
	width = 0;
	height = 0; caustics[7] = readPPMfile("caust/8.ppm", width, height);
	width = 0;
	height = 0;
	caustics[8] = readPPMfile("caust/9.ppm", width, height);
	width = 0;
	height = 0; caustics[9] = readPPMfile("caust/10.ppm", width, height);
	width = 0;
	height = 0;
	caustics[10] = readPPMfile("caust/11.ppm", width, height);
	width = 0;
	height = 0; caustics[11] = readPPMfile("caust/12.ppm", width, height);
	width = 0;
	height = 0;
	caustics[12] = readPPMfile("caust/13.ppm", width, height);
	width = 0;
	height = 0; caustics[13] = readPPMfile("caust/14.ppm", width, height);
	width = 0;
	height = 0;
	caustics[14] = readPPMfile("caust/15.ppm", width, height);
	width = 0;
	height = 0; caustics[15] = readPPMfile("caust/16.ppm", width, height);

	GLuint caustexid = createTextureBuffer(caustics[0], width, height);
	
	//lights
	myLight *lights = new myLight(); //list of lights

	//lampe positionnelle
	lights->addLight(glm::vec4(0, 0, 2, 1), glm::vec4(1, 1, 1, 1), glm::vec3(0, 0, 0), 0);
	
	//lampe directionnelle 1 
	//lights->addLight(glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 1, 0), glm::vec3(-1, 0, 0), 1);
	
	//lampe directionnelle 2
	//lights->addLight(glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 1, 0), glm::vec3(1, 0, 0), 1);
	
	//lampe spot
	lights->addLight(glm::vec4(0, 10, 0, 1), glm::vec4(1, 1, 1, 0), glm::vec3(0, -1, 0), 2);
	
	//material
	myMaterial* metal_1;
	metal_1 = new myMaterial();
	metal_1->material_Ka[0] = 0.2f; metal_1->material_Ka[1] = 0.2f; metal_1->material_Ka[2] = 0.2f; metal_1->material_Ka[3] = 0;
	metal_1->material_Kd[0] = 1; metal_1->material_Kd[1] = 1; metal_1->material_Kd[2] = 1; metal_1->material_Kd[3] = 0;
	metal_1->material_Ks[0] = 1; metal_1->material_Ks[1] = 1; metal_1->material_Ks[2] = 1; metal_1->material_Ks[3] = 0;
	metal_1->material_Sh = 100;

	myMaterial* metal_2;
	metal_2 = new myMaterial();
	metal_2 = metal_1;
	metal_2->material_Sh = 30;
 
	myMaterial* matte;
	matte = new myMaterial();
	matte->material_Ka[0] = 0.2f; matte->material_Ka[1] = 0.2f; matte->material_Ka[2] = 0.2f; matte->material_Ka[3] = 0;
	matte->material_Kd[0] = 0.5f; matte->material_Kd[1] = 0.5f; matte->material_Kd[2] = 0.5f; matte->material_Kd[3] = 0;
	matte->material_Ks[0] = 0.0f; matte->material_Ks[1] = 0.0f; matte->material_Ks[2] = 0.0f; matte->material_Ks[3] = 0;
	matte->material_Sh = 30;

	myMaterial* br;
	br = new myMaterial();
	br->material_Ka[0] = 0.2f; br->material_Ka[1] = 0.2f; br->material_Ka[2] = 0.2f; br->material_Ka[3] = 0;
	br->material_Kd[0] = 1.0f; br->material_Kd[1] = 1.0f; br->material_Kd[2] = 1.0f; br->material_Kd[3] = 0;
	br->material_Ks[0] = 0.1f; br->material_Ks[1] = 0.1f; br->material_Ks[2] = 0.1f; br->material_Ks[3] = 0;
	br->material_Sh = 30;

	obj1->material = br;

	obj2->material = br;

	myTexture* test;

	// Game loop
	while (!quit)
	{
		//deleteTexture(caustexid);
		treshold++;
			if (treshold > 300) {
				caustexid = createTextureBuffer(caustics[compteur], width, height);
				treshold = 0;
				compteur++;
				if (compteur == 16)
					compteur = 0;
			}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window_width, window_height);

		glm::mat4 projection_matrix = glm::perspective(glm::radians(fovy), (float) window_width / (float)window_height, zNear, zFar);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram1, "myprojection_matrix"), 1, GL_FALSE, &projection_matrix[0][0]);

		glm::mat4 view_matrix = glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram1, "myview_matrix"), 1, GL_FALSE, &view_matrix[0][0]);

		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram1, "mynormal_matrix"), 1, GL_FALSE, &normal_matrix[0][0]);

		
		//TEXTURE
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texid);
		glUniform1i(glGetUniformLocation(shaderprogram1, "tex"), 3);
		
		//CAUSTIC EFFECT
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, caustexid);
		glUniform1i(glGetUniformLocation(shaderprogram1, "caustex"), 9);
		
		//BUMP
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, normaltexid);
		glUniform1i(glGetUniformLocation(shaderprogram1, "normaltex"), 6);
		
		//LIGHTS
		glUniform1i(glGetUniformLocation(shaderprogram1, "numberofLights_shader"), lights->colors.size());		
		glUniform4fv(glGetUniformLocation(shaderprogram1, "light_positions"), lights->positions.size(), glm::value_ptr(lights->positions[0]));		
		glUniform4fv(glGetUniformLocation(shaderprogram1, "light_colors"), lights->colors.size(), glm::value_ptr(lights->colors[0]));		
		glUniform3fv(glGetUniformLocation(shaderprogram1, "light_directions"), lights->directions.size(), glm::value_ptr(lights->directions[0]));		
		glUniform1iv(glGetUniformLocation(shaderprogram1, "light_types"), lights->types.size(), &lights->types[0]);


		glUseProgram(shaderprogram2);
		//shaderprogram2
		glm::mat4 projection_matrix2 = glm::perspective(glm::radians(fovy), (float)window_width / (float)window_height, zNear, zFar);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram2, "myprojection_matrix"), 1, GL_FALSE, &projection_matrix2[0][0]);
		glm::mat4 view_matrix2 = glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram2, "myview_matrix"), 1, GL_FALSE, &view_matrix2[0][0]);
		glm::mat3 normal_matrix2 = glm::transpose(glm::inverse(glm::mat3(view_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram2, "mynormal_matrix"), 1, GL_FALSE, &normal_matrix2[0][0]);

		glUniform1i(glGetUniformLocation(shaderprogram2, "numberofLights_shader"), lights->colors.size());
		glUniform4fv(glGetUniformLocation(shaderprogram2, "light_positions"), lights->positions.size(), glm::value_ptr(lights->positions[0]));
		glUniform4fv(glGetUniformLocation(shaderprogram2, "light_colors"), lights->colors.size(), glm::value_ptr(lights->colors[0]));
		glUniform3fv(glGetUniformLocation(shaderprogram2, "light_directions"), lights->directions.size(), glm::value_ptr(lights->directions[0]));
		glUniform1iv(glGetUniformLocation(shaderprogram2, "light_types"), lights->types.size(), &lights->types[0]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, caustexid);
		glUniform1i(glGetUniformLocation(shaderprogram2, "tex"), 1);

		obj2->displayObject(shaderprogram2, 2);
		/*glBegin(GL_QUADS);
			glTexCoord2i(0, 0);      glVertex3i(-10, -10, -1);
			glTexCoord2i(10, 0);     glVertex3i(10, -10, -1);
			glTexCoord2i(10, 10);    glVertex3i(10, 10, -1);
			glTexCoord2i(0, 10);     glVertex3i(-10, 10, -1);
		glEnd();*/
		
		glUseProgram(shaderprogram1);
		//obj1->displayObject(shaderprogram1, 2);
		
		//glDisable(GL_TEXTURE_2D);
		
		//lights->drawLight(0, shaderprogram1);
		//lights->drawLight(1, shaderprogram1);
		//lights->drawLight(2, shaderprogram1);
		//lights->drawLight(3, shaderprogram1);
		
		SDL_GL_SwapWindow(window);

		SDL_Event current_event;
		while (SDL_PollEvent(&current_event) != 0)
			processEvents(current_event);
	}
	
	// Freeing resources before exiting.
	// Destroy window
	if (glContext) SDL_GL_DeleteContext(glContext);
	if (window) SDL_DestroyWindow(window);

	//Freeing up OpenGL resources.
	delete obj1;
	glDeleteProgram(shaderprogram1);
	glDeleteProgram(shaderprogram2);
	
	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}