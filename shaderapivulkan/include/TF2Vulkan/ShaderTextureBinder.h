#pragma once

#include "AlignedTypes.h"
#include "TF2Vulkan/Util/InPlaceVector.h"

namespace TF2Vulkan
{
	static constexpr size_t MAX_SHADER_TEXTURE_BINDINGS = 32;
	class ShaderTextureBinder final
	{
	public:
		void AddBinding(ITexture* tex, uint32_t& locationSpecConst)
		{
			assert(tex);

			for (uint32_t i = 0; i < m_Bindings.size(); i++)
			{
				auto& existing = m_Bindings[i];
				if (existing.m_Texture == tex)
				{
					locationSpecConst = i;
					return;
				}
			}

			auto& newBinding = m_Bindings.emplace_back();
			newBinding.m_Texture = tex;
			locationSpecConst = m_Bindings.size() - 1;
		}

		auto begin() const { return m_Bindings.begin(); }
		auto end() const { return m_Bindings.end(); }

	private:
		struct Binding
		{
			ITexture* m_Texture = nullptr;
		};
		Util::InPlaceVector<Binding, MAX_SHADER_TEXTURE_BINDINGS> m_Bindings;
	};
}
