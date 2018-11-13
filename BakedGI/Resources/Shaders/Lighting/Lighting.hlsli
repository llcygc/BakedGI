#include "BRDF.hlsli"
#include "LightData.hlsli"

float3 DirectLighting(PositionData posData, BRDFData brdfData, float3 lightColor, float3 lightDir)
{
    float3 result = 0;

    half3 specColor = 0;
    half3 diffuseColor = 0;
    half oneMinusReflectivity = 0;
    half metallic = brdfData.metallic;
    diffuseColor = brdfData.diffuse.rgb;//
    DiffuseAndSpecularFromMetallic(brdfData.diffuse.rgb, metallic, specColor, oneMinusReflectivity);

    float3 halfDir = normalize(lightDir + posData.viewDiretion);
        
    float nl = saturate(dot(posData.normalWorld, lightDir));
    float nh = saturate(dot(posData.normalWorld, halfDir));
    half nv = saturate(dot(posData.normalWorld, posData.viewDiretion));
    float lh = saturate(dot(lightDir, halfDir));
    
    half perceptualRoughness = SmoothnessToPerceptualRoughness(brdfData.glossness);
    half roughness = PerceptualRoughnessToRoughness(perceptualRoughness);
    
    // Diffuse term
    half diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl;
    
    // GGX with roughtness to 0 would mean no specular at all, using max(roughness, 0.002) here to match HDrenderloop roughtness remapping.
    roughness = max(roughness, 0.002);
    half V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
    float D = GGXTerm(nh, roughness);
    
    half specularTerm = V * D * PI; // Torrance-Sparrow model, Fresnel is applied later
    
    // specularTerm * nl can be NaN on Metal in some cases, use max() to make sure it's a sane value
    specularTerm = max(0, specularTerm * nl);
    
    half3 surfaceReduction = 1.0 / (roughness * roughness + 1.0); // fade \in [0.5;1]    
    
    half grazingTerm = saturate(brdfData.glossness + (1 - oneMinusReflectivity));

    float3 color = diffuseColor * (lightColor * diffuseTerm)
                    + specularTerm * lightColor * FresnelTerm(specColor, lh);
                    //+ surfaceReduction * gi.specular * FresnelLerp(specColor, grazingTerm, nv);

    return color; //FresnelTerm(specColor, lh);
}