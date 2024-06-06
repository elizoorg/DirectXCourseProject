#pragma pack_matrix(row_major)

cbuffer cbPerObject : register(b0)
{
    float4x4 world;
    float4x4 cameraView;
    float4x4 cameraProj;
    float4x4 InvWorldView;
};


struct VS_IN
{
    float3 pos : POSITION0;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
};



struct PixelShader_Input
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 world_pos : TEXCOORD1;
    float4 pos_in_light_view : TEXCOORD2;
    float3 camera_direction : TEXCOORD4;
};

cbuffer LightTransformBuffer : register(b1)
{
float4x4 dir_light_view_proj;
float4 camera_pos;
}

cbuffer AmbientLightBuffer : register(b2)
{
float4 ambient_light_col;
}

cbuffer DirectionalLightBuffer : register(b3)
{
float4 dir_light_direction;
float4 dir_light_col;
float4 dir_light_diffK_specA_specK;
}

Texture2D Texture : TEXTURE : register(t0);
Texture2D DirLightDepthMapTexture : register(t1);

SamplerState Sampler : SAMPLER : register(s0);
SamplerState SampleTypeClamp : register(s1);

PixelShader_Input VSMain(VS_IN input)
{
    PixelShader_Input output = (PixelShader_Input)0;

    output.pos = mul(float4(input.pos.xyz, 1.0f), world);
    output.pos = mul(float4(output.pos.xyz, 1.0f), cameraView);
    output.pos = mul(float4(output.pos.xyz, 1.0f), cameraProj);
    
    
    
    output.normal = normalize(mul(float4(input.normal, 0), world).xyz);
    output.tex = input.tex;
    output.world_pos = mul(float4(input.pos, 1), world).xyz;
    output.pos_in_light_view = mul(float4(output.world_pos, 1), dir_light_view_proj);
    output.camera_direction = normalize(camera_pos.xyz - output.world_pos);

    return output;
}

float4 PSMain(PixelShader_Input input) : SV_Target
{
    const float bias = 5e-6f;
    float4 col = float4(0, 0, 0, 1);

    //Ambient Light
    col.rgb += float4(0.2, 0.2, 0.2, 0);
    //Projected to Light View
    float2 projected_tex_coord;
    projected_tex_coord.x = input.pos_in_light_view.x / input.pos_in_light_view.w / 2.0f + 0.5f;
    projected_tex_coord.y = -input.pos_in_light_view.y / input.pos_in_light_view.w / 2.0f + 0.5f;

    if ((saturate(projected_tex_coord.x) == projected_tex_coord.x) &&
        (saturate(projected_tex_coord.y) == projected_tex_coord.y))
    {
        //Depth Comparison
        const float depth = DirLightDepthMapTexture.Sample(SampleTypeClamp, projected_tex_coord).r;
        const float light_depth = input.pos_in_light_view.z / input.pos_in_light_view.w - bias;

        if (light_depth < depth)
        {
            //Directional Light

            //Diffuse
            col.rgb += dir_light_col.xyz
                * max(0, dot(-dir_light_direction.xyz, input.normal))
                * dir_light_diffK_specA_specK.x;

            //Specular
            const float3 dir_light_reflection = normalize(reflect(-dir_light_direction.xyz, input.normal));
            col.rgb += dir_light_col.xyz
                * pow(max(0, dot(-input.camera_direction, dir_light_reflection)), dir_light_diffK_specA_specK.y)
                * dir_light_diffK_specA_specK.z;
        }
    }

    col.rgb = pow(col.rgb, 1 / 2.2f);

    const float3 tex_color = Texture.Sample(Sampler, input.tex).xyz;
    col.rgb *= tex_color;
    col = float4(col.r, col.g, col.b, 0.5f);
    return col;
}
