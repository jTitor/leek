//GLSL color fragment shader.
//We want to compile for GL 4.0
#version 400

//inputs - takes what vert shader outputs
in vec2 texCoord;

//outputs
out vec4 outputColor;

//sampler for a given texture.
uniform sampler2D diffTex;

//now for the actual program...
void main(void)
{

	//get the mapped texel
	vec4 texColor = texture(diffTex, vec2(texCoord.x, 1.0f - texCoord.y));

	//just place the color on the screen
	outputColor = vec4(1, 1, 1, texColor.r);
}