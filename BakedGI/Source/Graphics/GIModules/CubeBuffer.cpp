
#include "CubeBuffer.h"
#include "GraphicsCore.h"
#include "CommandContext.h"

using namespace Graphics;

void CubeBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips)
{
	m_NumMipMaps = NumMips - 1;

	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

	RTVDesc.Format = Format;
	UAVDesc.Format = GetUAVFormat(Format);
	SRVDesc.Format = Format;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	RTVDesc.Texture2DArray.MipSlice = 0;
	RTVDesc.Texture2DArray.FirstArraySlice = 0;
	RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	UAVDesc.Texture2DArray.MipSlice = 0;
	UAVDesc.Texture2DArray.FirstArraySlice = 0;
	UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MipLevels = NumMips;
	SRVDesc.TextureCube.MostDetailedMip = 0;

	
}

