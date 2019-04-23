#include "KeyValues.h"

#include <tier1/KeyValues.h>

namespace
{
	class KeyValuesDumpContextAsStdString : public IKeyValuesDumpContextAsText
	{
	public:
		KeyValuesDumpContextAsStdString(std::string& target) :
			m_Target(target)
		{
		}

	protected:
		bool KvWriteText(char const* szText) override
		{
			m_Target.append(szText);
			return true;
		}

	private:
		std::string& m_Target;
	};
}

bool Util::KVToString(const KeyValues& kv, std::string& str)
{
	// Workaround for broken const-correctness
	KeyValues::AutoDelete ad(kv.MakeCopy());
	return KVToString(*ad, str);
}

bool Util::KVToString(KeyValues& kv, std::string& str)
{
	KeyValuesDumpContextAsStdString ctx(str);
	return kv.Dump(&ctx);
}

std::string Util::KVToString(const KeyValues& kv)
{
	std::string retVal;
	if (!KVToString(kv, retVal))
		return {};

	return retVal;
}

std::string Util::KVToString(KeyValues& kv)
{
	std::string retVal;
	if (!KVToString(kv, retVal))
		return {};

	return retVal;
}
