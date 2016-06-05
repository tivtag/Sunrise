#include "GravityFile.hpp"

bool sr::GravityFile::IsCompatibleWith(unsigned char major, unsigned char minor) const 
{
	return ((version.cver.major == major) && (version.cver.minor >= minor)) || version.cver.major > major;
}

bool sr::GravityFile::IsCompatibleWith(short ver) const
{
	ObjectVersion v;
	v.sver = ver;
	return IsCompatibleWith(v.cver.major, v.cver.minor);
}

bool sr::GravityFile::ReadHeader(std::istream &s)
{
	s.read((char*)&magic, magicSize);
	s.read((char*)&version, 2);
	return !s.fail();
}

bool sr::GravityFile::CheckHeader(const std::string& s) const 
{
	if(strncmp(s.c_str(), magic, magicSize))
		return false;
	return true;
}

sr::GravityFile::GravityFile()
    : valid(false), magicSize(4)
{
	version.sver = 0;
	memset(magic, 0, 4);
}
