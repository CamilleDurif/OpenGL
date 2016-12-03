#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    


void rotate(glm::vec3 & inputvec, glm::vec3 rotation_axis, float theta, bool tonormalize = false)
{
	const float cos_theta = cos(theta);
	const float dot = glm::dot(inputvec, rotation_axis);
	glm::vec3 cross = glm::cross(inputvec, rotation_axis);

	inputvec.x *= cos_theta; inputvec.y *= cos_theta; inputvec.z *= cos_theta;
	inputvec.x += rotation_axis.x * dot * (float)(1.0 - cos_theta);
	inputvec.y += rotation_axis.y * dot * (float)(1.0 - cos_theta);
	inputvec.z += rotation_axis.z * dot * (float)(1.0 - cos_theta);

	inputvec.x -= cross.x * sin(theta);
	inputvec.y -= cross.y * sin(theta);
	inputvec.z -= cross.z * sin(theta);

	if (tonormalize) inputvec = glm::normalize(inputvec);
}


//Creates a texture buffer on the GPU, uploads the image file and returns the opengl id of the buffer.
GLuint createTextureBuffer(GLubyte *mytexture, int width, int height)
{
	GLuint texName;
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mytexture);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texName;
}

//Reads a .PPM image file and returns it in a byte array together with its image width and height.
GLubyte * readPPMfile(char *filename, int & width, int & height)
{
	FILE *inFile;
	char buffer[100];
	unsigned char c;
	int maxVal;

	GLubyte *mytexture;
	int pixelsize;

	if ((inFile = fopen(filename, "rb")) == NULL) {
		return 0;
	}

	//Read file type identifier (magic number)
	fgets(buffer, sizeof(buffer), inFile);
	if ((buffer[0] != 'P') || (buffer[1] != '6')) {
		fprintf(stderr, "not a binary ppm file %s\n", filename);
		return 0;
	}

	if (buffer[2] == 'A')
		pixelsize = 4;
	else
		pixelsize = 3;

	//Read image size
	do fgets(buffer, sizeof(buffer), inFile);
	while (buffer[0] == '#');
	sscanf(buffer, "%d %d", &width, &height);

	//Read maximum pixel value (usually 255)
	do fgets(buffer, sizeof(buffer), inFile);
	while (buffer[0] == '#');
	sscanf(buffer, "%d", &maxVal);

	//Allocate RGBA texture buffer
	int memSize = width * height * 4 * sizeof(GLubyte);
	mytexture = new GLubyte[memSize];

	// read RGB data and set alpha value
	for (int i = 0; i < memSize; i++) {
		if ((i % 4) < 3 || pixelsize == 4) {
			c = fgetc(inFile);
			mytexture[i] = (GLubyte)c;
		}
		else mytexture[i] = (GLubyte)255; //Set alpha to opaque
	}
	fclose(inFile);
	return mytexture;
}

GLuint readTexture(char *filename)
{
	FILE *inFile;
	char buffer[100];
	GLubyte *mytexture;
	unsigned char c;
	int maxVal;

	int pixelsize;
	int width, height;

	GLuint texName;

	if ((inFile = fopen(filename, "rb")) == NULL) {
		return 0;
	}

	//Read file type identifier (magic number)
	fgets(buffer, sizeof(buffer), inFile);
	if ((buffer[0] != 'P') || (buffer[1] != '6')) {
		fprintf(stderr, "not a binary ppm file %s\n", filename);
		return 0;
	}

	if (buffer[2] == 'A')
		pixelsize = 4;
	else
		pixelsize = 3;

	//Read image size
	do fgets(buffer, sizeof(buffer), inFile);
	while (buffer[0] == '#');
	sscanf(buffer, "%d %d", &width, &height);

	//Read maximum pixel value (usually 255)
	do fgets(buffer, sizeof(buffer), inFile);
	while (buffer[0] == '#');
	sscanf(buffer, "%d", &maxVal);

	//Allocate RGBA texture buffer
	int memSize = width * height * 4 * sizeof(GLubyte);
	mytexture = new GLubyte[memSize];

	// read RGB data and set alpha value
	for (int i = 0; i < memSize; i++) {
		if ((i % 4) < 3 || pixelsize == 4) {
			c = fgetc(inFile);
			mytexture[i] = (GLubyte)c;
		}
		else mytexture[i] = (GLubyte)255; //Set alpha to opaque
	}
	fclose(inFile);

	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ; 
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ; 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mytexture);

	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] mytexture;
	return texName;
}

void setupTexture(GLuint& texture) {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void setupTexture(GLuint& texture, SDL_Surface *s) {
	setupTexture(texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, s->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
}

void deleteTexture(GLuint& texture) {
	glDeleteTextures(1, &texture);
}

void setupCubeMap(GLuint& texture) {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void setupCubeMap(GLuint& texture, SDL_Surface *xpos, SDL_Surface *xneg, SDL_Surface *ypos, SDL_Surface *yneg, SDL_Surface *zpos, SDL_Surface *zneg) {
	setupCubeMap(texture);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, xpos->w, xpos->h, 0, xpos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, xpos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, xneg->w, xneg->h, 0, xneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, xneg->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, ypos->w, ypos->h, 0, ypos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ypos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, yneg->w, yneg->h, 0, yneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, yneg->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, zpos->w, zpos->h, 0, zpos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, zpos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, zneg->w, zneg->h, 0, zneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, zneg->pixels);
}

void deleteCubeMap(GLuint& texture) {
	glDeleteTextures(1, &texture);
}

void saveTGA(unsigned char* buffer, int width, int height, bool video) {
	static int i = 0;
	std::stringstream out;
	if (video) {
		if (i < 10)
			out << "video000" << (i++) << ".tga";
		else if (i < 100)
			out << "video00" << (i++) << ".tga";
		else if (i < 1000)
			out << "video0" << (i++) << ".tga";
		else if (i < 10000)
			out << "video" << (i++) << ".tga";
	}
	else {
		out << "capture" << (i++) << ".tga";
	}
	std::string s = out.str();

	glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
	std::fstream of(s.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	char header[18] = { 0 };
	header[2] = 2;
	header[12] = width & 0xff;
	header[13] = width >> 8;
	header[14] = height & 0xff;
	header[15] = height >> 8;
	header[16] = 32;
	of.write(header, 18);
	of.write((char *)buffer, width * height * 4);
}