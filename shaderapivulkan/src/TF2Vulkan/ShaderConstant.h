#pragma once

#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan
{
	union ShaderConstant
	{
		constexpr ShaderConstant() : m_Int{ 0, 0, 0, 0 } {}

#ifndef __INTELLISENSE__
		std::weak_equality operator<=>(const ShaderConstant& other) const
		{
			return operator==(other) ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
		}
#endif
		bool operator!=(const ShaderConstant& other) const { return !operator==(other); }
		bool operator==(const ShaderConstant& other) const
		{
			return
				m_Int[0] == other.m_Int[0] &&
				m_Int[1] == other.m_Int[1] &&
				m_Int[2] == other.m_Int[2] &&
				m_Int[3] == other.m_Int[3];
		}

		std::array<float, 4> m_Float;
		std::array<int, 4> m_Int;
		std::array<BOOL, 4> m_Bool;
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
