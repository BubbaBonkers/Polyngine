#ifdef __cplusplus
#define cbuffer struct
#define matrix DirectX::XMMATRIX
#endif

cbuffer MVP_t
{
    matrix Model;
    matrix View;
    matrix Projection;
    float4 DirLightClr;
    float4 HighlightedObjClr;
    float3 DirLightDir;
    float bShowLighting;
    float AmbientLightIntensity;
    float ObjSelected;
    float PointLightCount;
    float DirLightIntensity;
    float4 PointLightLoc[112];
    float4 PointLightClr[112];
    float4 PointLightRad[112];
    float4 PointLightInt[112];
    float4 CameraPos;
    float4 BaseSpecular;
    float4 BaseEmissive;
    
    float4 PaddingB;
    
    float bHasEmissiveTex;
    float bHasSpecularTex;
    
    float2 PaddingC;
    float4 PaddingD;
    float4 PaddingE;
    float4 PaddingF;
};