uniform sampler2D sampler2d;
uniform mediump vec3 LightPosition;

varying mediump vec2 v_uv;
varying mediump vec3 v_normal;
varying lowp vec4 v_color;
varying mediump vec3 v_Position_ws;

varying mediump vec3 v_normal_cs;
varying mediump vec3 LightDirection_cs;
varying mediump vec3 EyeDirection_cs;

void main(void)
{
	mediump vec4 TextureColor = texture2D(sampler2d, v_uv);
	if(TextureColor.a == 0.0)
	{
		discard;
	}

	mediump vec3 LightColor =  vec3(1,1.0,1.0); // vec3(1,0.8,0.8);
	mediump float LightPower = 100.0;
	mediump float distance = 10.0; // length( LightPosition - v_Position_ws );

	mediump vec3 MaterialDiffuseColor = TextureColor.rgb;
	mediump vec3 MaterialAmbiantColor = MaterialDiffuseColor * 0.1;
	mediump vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	// Normal of the computed fragment, in camera space
	mediump vec3 n = normalize( v_normal_cs );
	// Direction of the light (from the fragment to the light)
	mediump vec3 l = normalize( LightDirection_cs );

	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	mediump float cosTheta = dot( n, l );
	cosTheta = clamp( cosTheta, 0.0, 1.0 );

	// Eye vector (towards the camera)
	mediump vec3 E = normalize(EyeDirection_cs);

	// Direction in which the triangle reflects the light
	mediump vec3 R = reflect(-l,n);

	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	mediump float cosAlpha = dot( E,R );
	cosAlpha = clamp(cosAlpha, 0.0, 1.0);
	mediump float specFactor = pow(cosAlpha, 5.0);

	mediump vec3 color =
		// Ambiant : simulates indirect lighting
		MaterialAmbiantColor +
		
		// Diffuse : "color" of the object
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +

		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor * LightPower * specFactor / (distance*distance);
	
	//gl_FragColor = vec4(0.1,0,0,0) + texture2D(sampler2d, v_uv) * abs(vec4(v_normal, 1.0));
	
	gl_FragColor = vec4(color, TextureColor.a);

	//gl_FragColor = abs(vec4(v_normal, 1.0));

	//gl_FragColor = texture2D(sampler2d, v_uv);
}
