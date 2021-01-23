struct VertexInput
{
	float2 pos : POSITION;
	float4 color : COLOR;
};

struct VertexOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.pos = float4((input.pos + float2(64, 64)) / float2(256, 96) - float2(1, 1), 0, 1);
	output.color = input.color;
	return output;
}