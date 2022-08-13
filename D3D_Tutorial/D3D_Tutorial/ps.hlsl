Texture2D tex : register(t0);
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
	float dis = distance(float3(sampleColor.xyz), float3(0.0, 1.0, 0.0));
	return float4(sampleColor.xyz, step(m_threshold, 0.0));
}