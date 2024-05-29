
// The Pixel Shader (PS) stage takes the interpolated per-vertex values from
// the rasterizer stage and produces one (or more) per-pixel color values.

#include "PBR_Funcs.hlsli"

struct CameraDesc
{
    float4 Position;
    float4 Direction;
};

struct AmbientDesc
{
    float4 Up;
    float4 Down;
};

struct Tex
{
    bool HasTexture;
};

struct DirectionalLight
{
    float3 direction;
    float3 color;
};

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 Texture : TEXCOORD;
};

ConstantBuffer<Tex> Text : register(b1);
ConstantBuffer<AmbientDesc> Ambient : register(b2);
ConstantBuffer<CameraDesc> Camera : register(b3);

SamplerState Sampler : register(s0);
Texture2D Texture : register(t1);

float4 CalculateSemiAmbient(float3 normal, float3 color);
float3 CalculateAmbient(DirectionalLight light);
float3 CalculateDiffuse(float3 norm, DirectionalLight light);
float3 CookTorrance(in float3 surPos, in float3 normal, in float3 surColor);

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 norm = normalize(IN.Normal);
    
    float3 color;
    if (Text.HasTexture)
    {
        float2 uv = IN.Texture;
        uv.y = 1.0f - uv.y;
        color = Texture.Sample(Sampler, uv);
    }
    else
    {
        color = IN.Color;
    }
    
    DirectionalLight dirLight;
    dirLight.direction = normalize(float3(-0.5f, 1.0f, -0.5f));
    dirLight.color = float3(1.0f, 1.0f, 1.0f);
    //color = (CalculateAmbient(dirLight) + CalculateDiffuse(norm, dirLight)) * color;
    
    
    color = saturate(color);
    color = (CookTorrance(IN.Position.xyz, IN.Normal, color) * 0.9f) + (CalculateAmbient(dirLight) * color);
    
    return float4(color, 1.0f);
}

float4 CalculateSemiAmbient(float3 normal, float3 color)
{
    // Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5f + 0.5f;
    // Calculate the ambient value
    float4 ambient = Ambient.Down + up * Ambient.Up;

    // Apply the ambient value to the color
    return ambient;
}

float3 CalculateAmbient(DirectionalLight light)
{
    return 0.1f * light.color;
}

float3 CalculateDiffuse(float3 norm, DirectionalLight light)
{
    float3 lightDirection = normalize(light.direction);
    float diffuseFactor = max(dot(norm, lightDirection), 0.0f);
    return light.color * diffuseFactor;
}

float3 CookTorrance(in float3 surPos, in float3 surNormal, in float3 surColor)
{
    float3 lightColor = { 0.9f, 0.9f, 0.9f };
    float3 lightNormal = normalize(float3(-0.5f, 1.0f, -0.5f));
    
    float3 dirToLight = lightNormal;
    float3 toEye = normalize(Camera.Position.xyz - surPos);
    float3 halfWay = normalize(dirToLight + toEye);
    
    /*
        This is part of BRDF, so, skip for now. need to have
        info for future, here is specular part of CookTorrance

                 D * G * F
        Rs = ------------------
              PI * NdotL * NdotV

        NDF(D) - Normal Distribution Function, which describes the concentration of microfacets which are oriented such that they could reflect light
        G - Geometric Shadowing-Masking Function, which describes the percentage of microfacets which are neither shadowed or masked by neighboring microfacets
        F - is the Fresnel Function, which describes the amount of light reflected by the material (as opposed to being absorbed or transmitted by the material)
    */
    float NdotL = max(dot(surNormal, dirToLight), 0.0f);
    float NdotH = max(dot(surNormal, halfWay), 0.0f);
    float NdotV = max(dot(surNormal, toEye), 0.0f);
    float HdotV = max(dot(halfWay, toEye), 0.0f);

    float roughness = 0.2f;
    float metallic = 0.0f;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, surColor, metallic);


    // lambertian diffuse part 
    float attenuation = 0.04f;
    float3 radiance = lightColor * NdotL * attenuation;

    // Cook-Torrance BRDF
    float3 NDF = GGX_NDF(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    float3 F = FresnelSchlick(HdotV, F0);


    float3 nominator = NDF * G * F;
    float denominator = PI * NdotV * NdotL + 0.001; // to prevent divide by zero
    float3 specular = saturate(nominator / denominator);

    // kS is equal to Fresnel
    float3 kS = saturate(F);

    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;

    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // add to outgoing radiance Lo
    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    return (kD * surColor + specular) * radiance;
}
