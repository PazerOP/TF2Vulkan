#pragma once

#include <stdshader_dx9_tf2vulkan/ShaderData.h>

namespace TF2Vulkan{ namespace ShaderCompatData
{
	class IShaderCompatData
	{
	public:
		virtual void SetConstant(ShaderConstants::VSData& data, uint32_t var,
			const ShaderConstants::float4& vec4) const = 0;
		virtual void SetConstant(ShaderConstants::VSData& data, uint32_t var,
			const ShaderConstants::int4& vec4) const = 0;
		virtual void SetConstant(ShaderConstants::VSData& data, uint32_t var,
			const ShaderConstants::bool4& vec4) const = 0;

		virtual void SetConstant(ShaderConstants::PSData& data, uint32_t var,
			const ShaderConstants::float4& vec4) const = 0;
		virtual void SetConstant(ShaderConstants::PSData& data, uint32_t var,
			const ShaderConstants::int4& vec4) const = 0;
		virtual void SetConstant(ShaderConstants::PSData& data, uint32_t var,
			const ShaderConstants::bool4& vec4) const = 0;

		void SetConstants(ShaderConstants::VSData& data, uint32_t firstVar,
			const ShaderConstants::float4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}
		void SetConstants(ShaderConstants::VSData& data, uint32_t firstVar,
			const ShaderConstants::int4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}
		void SetConstants(ShaderConstants::VSData& data, uint32_t firstVar,
			const ShaderConstants::bool4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}

		void SetConstants(ShaderConstants::PSData& data, uint32_t firstVar,
			const ShaderConstants::float4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}
		void SetConstants(ShaderConstants::PSData& data, uint32_t firstVar,
			const ShaderConstants::int4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}
		void SetConstants(ShaderConstants::PSData& data, uint32_t firstVar,
			const ShaderConstants::bool4* vec4s, uint32_t numVecs) const
		{
			for (uint32_t i = 0; i < numVecs; i++)
				SetConstant(data, firstVar + i, vec4s[i]);
		}

		struct SpecConstMapping final
		{
			const char* m_SpecConstName;
			uint_fast8_t m_ComboOffset;
			uint32_t m_ComboMask = 0x1;
		};
		virtual const SpecConstMapping* GetSpecConstMappings(size_t& count) const = 0;
	};

	extern const IShaderCompatData& g_XLitGeneric;
	extern const IShaderCompatData& g_XLitGenericBump;
} }
