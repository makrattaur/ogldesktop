
uniform sampler2D outlineTex;
uniform sampler2D maskTex;

void main(void)
{
	//gl_FragColor = vec4(0.0, 0.0, 0.5, 0.5);
	float outline = texture2D(outlineTex, gl_TexCoord[0]).r;
	float mask = texture2D(maskTex, gl_TexCoord[0]).r;
	float value = outline * (1.0 - mask);
	gl_FragColor = vec4(value, 0.0, 0.0, value);
}

