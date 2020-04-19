Texture2D Texture;
SamplerState Sample;

struct PixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
	return Texture.Sample(Sample, input.tex);
}