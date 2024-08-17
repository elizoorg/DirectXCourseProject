struct VertexShaderOutput
{
    float4 position : SV_POSITION; // Clip space position.
    float2 uv : TexCoord;
};

VertexShaderOutput VSMain(uint vertexID : SV_VertexID)
{
    VertexShaderOutput result;
    result.uv = float2(vertexID & 2, (vertexID << 1) & 2);
    result.position = float4(result.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return result;
}

Texture2D AccumTexture : register(t0);
SamplerState sampl : register(s0);

float4 PSMain(VertexShaderOutput input) : SV_TARGET
{
    float4 color = AccumTexture.Sample(sampl, input.uv);
    
    //color = float4((color.x + color.y + color.z) / 3, (color.x + color.g + color.b) / 3, (color.x + color.g + color.b) / 3, 0.0f);

    return color; // must return an RGBA colour
}