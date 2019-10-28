#version 450

in layout(location=0) vec2 vpos;
out layout(location=0) vec2 col;

layout(binding=0, std140) uniform Test {
	vec2 mask;
};

//uniform layout(binding=0) sampler2D test;

void main() {
	//col = mask * /*texture(test,*/ vpos * 0.5 + 0.5/*).xy*/;
	col = mask * (vpos * 0.5 + 0.5);
    gl_Position = vec4(vpos, 0, 1);
}