#pragma once

#include <GL/glew.h>
#include <string>

class myMaterial
{
public:
	GLfloat material_Ka[4]; //couleur lampe ambiant
	GLfloat material_Kd[4]; //couleur effet diffus
	GLfloat material_Ks[4]; //couleur effet spéculaire
	GLfloat material_Sh; //coefficient de brillance (shininess)
	//string material_name; // nom du matériau

};



