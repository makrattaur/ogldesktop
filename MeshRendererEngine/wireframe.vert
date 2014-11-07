
attribute vec3 barycentric;
varying vec3 interpBarycentric;

void main(void)
{
	interpBarycentric = barycentric;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

