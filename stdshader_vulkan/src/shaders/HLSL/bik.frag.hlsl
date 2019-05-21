#include "common_ps_fxc.hlsli"

Texture2D VideoTextureY;
Texture2D VideoTextureCR;
Texture2D VideoTextureCB;

SamplerState VideoSamplerY;
SamplerState VideoSamplerCR;
SamplerState VideoSamplerCB;

static float3 ConvertColorSpace601(float3 y_cb_cr)
{
	float3 rgb;

	float yConst = (255.0f / 219 * y_cb_cr.x);

	rgb.r = yConst + (255.0f / 224 * 1.402 * y_cb_cr.z) - (222.921 / 255);
	rgb.g = yConst - (100.291 / 256 * y_cb_cr.y) - (208.120 / 255 * y_cb_cr.z) + (135.576 / 255);
	rgb.b = yConst + (516.412 / 256 * y_cb_cr.y) - (276.836 / 255);

	rgb = saturate(rgb);

	return rgb;
}

struct PS_INPUT
{
	float2 texCoord : TEXCOORD0;
};

float4 main(const PS_INPUT input) : SV_TARGET
{
	float3 y_cb_cr = float3(
		VideoTextureY.Sample(VideoSamplerY, input.texCoord).r,
		VideoTextureCB.Sample(VideoSamplerCB, input.texCoord).r,
		VideoTextureCR.Sample(VideoSamplerCR, input.texCoord).r);

	float3 rgb = ConvertColorSpace601(y_cb_cr);

	//return float4(1, 1, 1, 1);
	return float4(rgb, 1);
}
