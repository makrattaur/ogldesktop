
uniform sampler2D currentTexture;


void main(void)
{
	gl_FragColor = texture2D(currentTexture, gl_TexCoord[0]);
}

