#pragma pack_matrix(row_major)


cbuffer cbPerObject : register(b0)
{
    float4x4 world;
    float4x4 cameraView;
    float4x4 cameraProj;
    float4x4 InvWorldView;
};


struct VertexShader_Input
{
    float3 pos : POSITION0;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
};


struct PixelShader_Input
{
    float4 pos : SV_POSITION;
    float4 depth_pos : TEXTURE0;
};

PixelShader_Input VSMain(VertexShader_Input input)
{
    PixelShader_Input output;

    output.pos = mul(float4(input.pos.xyz, 1.0f), world);
    output.pos = mul(float4(output.pos.xyz, 1.0f), cameraView);
    output.pos = mul(float4(output.pos.xyz, 1.0f), cameraProj);
   
    output.depth_pos = output.pos;

    return output;
}

float4 PSMain(PixelShader_Input input) : SV_TARGET
{
    float depthValue = input.depth_pos.z;
    return float4(depthValue, depthValue, depthValue, 1.0f);
}
