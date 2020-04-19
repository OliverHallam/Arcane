struct VertexInput
{
	float2 pos : POSITION;
};

struct VertexOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;

	output.pos = float4(input.pos, 0, 1);

	float2 normalized = input.pos / 2 + float2(0.5f, 0.5f);
	output.tex = float2(normalized.x, -normalized.y);

	return output;
}