#include "../Headers/MVP.hlsli"

SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);
Texture2D txEmissive : register(t1);
Texture2D txSpecular : register(t2);

struct VS_Out
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD1;
    float3 WorldPos : WORLDPOSITION;
};

float4 main(VS_Out Input) : SV_TARGET
{
    //
    // GET SAMPLE OF THE PIXEL COLOR
    //
    // Get pixel color for texture.
    float4 FinalColor = txDiffuse.Sample(samLinear, Input.Texture);
    float OriginalAlpha = FinalColor.a;
    
    //
    // EMISSIVE STORAGE
    //
    float4 E_Result = txEmissive.Sample(samLinear, Input.Texture);
    float4 E_Base = BaseEmissive;
    float4 E_Factor = { 0, 0, 0, 0 };
    
    //
    // CHECK FOR PIXEL ALPHA ELIMINATION
    //
    // Check for alpha elimination.
    if (FinalColor.a < 0.05f)
    {
        discard;
    }
    
    // Generate lighting.
    if (bShowLighting)
    {
        // Create/allocate memory needed to calculate the lighting.
        float4 S_Result = { 0, 0, 0, 0 };
        float4 D_Result = { 0, 0, 0, 0 };
        float4 P_Result = { 0, 0, 0, 0 };
        float4 Ambient  = { 0, 0, 0, 0 };
        float4 S_Factor = { 0, 0, 0, 0 };
        
        //
        // FACTORS AND BASES
        //
        float4 S_Base = BaseSpecular;
        
        //
        // AMBIENT LIGHTING
        //
        // Get the ambient lighting.
        Ambient = ((float4(1, 1, 1, 1)) * AmbientLightIntensity);
        
        //
        // SPECULAR FACTOR FOR LIGHTING
        //
        // Get the specular for this spot.
        if (bHasSpecularTex)
        {
            S_Factor = S_Base + txSpecular.Sample(samLinear, Input.Texture);
        }
        else
        {
            S_Factor = S_Base;
        }
        
        //
        // DIRECTIONAL LIGHTING
        //
        // Get the directional lighting.
        float D_LightRatio = saturate((dot(-DirLightDir.xyz, Input.Normal)) * DirLightIntensity);
        D_Result = D_LightRatio * DirLightClr;
    
        // Phong Specular Calculation for the directional light/sunlight.
        float3 S_ViewDir = normalize(CameraPos.xyz - Input.WorldPos.xyz);
        float3 S_HalfVector = normalize(-DirLightDir + S_ViewDir);
        float3 S_Intensity = max(pow(clamp(dot(Input.Normal, normalize(S_HalfVector)), 0, 1), 64.0f), 0);
        S_Result = DirLightClr * S_Factor * float4(S_Intensity, 1);
    
        //
        // POINT LIGHTING
        //
        for (int i = 0; i < PointLightCount; ++i)
        {
	        // Get the point lights.
            float3 P_LightDir = normalize(PointLightLoc[i].xyz - Input.WorldPos.xyz);
            float P_LightRatio = saturate((dot(P_LightDir.xyz, Input.Normal)) * saturate(PointLightInt[i].x));
	
	        // Attenuation for point light.
            float P_Attenuation = (1.0f - ((saturate(length(PointLightLoc[i].xyz - Input.WorldPos.xyz) / PointLightRad[i].x))));
            P_Result += (P_LightRatio * PointLightClr[i]) * P_Attenuation;
            
            // Phong Specular Calculation for the point lights in the scene.
            S_ViewDir = normalize(CameraPos.xyz - Input.WorldPos.xyz);
            S_HalfVector = normalize(P_LightDir + S_ViewDir);
            S_Intensity = max(pow(clamp(dot(Input.Normal, S_HalfVector), 0, 1), 32.0f), 0);
            S_Result += (PointLightClr[i] * PointLightInt[i]) * S_Factor * float4(S_Intensity, 1);
        }
        
        //
        // FINAL LIGHTING COLOR COMBINATION
        //
        // Add the ambient to the output color and correct the alpha.
        FinalColor = float4(((D_Result + Ambient + P_Result + S_Result) * FinalColor).rgb, OriginalAlpha);
    }
    
    //
    // APPLY EMISSIVE COLORING
    //
    // Add the emissive colors to models.
    if (bHasEmissiveTex)
    {
        E_Factor = E_Base + E_Result;
    }
    else
    {
        E_Factor = E_Base;
    }
    
    FinalColor += E_Factor;
        
    //
    // APPLY EDITOR SELECTION EFFECT
    //
    // Highlight this object if it is seleced in the renderer.
    if (ObjSelected > 0)
    {
        FinalColor += (float4(HighlightedObjClr.xyz, 1.0f) * HighlightedObjClr.w);
    }
    
    //
    // SECOND CHECK FOR PIXEL ALPHA ELIMINATION
    //
    // Check for alpha elimination.
    if (FinalColor.a < 0.05f)
    {
        discard;
    }
        
    return FinalColor;
}