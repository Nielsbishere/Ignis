#version 450

vec2 data[] = {
	vec2(0, 0),
	vec2(0, 1),
	vec2(1, 1),
	vec2(1, 1),
	vec2(1, 0),
	vec2(0, 0)
};

out layout(location = 0) vec2 srcSpace;

layout(binding=0, std140) uniform blitCommand {
	vec4 start;		//.xy = src, .zw = dst / vp
	vec4 dimension;	//.xy = src, .zw = dst / vp
	uint samples;
};

void main() {

	vec2 uvLoc = data[gl_VertexID];

	//Convert uv to src space
	srcSpace = uvLoc * dimension.xy + start.xy;

	//Convert position to dst viewport space
	vec2 dstVPSpace = uvLoc * dimension.zw + start.zw;

	//Convert to clip space
    gl_Position = vec4(dstVPSpace * 2 - 1, 0, 1);
}