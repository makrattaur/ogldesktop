
varying vec3 interpBarycentric;
uniform sampler2D currentTexture;
uniform float lineWidth;
uniform vec3 lineColor;


float edgeFactor()
{
	// calculate the sum of the derivatives in x and y,
	// which relates the speed of change in x and y with
	// the size of the triangle itself and on the screen.
    vec3 deriv = fwidth(interpBarycentric);
	
	// interpolate using smoothstep between [0.0, derivative]
	// using interpBarycentric as input.
    vec3 interp = smoothstep(vec3(0.0), deriv * lineWidth, interpBarycentric);
	
	// take the smallest value.
	// each component in the interpolated value reaches its minimum
	// value at the corresponding edge of the triangle.
    return min(min(interp.x, interp.y), interp.z);
}

vec4 GetWireframe()
{
	if(any(lessThan(interpBarycentric, vec3(0.02))))
	{
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		return vec4(1.0, 1.0, 1.0, 1.0);
	}
}

vec4 GetWireframeMixTex()
{
	vec3 texColor = texture2D(currentTexture, gl_TexCoord[0]);
	float mixFactor = edgeFactor();
	
	return vec4((texColor * (mixFactor)) + (lineColor * (1.0 - mixFactor)), 1.0);
}

void main(void)
{
	//gl_FragColor = vec4(interpBarycentric, 1.0);
	gl_FragColor = GetWireframeMixTex();
	//gl_FragColor = vec4(vec3((interpBarycentric.x + interpBarycentric.y + interpBarycentric.z) / 3.0), 1.0);
	//gl_FragColor = vec4(vec3(min(min(interpBarycentric.x, interpBarycentric.y), interpBarycentric.z)) * 2.0, 1.0);
}

