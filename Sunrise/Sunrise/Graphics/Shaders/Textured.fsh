uniform sampler2D sampler2d;

varying mediump vec2 v_uv;
varying mediump vec3 v_normal;
varying lowp vec4 v_color;

void main(void)
{
	gl_FragColor = texture2D(sampler2d, v_uv) * abs(vec4(v_normal, 1.0));
}
