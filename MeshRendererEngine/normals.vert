
varying vec3 interpNormal;
varying vec3 interpWorldVertexPos;


void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	interpNormal = normalize(gl_NormalMatrix * gl_Normal);
	//interpNormal = normalize(vec3(gl_ModelViewMatrix * vec4(gl_Normal, 0.0)));
	//interpNormal = gl_Normal;
	//interpWorldVertexPos = gl_Position;
	interpWorldVertexPos = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

