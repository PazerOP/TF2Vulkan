
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/Macros.h>

#include <materialsystem/IShader.h>
#include <shaderlib/ShaderDLL.h>

#include <tier0/icommandline.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>

#include <cstdint>

namespace
{
	class IShaderDLLInternal
	{
	public:
		virtual bool Connect(CreateInterfaceFn factory, uint8_t something) = 0;
		virtual void Disconnect(uint8_t something) = 0;

		virtual int ShaderCount() const = 0;

		virtual IShader* GetShader(int index) = 0;
	};

	class VulkanShaderDLL final : public IShaderDLLInternal, public IShaderDLL
	{
		// Inherited via IShaderDLLInternal
		bool Connect(CreateInterfaceFn factory, uint8_t something) override;
		void Disconnect(uint8_t something) override { NOT_IMPLEMENTED_FUNC(); }
		int ShaderCount() const override { NOT_IMPLEMENTED_FUNC(); }
		IShader* GetShader(int index) override { NOT_IMPLEMENTED_FUNC(); }

		// Inherited via IShaderDLL
		void InsertShader(IShader* pShader) override { NOT_IMPLEMENTED_FUNC(); }
	};
}

static VulkanShaderDLL& GetShaderDLLRef()
{
	static VulkanShaderDLL s_VulkanShaderDLL;
	return s_VulkanShaderDLL;
}

static IShaderDLLInternal* GetShaderDLLInternal()
{
	return &GetShaderDLLRef();
}
IShaderDLL* GetShaderDLL()
{
	return &GetShaderDLLRef();
}

EXPOSE_INTERFACE_FN([] { return (void*)GetShaderDLLInternal(); }, IShaderDLLInternal, "ShaderDLL004");

bool VulkanShaderDLL::Connect(CreateInterfaceFn factory, uint8_t something)
{
	ConnectTier1Libraries(&factory, 1);
	ConnectTier2Libraries(&factory, 1);
	ConnectTier3Libraries(&factory, 1);

	if (!CommandLine()->CheckParm("-insecure"))
	{
		Error("Attempted to use stdshader_vulkan.dll without -insecure");
		return false;
	}

	return true;
}
