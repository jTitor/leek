//GLSL color fragment shader.
//We want to compile for GL 4.0
#version 400

//inputs - takes what vert shader outputs
in vec3 color;

//outputs
out vec4 outputColor;

//now for the actual program...
void main(void)
{
	//just place the color on the screen
	outputColor = vec4(color, 1.0f);
}