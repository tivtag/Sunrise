#pragma once
#include "../Resource.hpp"
#include <glm/glm.hpp>

struct FIBITMAP;

namespace sr
{
	/*
        Represents an 2D-Image resource
     */
    class Image : public sr::Resource
    {
    public:
        Image(const std::string& fullPath);        
        void Unload();

        unsigned char* Bits() const;
		glm::uvec2 Size() const;

        bool SaveAsPng(const std::string& fullPath) const;
    private:
        void LoadFromFile(const std::string& fullPath);

    private:
        FIBITMAP* dib;
    };
}
