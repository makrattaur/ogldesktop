
varying vec3 interpNormal;
varying vec3 interpWorldVertexPos;
//uniform vec3 eyeDir;
uniform vec3 eyePos;

void main(void)
{
	//gl_FragColor = vec4(normalize(interpNormal) / 2.0 + 0.5, 1.0);
	//gl_FragColor = vec4(normalize(abs(interpNormal)), 1.0);
	//gl_FragColor = vec4(vec3(dot(interpNormal, eyeDir)), 1.0);
	
	vec3 viewDir = normalize(eyePos - interpWorldVertexPos);
	//float dpr = dot(viewDir, normalize(interpNormal));
	//float dpr = dot(vec3(0, 0, 1), interpNormal);
	float dpr = dot(normalize(interpNormal), viewDir);
	
	//gl_FragColor = vec4(vec3(dpr * abs(apr)), 1.0);
/*
	if(dpr < 0)
	{
		gl_FragColor = vec4(0.0, -dpr, 0.0, 1.0);
	}
	else
	{
		gl_FragColor = vec4(dpr, 0.0, 0.0, 1.0);
	}
*/
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0) * max(dpr, 0.0), 1.0);
	//gl_FragColor = vec4(vec3(dpr), 1.0);
	
	//vec3 col = (eyePos * 10.0) / 255.0;
	//gl_FragColor = vec4(col, 1.0);
}

