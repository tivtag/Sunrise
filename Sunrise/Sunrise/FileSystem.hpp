#pragma once

#include <string>

namespace sr
{	
	/*
		Provides platform-independent access to the filesystem.
	*/
	class FileSystem 
	{
	public:
		FileSystem(const std::string& baseFolder);

		std::string GetFullPath(const std::string& relativePath) const;
        std::string GetFolder(const std::string& fullPath) const;
       
        bool HasFolder(const std::string&);
        bool CreateFolder(const std::string&);
        
        std::string ReadAllText(const std::string& path) const;

	private:
		const std::string baseFolder;
	};
}
