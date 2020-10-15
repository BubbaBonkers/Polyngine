#pragma once

#include <vector>

namespace PBlob
{
	using binary_blob_t = std::vector<uint8_t>;

	binary_blob_t LoadBinaryBlob(const char* path);
};

