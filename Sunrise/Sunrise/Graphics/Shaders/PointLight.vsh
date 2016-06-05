attribute highp   vec4 a_vertex;
attribute lowp    vec4 a_color;
attribute mediump vec4 a_uv;
attribute mediump vec4 a_normal;
			
uniform mediump mat4 Model;
uniform mediump mat4 View;								
uniform mediump mat4 ModelViewProj;
uniform mediump vec3 LightPosition;

varying lowp    vec4 v_color;
varying mediump vec2 v_uv;
varying mediump vec3 v_normal;
varying mediump vec3 v_Position_ws;

varying mediump vec3 v_normal_cs;
varying mediump vec3 EyeDirection_cs;
varying mediump vec3 LightDirection_cs;

void main(void)
{
	gl_Position   = ModelViewProj * a_vertex;
	v_Position_ws = (Model * a_vertex).xyz;

	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	mediump vec3 vertexPosition_cs = (View * Model * a_vertex).xyz;
	EyeDirection_cs = vec3(0,0,0) - vertexPosition_cs;
 
	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	mediump vec3 LightPosition_cs = (View * vec4(LightPosition, 1)).xyz;
	LightDirection_cs = LightPosition_cs + EyeDirection_cs;
 
	// Normal of the the vertex, in camera space
	v_normal_cs = (View * Model * vec4(a_normal.xyz,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	
	v_color = a_color;
	v_uv = a_uv.st;
	v_normal = a_normal.xyz;
}
