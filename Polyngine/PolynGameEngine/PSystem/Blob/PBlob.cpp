#include "PBlob.h"
#include "Assert.h"
#include <fstream>

namespace PBlob
{
	binary_blob_t LoadBinaryBlob(const char* path)
	{
		binary_blob_t blob;

		std::fstream file{ path, std::ios_base::in | std::ios_base::binary };

		assert(file.is_open());

		if (file.is_open())
		{
			file.seekg(0, std::ios_base::end);
			blob.resize(file.tellg());
			file.seekg(0, std::ios_base::beg);

			file.read((char*)blob.data(), blob.size());

			file.close();
		}

		return std::move(blob);
	}
}
