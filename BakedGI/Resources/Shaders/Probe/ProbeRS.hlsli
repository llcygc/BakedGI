
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
    "DescriptorTable(SRV(t0, numDescriptors = 2)),"\
    "StaticSampler(s0, maxAnisotropy = 8),"