Texture2D Texture;
SamplerState Sample;

struct PixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

#define PI 3.1415926

#define SCANLINE_WIDTH 256
#define SCANLINE_HEIGHT 240

float4 main(PixelInput input) : SV_TARGET
{
	float textureY = (floor(input.tex.y * SCANLINE_HEIGHT) + 0.5) / SCANLINE_HEIGHT;
	float4 rSample = Texture.Sample(Sample, float2(input.tex.x + 0.333333/ SCANLINE_WIDTH, textureY));
	float4 gSample = Texture.Sample(Sample, float2(input.tex.x, textureY));
	float4 bSample = Texture.Sample(Sample, float2(input.tex.x - 0.333333 / SCANLINE_WIDTH, textureY));
	
	float4 color = float4(rSample.x, gSample.y, bSample.z, 1.0);

	float brightness = dot(float3(0.2126, 0.7152, 0.0722), color.xyz);

	float phase = (input.tex.y * SCANLINE_HEIGHT * 2 - .5) * 3.1415926;
	float scanline = pow(sin(phase) * 0.5 + 0.5, 0.5 - 0.5 * brightness);

	float4 scanlineColor = color * scanline;
	return lerp(color, scanlineColor, brightness);
}