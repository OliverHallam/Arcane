struct VertexInput
{
	float2 pos : POSITION;
	float2 tex : TEXCOORD;
};

struct VertexOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

#define SCANLINE_HEIGHT 240
#define SCANLINE_WIDTH 256

VertexOutput main(VertexInput input)
{
	VertexOutput output;

	output.pos = float4(input.pos, 0, 1);

	float2 normalized = input.tex / 2 + float2(0.5f, 0.5f);
	output.tex = float2(normalized.x, 1-normalized.y);


	return output;
}