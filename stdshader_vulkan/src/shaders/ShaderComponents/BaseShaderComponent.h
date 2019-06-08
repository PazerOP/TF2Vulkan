#pragma once

#include "shaders/ShaderParamNext.h"

#include "TF2Vulkan/ISpecConstLayout.h"
#include "TF2Vulkan/ShaderResourceBinder.h"
#include "TF2Vulkan/Util/SafeConvert.h"

#include <tuple>

namespace TF2Vulkan{ namespace Shaders
{
	namespace detail
	{
		void LastDitchInitParams(IMaterialVar** params, const ShaderParamNext* base, size_t count);
	}

	template<typename... TComponents>
	class ShaderComponents : public TComponents::Params...
	{
	public:
		struct UniformBuf final : TComponents::UniformBuf...
		{

		};

	protected:
		struct SpecConstBuf final : BaseSpecConstBuf, TComponents::SpecConstBuf...
		{

		};

		struct SpecConstLayout final : BaseSpecConstLayout<SpecConstBuf>, TComponents::template SpecConstLayout<SpecConstBuf>...
		{
			static constexpr size_t LAYOUT_ENTRIES_SIZE = (sizeof(BaseSpecConstLayout<SpecConstBuf>) + ... + sizeof(TComponents::template SpecConstLayout<SpecConstBuf>));
			static constexpr size_t LAYOUT_ENTRY_COUNT = LAYOUT_ENTRIES_SIZE / sizeof(SpecConstLayoutEntry);
			const SpecConstLayoutEntry* begin() const { return reinterpret_cast<const SpecConstLayoutEntry*>(this); }
			const SpecConstLayoutEntry* end() const { return begin() + LAYOUT_ENTRY_COUNT; }
			operator SpecConstLayoutCreateInfo() const { return SpecConstLayoutCreateInfo{ begin(), LAYOUT_ENTRY_COUNT }; }

		} inline static const s_SpecConstLayout;
		static_assert(_MSC_VER <= 1921, "Try making the above statement constexpr again, if it still ICE's then increment this check");

		static constexpr size_t PARAMS_COUNT = (0 + ... + (sizeof(typename TComponents::Params))) / sizeof(ShaderParamNext);
		const ShaderParamNext* ParamsBase() const { return reinterpret_cast<const ShaderParamNext*>(this); }

	private:
		static constexpr size_t SC_BUFFER_SIZE = sizeof(SpecConstBuf);

		static constexpr size_t SC_BUFFER_ELEMENT_COUNT = SC_BUFFER_SIZE / 4;
		static_assert(sizeof(uint32_t) == 4);
		static_assert(sizeof(int32_t) == 4);
		static_assert(sizeof(float) == 4);

		ShaderParamNext* ParamsBase() { return reinterpret_cast<ShaderParamNext*>(this); }

	public:
		ShaderComponents()
		{
			// The whole idea is that they're contiguous... so here goes nothing
			{
				size_t index = NUM_SHADER_MATERIAL_VARS;
				for (size_t i = 0; i < PARAMS_COUNT; i++)
				{
					if (ParamsBase()[i].InitIndex(Util::SafeConvert<int>(index)))
						index++;
				}
			}
		}

		static constexpr size_t COMPONENT_COUNT = sizeof...(TComponents);

	protected:
		template<typename TUniforms, typename TSpecConsts>
		void PreDraw(IMaterialVar** params, TUniforms& uniformBuf, TSpecConsts& specConsts, ShaderTextureBinder& tb) const
		{
			(TComponents::Params::PreDraw(params, &uniformBuf, &specConsts, tb), ...);
		}

	private:
		template<typename T, typename TParams, ShaderFlags_t FLAGS> friend class ShaderNext;
		void InitParamGroups(IMaterialVar** params) const
		{
			(TComponents::Params::InitParamGroup(params), ...);
			(detail::LastDitchInitParams(params,
				reinterpret_cast<const ShaderParamNext*>(static_cast<const typename TComponents::Params*>(this)),
				sizeof(typename TComponents::Params) / sizeof(ShaderParamNext)), ...);
		}

		void LoadParamGroupResources(IMaterialVar** params, IShaderInit& init, const char* materialName, const char* texGroupName) const
		{
			(TComponents::Params::LoadResources(params, init, materialName, texGroupName), ...);
		}
	};

	namespace detail
	{
		static constexpr bool IsShaderComponents(void*) { return false; }
		template<typename... T> static constexpr bool IsShaderComponents(ShaderComponents<T...>*) { return true; }
	}

	template<typename T> static constexpr bool is_shader_components_v = detail::IsShaderComponents((T*)nullptr);
} }
