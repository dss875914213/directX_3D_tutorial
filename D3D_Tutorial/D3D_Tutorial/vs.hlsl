struct VSOut
{
	float4 pos : SV_Position;
};

VSOut MyVs(float3 pos : POSITION)
{
	VSOut vsOut;
	vsOut.pos = float4(pos.x, pos.y, pos.z, 1.0);
	return vsOut;
}
