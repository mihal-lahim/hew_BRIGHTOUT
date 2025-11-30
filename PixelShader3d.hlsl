
cbuffer PS_CONSTANT_BUFFER0 : register(b0)
{
    float4 ambient_color;
};

cbuffer PS_CONSTANT_BUFFER1 : register(b1)
{
    float4 diffuse_color;
    float4 diffuse_world_vector;
};

cbuffer PS_CONSTANT_BUFFER2 : register(b2)
{
    float4 material_diffuse_color;
};

struct PS_INPUT
{
    float4 posH      : SV_POSITION;
    float3 normalW   : NORMAL0;
    float4 color     : COLOR0;
    float2 uv        : TEXCOORD0;
};

Texture2D major_texture; // テクスチャ
SamplerState major_sampler; // テクスチャサンプラ

float4 main(PS_INPUT psin): SV_TARGET
{
    float4 material = major_texture.Sample(major_sampler, psin.uv) * psin.color * material_diffuse_color; // 自分たちが見えるマテリアルのカラー
    float3 ambient = material.rgb * ambient_color.rgb;
    
    // -1 ～ 1 → 0 ～ 1 0以下は0
    // -1 ～ 1 → 0 ～ 2 → 0 ～ 1
    //float brightness = max(dot(-diffuse_world_vector.xyz, normalize(psin.normalW.xyz)),0.0f); // 明るさ　ライトのベクトルは逆にする(-1)
    float brightness = (dot(-diffuse_world_vector.xyz, normalize(psin.normalW)) + 1.0f) * 0.5f;
    float3 diffuse = material.rgb * diffuse_color.rgb * brightness;
    
    float alpha = material.a * ambient_color.a;
    
    return float4(ambient + diffuse, alpha);
}
