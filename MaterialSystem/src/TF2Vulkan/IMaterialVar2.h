#pragma once

#include <materialsystem/imaterialvar.h>

namespace TF2Vulkan
{
	class IMaterialVar2 : public IMaterialVar
	{
	public:
		virtual void CopyFrom(const IMaterialVar* other) = 0;
		void CopyFrom(IMaterialVar* other) override final { CopyFrom(const_cast<const IMaterialVar*>(other)); }

		virtual void GetFourCCValue(FourCC* type, const void** data) const = 0;
		void GetFourCCValue(FourCC* type, void** data) override final
		{
			return GetFourCCValue(type, const_cast<const void**>(data));
		}

		virtual const VMatrix& GetMatrixValue() const = 0;
		const VMatrix& GetMatrixValue() override final { return Util::as_const(this)->GetMatrixValue(); }
	};
}
