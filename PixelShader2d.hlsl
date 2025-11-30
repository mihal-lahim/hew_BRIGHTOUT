float4 color;

// 何色にして返すのかを指定するシェーダー(floatで)
struct PS_INPUT//線形保管
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

Texture2D major_texture;    // テクスチャ
SamplerState major_sampler; // テクスチャサンプラ

float4 main(PS_INPUT psin): SV_TARGET
{
    float4 texcolor = major_texture.Sample(major_sampler, psin.uv);
    //return texcolor;
    
    return psin.color * texcolor * color;
    
    //return major_texture.Sample(major_sampler,psin.uv); // RGBの値を指定する
}
// 
// 
// 