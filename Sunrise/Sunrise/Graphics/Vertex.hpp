#pragma once
#include <glm/glm.hpp>
#include "API.hpp"

namespace sr
{
	/*
	   Vertex structure used during rendering of the Gravity objects: Position3, UV2, Normal3
	*/
    struct Vertex
    {	
    #pragma pack(push,1)
	    glm::vec3 position;
	    glm::vec2 uv;
        glm::vec3 normal;
    #pragma pack(pop)

        template<typename TX, typename TY, typename TZ, typename TUV>
        Vertex(TX x, TY y, TZ z, TUV u, TUV v)
	        : position(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)), 
              uv(static_cast<GLfloat>(u), static_cast<GLfloat>(v))
        {
        }
    };
}
