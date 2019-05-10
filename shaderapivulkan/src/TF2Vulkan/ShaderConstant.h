#pragma once

#include <stdshader_dx9_tf2vulkan/AlignedTypes.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan
{
	union ShaderConstant
	{
		constexpr ShaderConstant() : m_Int{} {}

#ifndef __INTELLISENSE__
		std::weak_equality operator<=>(const ShaderConstant& other) const
		{
			return operator==(other) ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
		}
#endif
		bool operator!=(const ShaderConstant& other) const { return !operator==(other); }
		bool operator==(const ShaderConstant& other) const { return m_Int == other.m_Int; }

		ShaderConstants::float4 m_Float;
		ShaderConstants::int4 m_Int;
		ShaderConstants::bool4 m_Bool;
	};

	struct ShaderConstantValues
	{
		constexpr ShaderConstantValues() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(ShaderConstantValues);

		void SetData(size_t startReg, const ShaderConstant* data, size_t numRegs);

		bool m_Dirty = false;
		std::array<ShaderConstant, 1024> m_Constants;
	};

	using VertexShaderConstants = ShaderConstantValues;
	using PixelShaderConstants = ShaderConstantValues;
}
