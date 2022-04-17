Texture2D tex : register(t0);
SamplerState samplerLinear : register(s0);

struct VSOut
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD1;
};

float4 MyPs(VSOut pIn) : SV_Target
{
	float4 sampleColor = tex.Sample(samplerLinear, pIn.tex);
	//float dis = distance(float3(sampleColor.xyz), float3(0.0, 0.6941177, 0.2509804));
	//return float4(sampleColor.xyz, 1-step(dis, 0.5));
	return sampleColor;
}