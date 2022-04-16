struct VSOut
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD;
};

struct VSIn
{
	float3 pos : POSITION;
	float3 tex : TEXCOORD;
};

VSOut MyVs(VSIn vIn)
{
	VSOut vsOut;
	vsOut.pos = float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.0);
	vsOut.tex = vIn.tex;
	return vsOut;
}
