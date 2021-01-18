Texture2D Texture;
SamplerState Sample;

struct PixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 phase : TEXCOORD1;
};

float4 main(PixelInput input) : SV_TARGET
{
	float4 color = Texture.Sample(Sample, input.tex);

	float brightness = 0.9 + dot(float2(0.01, 0.1), sin(input.phase));

	return (color + float4(0.05, 0.05, 0.0, 1)) * brightness;
}