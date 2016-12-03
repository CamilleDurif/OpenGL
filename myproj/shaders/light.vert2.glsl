#version 330 core

layout(location = 0) in vec4 vertex_modelspace;
layout(location = 1) in vec3 normal_modelspace;
layout(location = 2) in vec2 texture_modelspace;
layout(location = 3) in vec3 tangent_modelspace;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

out vec3 mynormal;
out vec4 myvertex;
out vec2 mytexture;
out vec3 mytangent;
 

void main() {
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace; 

	mynormal = normal_modelspace;
	myvertex = vertex_modelspace;
	mytexture = texture_modelspace;
	mytangent = tangent_modelspace;
}
