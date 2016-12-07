#include "FileSystem.hpp"
#include <direct.h>
#include <boost\algorithm\string.hpp>

#include <Windows.h>

sr::FileSystem::FileSystem(const std::string& _baseFolder)
	: baseFolder(_baseFolder)
{
}

std::string sr::FileSystem::GetFullPath(const std::string& relativePath) const
{
	return baseFolder + relativePath;
}

bool sr::FileSystem::CreateFolder(const std::string& _path)
{
    const char* path = _path.c_str();

    char * buffer = new char[(strlen(path) + 12) * sizeof(char)];
    sprintf(buffer, "mkdir -p \"%s\"", path);
    int result = system(buffer);
    delete buffer;
    return result;

 ////   std::string rpath = boost::replace_all_copy( path, "\\", "/");
 //   const char* cpath = path.c_str();

 //   BOOL result = ::CreateDirectoryA(cpath, NULL);
 //   return result;
}

std::string sr::FileSystem::GetFolder(const std::string& fullPath) const
{
    auto index = fullPath.find_last_of( "\\" );
    
    if(index != std::string::npos)
    {
        std::string dir = fullPath.substr(0, index+1);
        return dir;
    }
    else
    {
        return "";
    }
}
