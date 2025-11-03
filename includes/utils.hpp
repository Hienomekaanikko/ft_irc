#pragma once

#include <string>

namespace utils {
	inline std::string trimCRLF(const std::string& s) {
		std::string out = s;
		if (!out.empty() && out.back() == '\n') out.pop_back();
		if (!out.empty() && out.back() == '\r') out.pop_back();
		return out;
	}
}

