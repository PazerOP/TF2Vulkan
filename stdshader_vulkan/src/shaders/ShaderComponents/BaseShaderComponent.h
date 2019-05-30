#pragma once

#include "shaders/ShaderParamNext.h"

#include "TF2Vulkan/Util/SafeConvert.h"

#include <tuple>

namespace TF2Vulkan{ namespace Shaders
{
	template<typename T>
	struct ParamGroupWrapper : T
	{
		constexpr ParamGroupWrapper()
		{
			static_assert(sizeof(T) % sizeof(ShaderParamNext) == 0,
				"ParamGroup type must contain only ShaderParamNext member variables");
		}

		const ShaderParamNext* LocalBase() const { return reinterpret_cast<const ShaderParamNext*>(static_cast<const T*>(this)); }
		ShaderParamNext* LocalBase() { return reinterpret_cast<ShaderParamNext*>(static_cast<T*>(this)); }
		static constexpr size_t size() { return sizeof(T) / sizeof(ShaderParamNext); }
	};

	namespace detail
	{
		void LastDitchInitParams(IMaterialVar** params, const ShaderParamNext* base, size_t count);
	}

	template<typename... TGroups>
	struct ShaderParams : ParamGroupWrapper<TGroups>...
	{
		ShaderParams()
		{
			if constexpr (sizeof...(TGroups) > 0)
				InitParamIndices<0>(NUM_SHADER_MATERIAL_VARS);
		}

	protected:
		static constexpr size_t ParamsCount = (0 + ... + (ParamGroupWrapper<TGroups>::size()));
		static constexpr size_t GroupCount() { return sizeof...(TGroups); }
		template<size_t i> using GroupType = std::tuple_element_t<i, std::tuple<ParamGroupWrapper<TGroups>...>>;
		template<size_t i> static constexpr size_t GroupSize = GroupType<i>::size();
		const ShaderParamNext* ParamsBase() const { return reinterpret_cast<const ShaderParamNext*>(this); }

	private:
		template<size_t groupIndex>
		void InitParamIndices(size_t paramIndex)
		{
			if constexpr (groupIndex < sizeof...(TGroups))
			{
				for (size_t i = 0; i < GroupSize<groupIndex>; i++)
				{
					if (GroupType<groupIndex>::LocalBase()[i].InitIndex(Util::SafeConvert<int>(paramIndex)))
						paramIndex++;
				}

				if constexpr ((groupIndex + 1) < sizeof...(TGroups))
					InitParamIndices<groupIndex + 1>(paramIndex);
			}
		}

		template<typename T, typename TParams, ShaderFlags_t FLAGS> friend class ShaderNext;
		void InitParamGroups(IMaterialVar** params)
		{
			(TGroups::InitParamGroup(params), ...);
			(detail::LastDitchInitParams(params, ParamGroupWrapper<TGroups>::LocalBase(), ParamGroupWrapper<TGroups>::size()), ...);
		}
	};

	namespace detail
	{
		template<typename T> static constexpr bool IsShaderParams(T*) { return false; }
		template<typename... T> static constexpr bool IsShaderParams(ShaderParams<T...>*) { return true; }
	}

	template<typename T> static constexpr bool is_shader_params_v = detail::IsShaderParams((T*)nullptr);
} }
