#pragma once

#include <memory>
#include <glm\glm.hpp>

#include "..\FileSystem.hpp"
#include "..\Resource.hpp"

namespace sr
{	
	class Texture : public Resource
	{
	public:
		glm::uvec2 Size() const { return size; }
		Texture();

		bool LoadFromFile(const std::string& relativePath, const sr::FileSystem& fileSystem);
		virtual void Unload();
		
		void Bind();
	private:
		glm::uvec2 size;
		unsigned int id;
	};

    typedef std::shared_ptr<Texture> TexturePtr;
}