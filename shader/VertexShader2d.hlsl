
//float4x4 mtx; // 4x4の行列
//float4 position;

cbuffer VS_CONSTANT_BUFFER0 : register(b0) // 定数バッファ
{
    float4x4 proj;
};

cbuffer VS_CONSTANT_BUFFER1 : register(b1) // 定数バッファ
{
    float4x4 world;
};

struct VS_INPUT
{
    float4 posL     : POSITION0;
    float4 color    : COLOR0;
    float2 uv       : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_INPUT vsin)
{
    VS_OUT vsout;
    // 行列
    float4x4 mtx = mul(world,proj);
    vsout.posH = mul(vsin.posL, mtx); // mul 行列の乗算
    
    vsout.color = vsin.color;
    vsout.uv = vsin.uv;
    
    return vsout;
}
// floatで戻る
//
// これは、頂点シェーダーの出力を定義するための構造体です
// セマンティクス　→ : POSITION 　また(POSITION0)ゼロでもよい
// 座標変換が終わったものを上げる →　: SV_POSITION