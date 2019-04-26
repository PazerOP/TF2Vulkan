#pragma once

#include <stdexcept>
#include <optional>

namespace TF2Vulkan
{
	struct ExceptionData
	{
		std::string m_FuncSig;
		std::string m_Function;
		std::string m_File;
		size_t m_Line;
	};

	class VulkanException : public std::runtime_error
	{
	public:
		VulkanException(vk::Result code, ExceptionData&& data);
		VulkanException(VkResult code, ExceptionData&& data);
		VulkanException(std::string&& usrMsg, ExceptionData&& data);
		VulkanException(std::string&& usrMsg, VkResult code, ExceptionData&& data);
		VulkanException(std::string&& usrMsg, vk::Result code, ExceptionData&& data);

	private:
		VulkanException(std::string&& usrMsg, std::optional<vk::Result> code, ExceptionData&& data);

		static std::string CreateFullMessage(const std::string& usrMsg,
			const std::optional<vk::Result>& code, const ExceptionData& data);

		ExceptionData m_Data;
		std::string m_UserMessage;
		std::optional<vk::Result> m_ResultCode;
	};
}

#define EXCEPTION_DATA() ::TF2Vulkan::ExceptionData{ __FUNCSIG__, __FUNCTION__, __FILE__, __LINE__ }
