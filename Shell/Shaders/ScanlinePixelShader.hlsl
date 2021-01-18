Texture2D Texture;
SamplerState Sample;

struct PixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 phase : TEXCOORD1;
};

#define PI 3.1415926

float4 main(PixelInput input) : SV_TARGET
{
	float4 color = Texture.Sample(Sample, input.tex);

	float scanline = 0.9 + 0.1 * sin(input.phase.y);

	float4 brightness = 0.95 + 0.05 * float4(sin(float3(input.phase.x, input.phase.x - 0.66666666 * PI, input.phase.x - 1.33333333 * PI)), 1);

	return (color + float4(0.05, 0.05, 0.05, 1)) * brightness * scanline;
}