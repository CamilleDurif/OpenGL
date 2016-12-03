#pragma once
#include <GL/glew.h>

class myLight
{
public:

	vector<glm::vec4> positions; //light position
	vector<glm::vec4> colors; //ligh color
	vector<glm::vec3> directions; //light direction
	vector<int> types; // 0 = point light, 1 = directional light, 2 = spot.

	void addLight(glm::vec4 pPosition, glm::vec4 pColor, glm::vec3 pDirection, GLuint pType) {

		positions.push_back(pPosition);
		colors.push_back(pColor);
		directions.push_back(pDirection);
		types.push_back(pType);

	}

	
	void drawLight(int i, GLuint shader) {

		glm::vec3 direction = directions[i];
		glm::vec4 position = positions[i];
		glm::vec4 color = colors[i];
		int type = types[i];

		glUniform1i(glGetUniformLocation(shader, "to_draw"), 2);

		float length = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
		if (length != 0) {
			for (int i = 0; i < 3; i++) {
				direction[i] /= length;
			}
		}
		glPointSize(6.0);
		glUniform4fv(glGetUniformLocation(shader, "Kd"), 1, glm::value_ptr(color));
		if (type == 0 || type == 2) {
			glBegin(GL_POINTS);
			glVertex3f(position[0], position[1], position[2]);
			glEnd();
		}
		if (type == 1 || type == 2) {
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(direction[0], direction[1], direction[2]);
			glEnd();
		}
		if (type == 2) {
			glBegin(GL_POINTS);
			glVertex3f(position[0], position[1], position[2]);
			glVertex3f(position[0] + direction[0] / 2.0, position[1] + direction[1] / 2.0, position[2] + direction[2] / 2.0);
			glEnd();
		}
		glUniform1i(glGetUniformLocation(shader, "to_draw"), 0);
	}
};

