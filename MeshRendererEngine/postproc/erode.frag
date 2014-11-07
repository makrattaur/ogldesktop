
uniform sampler2D screenTexture;
uniform int texWidth;
uniform int texHeight;


void main(void)
{
	float stepX = 1.0 / texWidth;
	float stepY = 1.0 / texHeight;

	vec2 texStep = vec2(stepX, stepY);
/*
	11 21 31
	12 22 32
	13 23 33
*/
	float m11 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2(-1,  1));
	float m21 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 0,  1));
	float m31 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 1,  1));

	float m12 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2(-1,  0));
	float m22 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 0,  0));
	float m32 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 1,  0));

	float m13 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2(-1, -1));
	float m23 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 0, -1));
	float m33 = texture2D(screenTexture, gl_TexCoord[0] + texStep * vec2( 1, -1));
	
	float value = min(min(
		min(min(m11, m21), m31),
		min(min(m12, m22), m33)),
		min(min(m12, m22), m33));
	
	gl_FragColor = vec4(vec3(value), 1.0);
}

