#pragma once

#include <string>

namespace sr
{
	class FileSystem 
	{
	public:
		FileSystem(const std::string& baseFolder);

		std::string GetFullPath(const std::string& relativePath) const;
        std::string GetFolder(const std::string& fullPath) const;

        bool CreateFolder(const std::string&);
        
	private:
		const std::string baseFolder;
	};
}
