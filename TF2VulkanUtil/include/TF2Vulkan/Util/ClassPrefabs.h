#pragma once

namespace Util
{
	struct DisableCopy
	{
		constexpr DisableCopy() = default;
		DisableCopy(const DisableCopy&) = delete;
		DisableCopy& operator=(const DisableCopy&) = delete;
	};

	struct DisableMove
	{
		constexpr DisableMove() = default;
		DisableMove(const DisableMove&) = delete;
		DisableMove& operator=(const DisableMove&) = delete;
	};

	struct DisableMoveCopy : public DisableMove, public DisableCopy
	{
	};
}
