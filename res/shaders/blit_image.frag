#version 450

in layout(location=0) vec2 uv;
out layout(location=0) vec4 color;

layout(binding=0) uniform sampler2DMS imageMS;

layout(binding=0, std140) uniform blitCommand {
	vec4 start;		//.xy = src, .zw = dst / vp
	vec4 dimension;	//.xy = src, .zw = dst / vp
	uint samples;
};

//Resolve a MSAA sample

vec4 resolveMSAA(ivec2 coord){

	vec4 resolved = texelFetch(imageMS, coord, 0);

	for(int i = 1; i < samples; ++i)
		resolved += texelFetch(imageMS, coord, i);

	return resolved / float(samples); 
}

void main() {
	color = resolveMSAA(ivec2(uv));
}