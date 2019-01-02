
#define CubeToOctan_RootSig \
    "RootFlags(0)," \
    "RootConstants(b0, num32BitConstants = 4)," \
    "CBV(b1)," \
	"DescriptorTable(UAV(u0, numDescriptors = 4))," \
    "DescriptorTable(SRV(t0, numDescriptors = 3)),"\
    "StaticSampler(s0, maxAnisotropy = 8),"

#define ProbeTrace_RootSig \
    "RootFlags(0)," \
    "RootConstants(b0, num32BitConstants = 4)," \
    "CBV(b1)," \
	"DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "DescriptorTable(SRV(t0, numDescriptors = 4)),"\
    "StaticSampler(s0, maxAnisotropy = 8),"

#define Temporal_RootSig \
    "RootFlags(0)," \
    "RootConstants(b0, num32BitConstants = 4)," \
    "CBV(b1)," \
	"DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "DescriptorTable(SRV(t0, numDescriptors = 2))," \
    "StaticSampler(s0, maxAnisotropy = 8),"

#define ProbeRender_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "RootConstants(b1, num32BitConstants = 2, visibility = SHADER_VISIBILITY_VERTEX), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0, numDescriptors = 6), visibility = SHADER_VISIBILITY_PIXEL)," \
    "CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t64, numDescriptors = 6), visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s1, visibility = SHADER_VISIBILITY_PIXEL," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "comparisonFunc = COMPARISON_GREATER_EQUAL," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)"