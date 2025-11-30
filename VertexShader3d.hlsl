
cbuffer VS_CONSTANT_BUFFER0 : register(b0)
{
    float4x4 world;
};

cbuffer VS_CONSTANT_BUFFER1 : register(b1)
{
    float4x4 view;
};

cbuffer VS_CONSTANT_BUFFER2 : register(b2)
{
    float4x4 proj;
};


struct VS_INPUT
{
    float3 posL     : POSITION0;
    float3 normalL  : NORMAL0;
    float4 color    : COLOR0;
    float2 uv       : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH     : SV_POSITION;
    float3 normalW  : NORMAL0;
    float4 color    : COLOR0;
    float2 uv       : TEXCOORD0;
};

VS_OUT main(VS_INPUT vsin)
{
    VS_OUT vsout;
    
    // 行列を合成してから変換
    float4x4 mtxWV = mul(world,view);
    float4x4 mtxWVP = mul(mtxWV, proj);
    vsout.posH = mul(float4(vsin.posL, 1.0f), mtxWVP); // mul 行列の乗算
    
    // ワールド空間の法線を作る
    // ①ローカルライトとローカル法線でのライティング(あやしい)
    // ②ワールド変換行列の転置逆行列でワールド法線を作る(あやしい?)
    
    vsout.normalW = mul(float4(vsin.normalL,0.0f), world).xyz;
    
    vsout.color = vsin.color;
    vsout.uv = vsin.uv;
    
    return vsout;
}
