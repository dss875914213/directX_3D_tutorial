Texture2D tex : register(t0);
Texture2D tex1 : register(t1);
SamplerState samplerLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	float m_threshold;
}

struct VSOut
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD1;
};

float4 MyPs(VSOut pIn) : SV_Target
{
	float4 sampleColor = tex.Sample(samplerLinear, pIn.tex);
	//return float4(0.0, 0.0, 0.0, 1.0);
	//return float4(1.0, 1.0, 1.0, 1.0);
	return float4(sampleColor.x, 1.0,1.0, 1.0);
}