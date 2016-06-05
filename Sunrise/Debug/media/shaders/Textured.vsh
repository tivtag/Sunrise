attribute highp   vec4 a_vertex;
attribute lowp    vec4 a_color;
attribute mediump vec4 a_uv;
attribute mediump vec4 a_normal;
											
uniform mediump mat4 ModelViewProj;

varying lowp    vec4 v_color;
varying mediump vec2 v_uv;
varying mediump vec3 v_normal;
								
void main(void)
{
	gl_Position = ModelViewProj * a_vertex;

	v_color = a_color;
	v_uv = a_uv.st;
	v_normal = a_normal.xyz;
}
