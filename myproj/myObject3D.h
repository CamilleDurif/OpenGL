#include <math.h>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <fstream>
#include <myMaterial.h>
#include <mySubObject3D.h>
#include <myTexture.h>
#define PI 3.14159265

using namespace std;

class myObject3D
{
public:
	GLuint buffers[6];
	GLuint myshaderprogram; //Id of program shader, in case of multiple shaders
	enum { VERTEX_BUFFER = 0, INDEX_BUFFER, NORMAL_BUFFER, TEXTURE_BUFFER, BUMP_BUFFER };

	myMaterial *material; //contient la description de la couleur de l'objet*

	GLuint texid;
	GLuint normaltexid;

	myTexture *texture;
	myTexture *bump_tex;

	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> indices;
	std::vector<glm::vec3> normals;
	std::vector<mySubObject3D *> parts; //contains subparts of the scene
	std::vector<glm::vec2> texturecoordinates;
	std::vector<glm::vec3> tangents;

	glm::mat4 model_matrix;

	myObject3D() {
		model_matrix = glm::mat4(1.0f);
		clear();
	}

	~myObject3D()
	{
		clear();
	}

	void clear() {		
		vertices.clear();
		indices.clear();
		normals.clear();
		texturecoordinates.clear();
		tangents.clear();
		glDeleteBuffers(6, buffers);
	}

	void normalize()
	{
		unsigned int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

		for (unsigned i = 0; i<vertices.size(); i++) {
			if (vertices[i].x < vertices[tmpxmin].x) tmpxmin = i;
			if (vertices[i].x > vertices[tmpxmax].x) tmpxmax = i;

			if (vertices[i].y < vertices[tmpymin].y) tmpymin = i;
			if (vertices[i].y > vertices[tmpymax].y) tmpymax = i;

			if (vertices[i].z < vertices[tmpzmin].z) tmpzmin = i;
			if (vertices[i].z > vertices[tmpzmax].z) tmpzmax = i;
		}

		float xmin = vertices[tmpxmin].x, xmax = vertices[tmpxmax].x,
			   ymin = vertices[tmpymin].y, ymax = vertices[tmpymax].y,
			   zmin = vertices[tmpzmin].z, zmax = vertices[tmpzmax].z;

		float scale = (xmax - xmin) <= (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
		scale = scale >= (zmax - zmin) ? scale : (zmax - zmin);

		for (unsigned int i = 0; i<vertices.size(); i++) {
			vertices[i].x -= (xmax + xmin) / 2;
			vertices[i].y -= (ymax + ymin) / 2;
			vertices[i].z -= (zmax + zmin) / 2;

			vertices[i].x /= scale;
			vertices[i].y /= scale;
			vertices[i].z /= scale;
		}
	}


	bool readMesh(string filename)
	{
		clear();
		string s, t;
		float x, y, z;
		int index1, index2, index3;

		ifstream fin(filename);
		if (!fin.is_open())
		{
			cout << "Error: unable to open file in readMesh().\n";
			return false;
		}

		while (getline(fin, s))
		{
			stringstream myline(s);
			myline >> t;
			if (t == "v")
			{
				myline >> x;  myline >> y; myline >> z;
				vertices.push_back(glm::vec3(x, y, z));
			}
			else if (t == "f")
			{
				myline >> t; index1 = atoi((t.substr(0, t.find("/"))).c_str());
				myline >> t; index2 = atoi((t.substr(0, t.find("/"))).c_str());
				while (myline >> t)
				{
					index3 = atoi((t.substr(0, t.find("/"))).c_str());
					indices.push_back(glm::ivec3(index1-1, index2-1, index3-1));
					index2 = index3;
				}
			}
		}
		return true;
	}
 
	void computeNormals()
	{
		normals.assign(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));
		for (unsigned int i = 0; i<indices.size(); i++)
		{
			glm::vec3 face_normal = glm::cross( vertices[indices[i][1]] - vertices[indices[i][0]], vertices[indices[i][2]] - vertices[indices[i][1]] );
			normals[indices[i][0]] += face_normal;
			normals[indices[i][1]] += face_normal;
			normals[indices[i][2]] += face_normal;
		}
		for (unsigned int i = 0; i < vertices.size(); i++)  normals[i] = glm::normalize(normals[i]);
	}

