#pragma once

#include <string>
#include <iostream>
#include <boost/noncopyable.hpp>

namespace sr
{	
	/*
		Repesents the version of a Gravity Object
	*/
    union ObjectVersion
    {
	    short sver;
	    struct _cver 
        {
		    unsigned char major;
		    unsigned char minor;
	    }
        cver;
    };

	/*
		Represents the base Gravity file header
	*/
    class GravityFile
    {
    protected:
        GravityFile();

        bool IsCompatibleWith(unsigned char major, unsigned char minor) const;
        bool IsCompatibleWith(short ver) const;        
        bool ReadHeader(std::istream &s);
        bool CheckHeader(const std::string& s) const;

    protected:
	    unsigned char magicSize;
	    char magic[4];
	    ObjectVersion version;
	    bool valid;
    };
}
