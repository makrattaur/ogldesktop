
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
	
/*
	Horizontal Sobel:
	-1 +0 +1
	-2 +0 +2
	-1 +0 +1
*/
/*
	Vertical Sobel:
	-1 -2 -1
	+0 +0 +0
	+1 +2 +1
*/

	float sumH =
		m11 * -1 + m21 * 0 + m31 * 1 +
		m12 * -2 + m21 * 0 + m32 * 2 +
		m13 * -1 + m23 * 0 + m33 * 1;
		
	float sumHI =
		m11 *  1 + m21 * 0 + m31 * -1 +
		m12 *  2 + m21 * 0 + m32 * -2 +
		m13 *  1 + m23 * 0 + m33 * -1;

	float sumV =
		m11 * -1 + m21 * -2 + m31 * -1 +
		m12 *  0 + m21 *  0 + m32 *  0 +
		m13 *  1 + m23 *  2 + m33 *  1;
	
	float sumVI =
		m11 *  1 + m21 *  2 + m31 *  1 +
		m12 *  0 + m21 *  0 + m32 *  0 +
		m13 * -1 + m23 * -2 + m33 * -1;
	
	//gl_FragColor = vec4(vec3(sum), 1.0);
	gl_FragColor = vec4(vec3(max(max(sumH, sumHI), max(sumV, sumVI))), 1.0);
}

