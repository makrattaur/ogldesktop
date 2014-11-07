
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
/*
	float[3 * 3] values;
	
	for(int y = 0; y < 3; y++)
	{
		for(int x = 0; x < 3; x++)
		{
			values[y * 3 + x] = texture2D(screenTexture,
				gl_TexCoord[0] + texStep * vec2(x - 1, y - 1));
		}
	}
*/
	float[5 * 5] values;
	
	for(int y = 0; y < 5; y++)
	{
		for(int x = 0; x < 5; x++)
		{
			values[y * 5 + x] = texture2D(screenTexture,
				gl_TexCoord[0] + texStep * vec2(x - 2, y - 2));
		}
	}
	
	float value = 0;
	for(int i = 0; i < values.length(); i++)
	{
		value += values[i];
	}
	value /= values.length();
	
	gl_FragColor = vec4(vec3(value), 1.0);
}

