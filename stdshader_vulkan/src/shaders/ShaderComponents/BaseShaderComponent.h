#pragma once

#include "shaders/ShaderParamNext.h"

#include "TF2Vulkan/ISpecConstLayout.h"
#include "TF2Vulkan/ShaderTextureBinder.h"
#include "TF2Vulkan/Util/SafeConvert.h"

#include <tuple>

namespace TF2Vulkan{ namespace Shaders
{
	namespace detail
	{
		void LastDitchInitParams(IMaterialVar** params, const ShaderParamNext* base, size_t count);

#if false
		template<typename T>
		class ParamGroupWrapper : public T
		{
		public:
			constexpr ParamGroupWrapper()
			{
				static_assert(sizeof(T) % sizeof(ShaderParamNext) == 0,
					"ParamGroup type must contain only ShaderParamNext member variables");
			}

			const ShaderParamNext* LocalBase() const { return reinterpret_cast<const ShaderParamNext*>(static_cast<const T*>(this)); }
			ShaderParamNext* LocalBase() { return reinterpret_cast<ShaderParamNext*>(static_cast<T*>(this)); }
			static constexpr size_t size() { return sizeof(T) / sizeof(ShaderParamNext); }
		};
#endif
	}

#if false
	template<typename... TGroups>
	class ShaderParams : public detail::ParamGroupWrapper<TGroups>...
	{
	public:
		ShaderParams()
		{
			size_t index = NUM_SHADER_MATERIAL_VARS;
			(InitParamIndices<TGroups>(index), ...);
		}

	protected:
		static constexpr size_t ParamsCount = (0 + ... + (detail::ParamGroupWrapper<TGroups>::size()));
		static constexpr size_t GroupCount() { return sizeof...(TGroups); }
		template<size_t i> using GroupType = std::tuple_element_t<i, std::tuple<detail::ParamGroupWrapper<TGroups>...>>;
		template<size_t i> static constexpr size_t GroupSize = GroupType<i>::size();
		const ShaderParamNext* ParamsBase() const { return reinterpret_cast<const ShaderParamNext*>(this); }

		template<typename TUniforms, typename TSpecConsts>
		void PreDraw(IMaterialVar** params, TUniforms& uniformBuf, TSpecConsts& specConsts) const
		{
			(TGroups::PreDraw(params, &uniformBuf, &specConsts), ...);
		}

	private:
		template<typename T>
		void InitParamIndices(size_t& paramIndex)
		{
			for (size_t i = 0; i < detail::ParamGroupWrapper<T>::size(); i++)
			{
				if (detail::ParamGroupWrapper<T>::LocalBase()[i].InitIndex(Util::SafeConvert<int>(paramIndex)))
					paramIndex++;
			}
		}

		template<typename T, typename TParams, ShaderFlags_t FLAGS> friend class ShaderNext;
		void InitParamGroups(IMaterialVar** params)
		{
			(TGroups::InitParamGroup(params), ...);
			(detail::LastDitchInitParams(params, detail::ParamGroupWrapper<TGroups>::LocalBase(), detail::ParamGroupWrapper<TGroups>::size()), ...);
		}
	};

	template<typename... TBufs>
	class ShaderSpecConstBufs final : public TBufs...
	{
	public:
		constexpr ShaderSpecConstBufs()
		{
			static_assert(std::has_unique_object_representations_v<ShaderSpecConstBufs<TBufs...>>,
				"Must have no padding, so we can be memcmp'd");
		}
	};

	template<typename TBuffer, typename... TLayouts>
	class ShaderSpecConstLayouts final : public TLayouts...
	{
		using TLayout0 = std::tuple_element_t<0, std::tuple<TLayouts...>>;

		static constexpr size_t BUFFER_SIZE = sizeof(TBuffer);

		static constexpr size_t BUFFER_ELEMENT_COUNT = BUFFER_SIZE / 4;
		static_assert(sizeof(uint32_t) == 4);
		static_assert(sizeof(int32_t) == 4);
		static_assert(sizeof(float) == 4);

		static constexpr size_t LAYOUT_COUNT = sizeof...(TLayouts);
		static constexpr size_t LAYOUT_ENTRIES_SIZE = (0 + ... + sizeof(TLayouts));
		static constexpr size_t LAYOUT_ENTRY_COUNT = LAYOUT_ENTRIES_SIZE / sizeof(SpecConstLayoutEntry);

		static_assert(LAYOUT_ENTRY_COUNT == BUFFER_ELEMENT_COUNT,
			"Mismatching element count for info struct and buffer struct");

	public:
		const SpecConstLayoutEntry* begin() const
		{
			return reinterpret_cast<const SpecConstLayoutEntry*>(static_cast<const TLayout0*>(this));
		}
		const SpecConstLayoutEntry* end() const { return begin() + LAYOUT_ENTRY_COUNT; }
		static constexpr size_t GetLayoutEntryCount() { return LAYOUT_ENTRY_COUNT; }
		static constexpr size_t GetBufferSize() { return BUFFER_SIZE; }

		operator SpecConstLayoutCreateInfo() const
		{
			return SpecConstLayoutCreateInfo{ begin(), LAYOUT_ENTRY_COUNT };
		}
	};
#endif

	template<typename... TComponents>
	class ShaderComponents : public TComponents::Params...
	{
	protected:
		struct UniformBuf final : TComponents::UniformBuf...
		{

		};

		struct SpecConstBuf final : TComponents::SpecConstBuf...
		{

		};

		struct SpecConstLayout final : TComponents::template SpecConstLayout<SpecConstBuf>...
		{
			static constexpr size_t LAYOUT_ENTRIES_SIZE = (0 + ... + sizeof(TComponents::template SpecConstLayout<SpecConstBuf>));
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
		void InitParamGroups(IMaterialVar** params)
		{
			(TComponents::Params::InitParamGroup(params), ...);
			(detail::LastDitchInitParams(params,
				reinterpret_cast<ShaderParamNext*>(static_cast<typename TComponents::Params*>(this)),
				sizeof(typename TComponents::Params) / sizeof(ShaderParamNext)), ...);
		}
	};

	namespace detail
	{
		static constexpr bool IsShaderComponents(void*) { return false; }
		template<typename... T> static constexpr bool IsShaderComponents(ShaderComponents<T...>*) { return true; }
	}

	template<typename T> static constexpr bool is_shader_components_v = detail::IsShaderComponents((T*)nullptr);
} }
