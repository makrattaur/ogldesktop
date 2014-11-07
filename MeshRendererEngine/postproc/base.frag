
uniform sampler2D screenTexture;
uniform int texWidth;
uniform int texHeight;

void main(void)
{
	float stepX = 1.0 / texWidth;
	float stepY = 1.0 / texHeight;

	vec3 texColor = texture2D(screenTexture, gl_TexCoord[0]);
	
	gl_FragColor = vec4(texColor, 1.0);
}

