#include "FileSystem.hpp"
#include "Platform.hpp"

#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <boost\algorithm\string.hpp>

sr::FileSystem::FileSystem(const std::string& _baseFolder)
	: baseFolder(_baseFolder)
{
}

std::string sr::FileSystem::GetFullPath(const std::string& relativePath) const
{
	return baseFolder + relativePath;
}

bool sr::FileSystem::HasFolder(const std::string& _path)
{
    const char* path = _path.c_str();
    struct _stat32 statBuffer;
    return (_stat32(path, &statBuffer) <= 0 && statBuffer.st_mode & S_IFDIR);
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

std::string sr::FileSystem::ReadAllText(const std::string& path) const
{
    const std::string fullPath = GetFullPath(path);

    std::ifstream stream(fullPath);
    std::stringstream str;

    if(stream.is_open())
    {        
        str << stream.rdbuf();
    }

    return str.str();
}

#ifdef SR_PLATFORM_WIN
#include <Windows.h>
#include "Log.hpp"

bool sr::FileSystem::CreateFolder(const std::string& _path)
{
    const char* name = _path.c_str();
    bool result = CreateDirectory(name,NULL);

    if(!result)
    {
        DWORD error = GetLastError();

        if(error!= ERROR_ALREADY_EXISTS)
        {
            std::cerr << error;
        }
    }
    
    if(true ||!HasFolder(_path))
    {
        const char* path = _path.c_str();  
        const std::size_t size = (strlen(path) + 12) * sizeof(char);
        
        char* buffer = new char[size];
        buffer[size-1] = '\0';
        sprintf(buffer, "mkdir -p \"%s\"", path);

        int result = system(buffer);
        delete buffer;
        return result;
    }
    else
    {
        return true;
    }
}
#else
bool sr::FileSystem::CreateFolder(const std::string& _path)
{
    return false;
}
#endif
