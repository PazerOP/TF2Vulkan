#pragma once

#include <string>

class KeyValues;

namespace Util
{
	bool KVToString(const KeyValues& kv, std::string& str);
	bool KVToString(KeyValues& kv, std::string& str);
	std::string KVToString(const KeyValues& kv);
	std::string KVToString(KeyValues& kv);
}
