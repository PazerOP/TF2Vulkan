#include "stdafx.h"

using namespace TF2Vulkan;

VulkanException::VulkanException(vk::Result code, ExceptionData&& data) :
	VulkanException({}, code, std::move(data))
{
}

VulkanException::VulkanException(VkResult code, ExceptionData&& data) :
	VulkanException(vk::Result(code), std::move(data))
{
}

VulkanException::VulkanException(std::string&& usrMsg, ExceptionData&& data) :
	VulkanException(std::move(usrMsg), std::nullopt, std::move(data))
{
}

VulkanException::VulkanException(std::string&& usrMsg, VkResult code, ExceptionData&& data) :
	VulkanException(std::move(usrMsg), vk::Result(code), std::move(data))
{
}

VulkanException::VulkanException(std::string&& usrMsg, vk::Result code, ExceptionData&& data) :
	VulkanException(std::move(usrMsg), std::optional(code), std::move(data))
{
}

VulkanException::VulkanException(std::string&& usrMsg, std::optional<vk::Result> code, ExceptionData&& data) :
	std::runtime_error(CreateFullMessage(usrMsg, code, data)),
	m_Data(std::move(data)),
	m_UserMessage(std::move(usrMsg)),
	m_ResultCode(std::move(code))
{
}

std::string VulkanException::CreateFullMessage(const std::string& usrMsg,
	const std::optional<vk::Result>& code, const ExceptionData& data)
{
	char buf[1024];
	sprintf_s(buf, "[TF2Vulkan] Exception thrown: %s\n\tFunction: %s\n\tFile: %s:%zu",
		code ? vk::to_string(*code).c_str() : "",
		data.m_FuncSig.c_str(), data.m_File.c_str(), data.m_Line);

	Warning("%s\n", buf);

	if (IsDebuggerPresent())
		__debugbreak();

	return std::string(buf);
}
