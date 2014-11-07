

//uniform sampler2D currentTexture;
uniform sampler2D textureY;
uniform sampler2D textureCb;
uniform sampler2D textureCr;


void main(void)
{
	vec2 texCoords = vec2(gl_TexCoord[0].s, 1.0f - gl_TexCoord[0].t);
	
	float y = texture2D(textureY, texCoords).r;
	float cb = texture2D(textureCb, texCoords).r;
	float cr = texture2D(textureCr, texCoords).r;
/*
	y *= (65535.0 / 1023.0);
	cb *= (65535.0 / 1023.0);
	cr *= (65535.0 / 1023.0);
	
	y = clamp(y, 0.0, 1.0);
	cb = clamp(cb, 0.0, 1.0);
	cr = clamp(cr, 0.0, 1.0);
*/
	float mcb = cb - 0.5;
	float mcr = cr - 0.5;
/*
	float mcb = 0.0;
	float mcr = 0.0;
*/
	// Y in [0; 1], Cb and Cr in [-0.5; 0.5]
		
	// BT.601
	//float r = clamp(y + 1.403 * mcr, 0.0, 1.0);
	//float g = clamp(y - 0.344 * mcb - 0.714 * mcr, 0.0, 1.0);
	//float b = clamp(y + 1.772 * mcb, 0.0, 1.0);
	
	// BT.709
	float r = clamp(y + 1.5701 * mcr, 0.0, 1.0);
	float g = clamp(y - 0.1870 * mcb - 0.4664 * mcr, 0.0, 1.0);
	float b = clamp(y + 1.8556 * mcb, 0.0, 1.0);
	
	vec3 finalRGB = vec3(r, g, b);
	
	// rgb in range [16; 235], convert to full-range [0; 255]
	finalRGB = clamp(finalRGB - (16.0 / 255.0), 0.0, 1.0);
	finalRGB = clamp(finalRGB * (255.0 / 219.0), 0.0, 1.0);
	
	gl_FragColor = vec4(finalRGB, 1.0);
}

