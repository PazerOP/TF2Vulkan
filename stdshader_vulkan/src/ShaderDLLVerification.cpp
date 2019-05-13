#include <TF2Vulkan/Util/Macros.h>

#include <tier1/checksum_crc.h>
#include <tier1/checksum_md5.h>

#include <cstddef>

#undef _PREFAST_
#undef RTL_NUMBER_OF_V2
#include <Windows.h>

// Bypasses some questionable dll verification code

static HINSTANCE s_DLLInstance;

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	s_DLLInstance = hinstDLL;
	return TRUE;
}

namespace
{
	class VerifyFunctions final
	{
	public:
		virtual CRC32_t VerifyFunc1(const std::byte* data) final
		{
			const void* pVerifyFuncs = &s_VerifyFunctions;
			m_VerifyFunc1Data = data + 43;

			CRC32_t crc;
			CRC32_Init(&crc);
			CRC32_ProcessBuffer(&crc, m_VerifyFunc1Data, 4101);
			CRC32_ProcessBuffer(&crc, &s_DLLInstance, 4);
			CRC32_ProcessBuffer(&crc, &pVerifyFuncs, 4);
			CRC32_Final(&crc);

			return crc;
		}

		virtual int VerifyFunc2(int something1, int something2, int something3) const final
		{
			MD5Context_t ctx;
			MD5Init(&ctx);
			MD5Update(&ctx, reinterpret_cast<const unsigned char*>(m_VerifyFunc1Data + 43), 4058);
			MD5Final(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(m_VerifyFunc1Data)), &ctx);

			// Probably doesn't actually return anything?
			return 0;
		}

		virtual void VerifyFunc3() const { NOT_IMPLEMENTED_FUNC(); }
		virtual void VerifyFunc4() const { NOT_IMPLEMENTED_FUNC(); }
		virtual int VerifyFunc5() const final
		{
			return 32423; // V E R I F I C A T I O N
		}
		virtual void VerifyFunc6() const { NOT_IMPLEMENTED_FUNC(); }
		virtual void VerifyFunc7() const { NOT_IMPLEMENTED_FUNC(); }

		const std::byte* m_VerifyFunc1Data;

	} static s_VerifyFunctions;
}

extern "C" __declspec(dllexport) std::byte* _ftol3(std::byte* data)
{
	*reinterpret_cast<DWORD**>(data + 43) = reinterpret_cast<DWORD*>(&s_VerifyFunctions);
	return data;
}
