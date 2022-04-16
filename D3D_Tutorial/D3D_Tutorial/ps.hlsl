Texture2D tex : register(t0);
SamplerState samplerLinear : register(s0);

struct VSOut
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD;
};

float4 MyPs(VSOut pIn) : SV_Target
{
	return tex.Sample(samplerLinear, pIn.tex);
}