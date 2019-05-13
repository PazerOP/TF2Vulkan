#pragma once

#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/std_string.h>
#include <TF2Vulkan/Util/std_utility.h>
#include <TF2Vulkan/Util/std_variant.h>
#include <TF2Vulkan/Util/std_vector.h>

namespace TF2Vulkan
{
	struct ShaderSpecConstant
	{
		bool operator==(const ShaderSpecConstant& other) const noexcept
		{
			return m_Name == other.m_Name &&
				m_Value == other.m_Value;
		}

		std::string m_Name;
		std::variant<bool, int, float> m_Value;
	};

	struct ShaderInstanceSettingsBase
	{
		ShaderInstanceSettingsBase() = default;
		ShaderInstanceSettingsBase(const std::initializer_list<ShaderSpecConstant>& specConstants) :
			m_SpecConstants(specConstants)
		{
		}

		bool operator==(const ShaderInstanceSettingsBase& other) const noexcept
		{
			return m_SpecConstants == other.m_SpecConstants;
		}
		std::vector<ShaderSpecConstant> m_SpecConstants;
	};
	struct PSInstanceSettings : ShaderInstanceSettingsBase
	{
		using ShaderInstanceSettingsBase::ShaderInstanceSettingsBase;
	};
	struct VSInstanceSettings : ShaderInstanceSettingsBase
	{
		using ShaderInstanceSettingsBase::ShaderInstanceSettingsBase;
	};

	enum class ShaderType
	{
		Pixel,
		Vertex,
	};

	class IShaderInstance
	{
	public:
		virtual const char* GetName() const = 0;
		virtual const ShaderInstanceSettingsBase& GetBaseSettings() const = 0;
		virtual ShaderType GetShaderType() const = 0;
	};
	class IPSInstance : public virtual IShaderInstance
	{
	public:
		const PSInstanceSettings& GetSettings() const { return static_cast<const PSInstanceSettings&>(GetBaseSettings()); }
		ShaderType GetShaderType() const override final { return ShaderType::Pixel; }
	};
	class IVSInstance : public virtual IShaderInstance
	{
	public:
		const VSInstanceSettings& GetSettings() const { return static_cast<const VSInstanceSettings&>(GetBaseSettings()); }
		ShaderType GetShaderType() const override final { return ShaderType::Vertex; }
	};

	class IShaderInstanceManager
	{
	public:
		virtual const IPSInstance* FindOrCreatePSInstance(const char* name, const PSInstanceSettings& settings = {}) = 0;
		virtual const IVSInstance* FindOrCreateVSInstance(const char* name, const VSInstanceSettings& settings = {}) = 0;
	};
}

STD_HASH_DEFINITION(TF2Vulkan::ShaderInstanceSettingsBase, );

STD_HASH_DEFINITION(TF2Vulkan::PSInstanceSettings,
	static_cast<const TF2Vulkan::ShaderInstanceSettingsBase&>(v)
);

STD_HASH_DEFINITION(TF2Vulkan::VSInstanceSettings,
	static_cast<const TF2Vulkan::ShaderInstanceSettingsBase&>(v)
);
