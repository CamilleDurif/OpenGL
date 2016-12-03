#version 330 core

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

//layout(location = 0) in vec4 myvertex;
in vec4 myvertex;
in vec3 mynormal;
in vec2 mytexture;
in vec3 mytangent;

out vec4 final_color;

uniform sampler2D tex;
uniform sampler2D normaltex;
uniform sampler2D caustex;

uniform vec4 light_colors[16];
uniform vec4 light_positions[16];
uniform vec3 light_directions[16];
uniform int light_types[16];
uniform int numberofLights_shader;

uniform vec4 Kd;
uniform vec4 Ks;
uniform vec4 Ka;
uniform float Ns;

uniform int to_draw = 2;

/*-------------------------------------------OLD-------------------------
vec3 light_position = vec3(1, 1, 1);
vec4 light_color = vec4(1,1,1,1);

vec4 kd = vec4(1.0,0,0,0);
vec4 ks = vec4(1,1,1,0);
vec4 ka = vec4(0.1,0.1,0.1,1);

vec3 light_position = vec3(1, 1, 1);
vec4 light_color = vec4(1,1,1,1);
--------------------------------------------------------------------------*/

vec4 kd = vec4(1.0,0,0,0);
vec4 ka = vec4(0.5,0.5,0.5,1);

vec4 light_position1 = vec4(0,0,2,1);
vec4 light_color1 = vec4(1,1,1,0);

bool bump = true;

vec4 assignColor(vec3 mylightpos_use, vec4 mylightcolor_use, vec3 mynormal_use, vec3 eyepos_use, vec3 mypos_use, vec4 kd)
{
	vec3 mypos_to_eyepos = normalize(eyepos_use - mypos_use);
	vec3 mypos_to_lightpos = normalize( mylightpos_use - mypos_use);

	return mylightcolor_use * kd * max(dot(mynormal_use, mypos_to_lightpos), 0);
}


vec3 light_position = vec3(1, 1, 1);

vec4 texture_color = texture(tex, mytexture.st);
vec4 caustex_color = texture(caustex, mytexture.st);

vec4 bl = vec4(0,0.5,0.5,0);