	void createObjectBuffers()
	{
		glGenBuffers(4, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*12, &vertices.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*12, &indices.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, normals.size()*12, &normals.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXTURE_BUFFER]);
 		glBufferData(GL_ARRAY_BUFFER, texturecoordinates.size() * 8, &texturecoordinates.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[BUMP_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, tangents.size() * 12, &tangents.front(), GL_STATIC_DRAW);


	}

	void displayObject(GLuint shaderprogram1, int render_method = 1)
	{
		if (render_method == 1)
		{
			glBegin(GL_TRIANGLES);
			for (unsigned int i = 0; i < indices.size(); i++)
			{
				glVertex3f(vertices[indices[i][0]].x, vertices[indices[i][0]].y, vertices[indices[i][0]].z);
				glVertex3f(vertices[indices[i][1]].x, vertices[indices[i][1]].y, vertices[indices[i][1]].z);
				glVertex3f(vertices[indices[i][2]].x, vertices[indices[i][2]].y, vertices[indices[i][2]].z);
			}
			glEnd();
		}

		else {

			glUniform4fv(glGetUniformLocation(shaderprogram1, "Kd"), 1, material->material_Kd);
			glUniform4fv(glGetUniformLocation(shaderprogram1, "Ks"), 1, material->material_Ks);
			glUniform4fv(glGetUniformLocation(shaderprogram1, "Ka"), 1, material->material_Ka);
			glUniform1f(glGetUniformLocation(shaderprogram1, "Ns"), material->material_Sh);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUFFER]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUFFER]);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXTURE_BUFFER]);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(3);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[BUMP_BUFFER]);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);
			glDrawElements(GL_TRIANGLES, indices.size()*3, GL_UNSIGNED_INT, 0);
			
		}
	}

	void displayNormals()
	{

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUFFER]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawElements(GL_LINES, normals.size(), GL_UNSIGNED_INT, 0);


	}

	void computeTexturecoordinates_plane()
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			texturecoordinates[i].x = vertices[i].x;
			texturecoordinates[i].y = vertices[i].z;
		}
	}

	void computeTexturecoordinates_cylinder()
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			float x = vertices[i].x;
			float y = vertices[i].y;
			float z = vertices[i].z;

			texturecoordinates[i].t = y - 0.5f;
			texturecoordinates[i].s = (z >= 0.0f) ? atan2(z, x) / (PI) : (-atan2(z, x)) / (PI);
		}
	}

	void computeTexturecoordinates_sphere() 
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			float x = vertices[i].x;
			float y = vertices[i].y;
			float z = vertices[i].z;
				

		}
	}

	void computeTangents()
	{
		tangents.assign(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));
		for (unsigned int i = 0; i < indices.size(); i++)
		{
			glm::vec2 t10 = texturecoordinates[indices[i][1]] - texturecoordinates[indices[i][0]];
			glm::vec2 t20 = texturecoordinates[indices[i][2]] - texturecoordinates[indices[i][0]];
			float f = t10.s*t20.t - t10.t*t20.s;
			if (f == 0) continue;
			f = 1.0f / f;
			glm::vec3 v10 = vertices[indices[i][1]] - vertices[indices[i][0]];
			glm::vec3 v20 = vertices[indices[i][2]] - vertices[indices[i][0]];
			glm::vec3 t(f*(t20.t*v10.x - t10.t*v20.x), f*(t20.t*v10.y - t10.t*v20.y),
				f*(t20.t*v10.z - t10.t*v20.z));

			tangents[indices[i][0]] += t;
			tangents[indices[i][1]] += t;
			tangents[indices[i][2]] += t;
		}
		for (unsigned int i = 0; i < vertices.size(); i++)  tangents[i] = glm::normalize(tangents[i]);
	}
};