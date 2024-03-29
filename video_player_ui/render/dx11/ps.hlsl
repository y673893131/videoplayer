////////////////////////////////////////////////////////////////////////////////
// Filename: texture.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
//Texture2D shaderTexture;
Texture2D u_texY;
Texture2D u_texU;
Texture2D u_texV;

Texture2D shaderTexture;
SamplerState SampleType;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR;
    bool   bcolor : bool;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
//RGB
//    float4 textureColor;
//    textureColor = shaderTexture.Sample(SampleType, input.tex);
//    return textureColor;


//YUV420P
//    float y = u_texY.Sample(SampleType, input.tex).r;
//    float u = u_texU.Sample(SampleType, input.tex).r  - 0.5f;
//    float v = u_texV.Sample(SampleType, input.tex).r  - 0.5f;
//    float r = y + 1.14f * v;
//    float g = y - 0.394f * u - 0.581f * v;
//    float b = y + 2.03f * u;
//    return float4(r,g,b, 1.0f);

//    return input.color;

    if(input.bcolor == true)
    {
        return input.color;
    }

    float y = u_texY.Sample(SampleType, input.tex).r;
    y = 1.164 * (y - 0.0625);
    float u = u_texU.Sample(SampleType, input.tex).r  - 0.5f;
    float v = u_texV.Sample(SampleType, input.tex).r  - 0.5f;
    float r = y + 1.596023559570f * v;
    float g = y - 0.3917694091796875f * u - 0.8129730224609375f * v;
    float b = y + 2.017227172851563f * u;
    return float4(r,g,b, 1.0f);
}