void main (void)
{   

	if (to_draw == 2)
		{
			final_color = texture_color * bl;
			return;
		}

	if(bump){//...........................................START WITH BUMP...........................................................................
	vec3 eyepos = vec3(0,0,0);

    vec4 _pos = myview_matrix * myvertex;
	vec3 pos = _pos.xyz / _pos.w;

	vec3 normal = normalize(mynormal_matrix * mynormal);
	
	final_color = Ka * (texture_color + caustex_color);

	vec3 t = normalize ( mynormal_matrix * mytangent);
	vec3 b = normalize (cross(normal,t));
	mat3 in_m = mat3(t,b,normal);
	mat3 out_m = transpose(in_m);

	for(int i=0; i < numberofLights_shader; i++){
	//-------------------------------------------lumiere positionnelle------------------------------------------
		if(light_types[i]==0){

		vec4 _lightpos = myview_matrix * light_positions[i];
		vec3 lightpos = _lightpos.xyz / _lightpos.w;
		
		vec3 light_to_pos = normalize(pos - lightpos);
				
		lightpos = out_m * lightpos;
		eyepos = out_m * eyepos;
		normal = normalize (2.0 * texture2D(normaltex, mytexture.st).rgb - 1.0f);

		vec3 pos_to_eyepos = normalize(eyepos - pos);
		vec3 pos_to_lightpos = normalize(lightpos - pos);

		float cos_theta = max( dot(normal, -light_to_pos), 0.0 );
		vec3 r = normalize(reflect(light_to_pos, normal));

		final_color += light_colors[i] * Kd * cos_theta * texture_color * max(dot(normal, pos_to_lightpos), 0);
		final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns) * max(dot(normal, pos_to_lightpos), 0);	

		//final_color = assignColor(lightpos, light_colors[i], normal, eyepos, pos, texture(tex, mytexture.st));

		//final_color = caustex_color;

		}

		//------------------------lumiere directionnelle---------------------------------------------------------------
		else if(light_types[i]==1){
	
			//vec3 light_to_pos = normalize(mynormal_matrix * light_directions[i]);
		
			////lightpos = out_m * lightpos;
			//eyepos = out_m * eyepos; //eye position
			////normal = normalize (2.0 * texture2D(normaltex, mytexture.st).rgb - 1.0f); //normal with bump

			//vec3 pos_to_eyepos = normalize(eyepos - pos);
			////vec3 mypos_to_lightpos = normalize(lightpos - pos);

			//float cos_theta = max( dot(normal, -light_to_pos), 0.0 );
			//vec3 r = normalize(reflect(light_to_pos, normal));

			//final_color += light_colors[i] * Kd * cos_theta * texture_color;
			//final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns);

			vec3 light_to_pos = normalize(mynormal_matrix * light_directions[i]);

			vec3 r = normalize(reflect(light_to_pos, normal));

			vec3 pos_to_eyepos = normalize( eyepos - pos );

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

		
			final_color += light_colors[i] * Kd * cos_theta* texture_color;
			final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns);
	
		}
		//--------------------------------lumière spot------------------------------------------------------------------
		else if(light_types[i]==2){
	
			vec4 _lightpos = myview_matrix * light_positions[i];
			//vec4 _lightpos = light_positions[i] * myview_matrix;
			vec3 lightpos = _lightpos.xyz / _lightpos.w;

			vec3 light_to_pos = normalize(pos - lightpos);
			vec3 direction = normalize(mynormal_matrix * light_directions[i]);

			lightpos = out_m * lightpos;
			eyepos = out_m * eyepos;
			normal = normalize (2.0 * texture2D(normaltex, mytexture.st).rgb - 1.0f);

			vec3 pos_to_eyepos = normalize(eyepos - pos);
			vec3 pos_to_lightpos = normalize(lightpos - pos);

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );
			float cos_alpha = max( dot(light_to_pos, direction), 0.0);
			vec3 r = normalize(reflect(light_to_pos, normal));

			final_color += light_colors[i] * Kd * cos_theta * cos_alpha * texture_color * max(dot(normal, pos_to_lightpos), 0);
			final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns) * max(dot(normal, pos_to_lightpos), 0) * cos_alpha;
		}
		}
	}
	//..........................................................END WITH BUMP............................................................................
	else {
	vec3 eyepos = vec3(0,0,0);

    vec4 _pos = myview_matrix * myvertex;
	vec3 pos = _pos.xyz / _pos.w;

	vec3 normal = normalize(mynormal_matrix * mynormal);
	
	final_color = Ka;

	for(int i=0; i < numberofLights_shader; i++){
	//-------------------------------------------lumiere positionnelle------------------------------------------
		if(light_types[i]==0){

		//vec3 lightpos = myview_matrix * light_positions[i];
		vec4 _lightpos = myview_matrix * light_positions[i];
		vec3 lightpos = _lightpos.xyz / _lightpos.w;
		
		vec3 light_to_pos = normalize(pos - lightpos);

		vec3 r = normalize(reflect(light_to_pos, normal));

		vec3 pos_to_eyepos = normalize( eyepos - pos );

		float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

		final_color += light_colors[i] * Kd * cos_theta * texture_color;
		final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns);
	
		}

		//------------------------lumiere directionnelle---------------------------------------------------------------
		else if(light_types[i]==1){
	
			vec3 light_to_pos = normalize(mynormal_matrix * light_directions[i]);

			vec3 r = normalize(reflect(light_to_pos, normal));

			vec3 pos_to_eyepos = normalize( eyepos - pos );

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

		
			final_color += light_colors[i] * Kd * cos_theta* texture_color;
			final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns);
	
		}
		//--------------------------------lumière spot------------------------------------------------------------------
		else if(light_types[i]==2){
	
			vec4 _lightpos = myview_matrix * light_positions[i];
			//vec4 _lightpos = light_positions[i] * myview_matrix;
			vec3 lightpos = _lightpos.xyz / _lightpos.w;

			vec3 light_to_pos = normalize(pos - lightpos);
			vec3 direction = normalize(mynormal_matrix * light_directions[i]);

			vec3 r = normalize(reflect(light_to_pos, normal));

			float cos_alpha = max( dot(light_to_pos, direction), 0.0);
		
			vec3 pos_to_eyepos = normalize( eyepos - pos );

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

			final_color += light_colors[i] * Kd * cos_theta * cos_alpha* texture_color;
			final_color += light_colors[i] * Ks * pow(max( dot(r, pos_to_eyepos), 0.0), Ns) * cos_alpha;
		}

	}}
	
}
/*
#version 330 core

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

in vec4 myvertex;
in vec3 mynormal;

out vec4 final_color;

vec4 kd = vec4(1.0,0,0,0);
vec4 ks = vec4(1,1,1,0);
vec4 ka = vec4(0.1,0.1,0.1,1);

vec3 light_position = vec3(1, 1, 1);
vec4 light_color = vec4(1,1,1,1);

void main (void)
{   
	vec3 eyepos = vec3(0,0,0);

    vec4 _pos = myview_matrix * myvertex;
	vec3 pos = _pos.xyz / _pos.w;

	vec3 normal = normalize(mynormal_matrix * mynormal);

	vec4 _lightpos = myview_matrix * vec4(light_position, 1);
	vec3 lightpos = _lightpos.xyz / _lightpos.w;
	//vec3 lightpos = light_position;

	vec3 light_to_pos = normalize(pos - lightpos);

	vec3 r = normalize(reflect(light_to_pos, normal));

	vec3 pos_to_eyepos = normalize( eyepos - pos );

	float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

	final_color = ka;
	final_color += light_color * kd * cos_theta;
	final_color += light_color * ks * 
				   pow(max( dot(r, pos_to_eyepos), 0.0), 10);
}


vec4 assignColor(vec3 mylightpos_use, vec4 mylightcolor_use, vec3 mynormal_use, vec3 eyepos_use, vec3 mypos_use, vec4 kd)
{
	vec3 mypos_to_eyepos = normalize(eyepos_use - mypos_use);
	vec3 mypos_to_lightpos = normalize( mylightpos_use - mypos_use);

	return kd;
}

//----------------CODE POUR BUMP--------------------------------
void main (void)
{   
	vec3 eyepos = vec3(0,0,0);

	vec4 _mypos = myview_matrix * myvertex;
	vec3 mypos = _mypos.xyz / _mypos.w;

	vec4 _lightpos =  light_position1;
	vec3 lightpos = _lightpos.xyz / _lightpos.w;

	vec3 normal = normalize( mynormal_matrix * mynormal );

	//start: additional stuff for bump mapping
	vec3 t = normalize ( mynormal_matrix * mytangent);
	vec3 b = normalize (cross(normal,t));
	mat3 in_m = mat3(t,b,normal);
	mat3 out_m = transpose(in_m);

	lightpos = out_m * lightpos;
	eyepos = out_m * eyepos;
	normal = normalize (2.0 * texture2D(normaltex, mytexture.st).rgb - 1.0f);
	//end: additional stuff for bump mapping

	final_color = assignColor(lightpos, light_color1, normal, eyepos, mypos, texture(tex, mytexture.st));
}

*/