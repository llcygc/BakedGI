#pragma once

#include "pch.h"
#include "PixelBuffer.h"
#include "Color.h"

class CubeBuffer : public PixelBuffer
{
public:
	CubeBuffer(Color ClearColor = Color(0.0f, 0.0f, 0.0f, 0.0f))
		: m_ClearColor(ClearColor), m_NumMipMaps(0), m_FragmentCount(1), m_SampleCount(1)
	{
		m_SRVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_RTVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		std::memset(m_UAVHandle, 0xFF, sizeof(m_UAVHandle));
	}

	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* BaseResource);

	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return m_RTVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAVHandle[0]; }

	void SetClearColor(Color ClearColor) { m_ClearColor = ClearColor };

	void SetMsaaMode(uint32_t NumColorSamples, uint32_t NumCoverageSamples)
	{
		ASSERT(NumCoverageSamples >= NumColorSamples);
		m_FragmentCount = NumColorSamples;
		m_SampleCount = NumCoverageSamples;
	}

	Color GetClearColor(void) const { return m_ClearColor; }

	void GenerateMipMaps(CommandContext& context);

protected:

	D3D12_RESOURCE_FLAGS CombineResourseFlags(void) const
	{
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;

		if (Flags == D3D12_RESOURCE_FLAG_NONE && m_FragmentCount == 1)
			Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | Flags;
	}

	static inline uint32_t ComputeNumMips(uint32_t Width, uint32_t Height)
	{
		uint32_t HighBit;
		_BitScanReverse((unsigned long *)&HighBit, Width | Height);
		return HighBit + 1;
	}

	void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize = 6, uint32_t NumMips = 1);

	Color m_ClearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_UAVHandle[12];
	uint32_t m_NumMipMaps;
	uint32_t m_FragmentCount;
	uint32_t m_SampleCount;
};

