#ifndef __PBRFUNCS__
#define __PBRFUNCS__

static const float PI = 3.14159265f;

// ----------------------------------------------------------------------------
float DistributionGGX( float NdotH, float roughness )
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = ( NdotH2 * ( a2 - 1.0f ) + 1.0f );
    denom = PI * denom * denom;

    return nom / denom;
}

float GGX_NDF( float NdotH, float roughness )
{
    float roughnessSqr = roughness * roughness;
    float NdotHSqr = NdotH * NdotH;
    float TanNdotHSqr = ( 1.0f - NdotHSqr ) / NdotHSqr;
    return ( 1.0f / 3.1415926535f ) * sqrt( roughness / ( NdotHSqr * ( roughnessSqr + TanNdotHSqr ) ) );
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX( float NdotV, float roughness )
{
    float r = ( roughness + 1.0 );
    float k = ( r * r ) / 8.0;

    float nom = NdotV;
    float denom = NdotV * ( 1.0 - k ) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith( float NdotV, float NdotL, float roughness )
{
    float ggx2 = GeometrySchlickGGX( NdotV, roughness );
    float ggx1 = GeometrySchlickGGX( NdotL, roughness );

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
// float cosTheta = NdotV
// F0 = ( (n1 - n2) / (n1 + n2 ) ) ^2
// n -> https://en.wikipedia.org/wiki/Refractive_index
//
float3 FresnelSchlick( float cosTheta, float3 F0 )
{
    float f = 1.0f - cosTheta;
    float fPowSchlick = f * f * f * f * f;
    return F0 + ( 1.0f - F0 ) * fPowSchlick;
}

#endif