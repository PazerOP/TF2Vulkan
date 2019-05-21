#pragma once

namespace TF2Vulkan{ namespace RenderDoc
{
	void StartCapture();
	void EndCapture();
	void StartSingleFrameCapture();
	void QueueCapture();

	void FrameUpdate();
} }
