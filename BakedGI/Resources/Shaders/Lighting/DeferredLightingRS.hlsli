
#define Deferred_RootSig \
    "RootFlags(0)," \
    "RootConstants(b0, num32BitConstants = 4)," \
    "CBV(b1)," \
    "CBV(b2)," \
	"DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "DescriptorTable(SRV(t0, numDescriptors = 4)),"\
    "DescriptorTable(SRV(t64, numDescriptors = 1)),"\
	"StaticSampler(s0," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"comparisonFunc = COMPARISON_GREATER_EQUAL," \
		"filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)"