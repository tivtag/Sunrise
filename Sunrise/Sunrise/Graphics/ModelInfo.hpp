#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>			// value_ptr

#include "../GravityFile.hpp"

namespace sr
{
    class GravityResourceFile;
    class FileSystem;

	/*
	   Gravity ModelInfo structure descriping a Gravity 3D Model
	*/
    class ModelInfo : public sr::GravityFile, boost::noncopyable
    {
    public:
#pragma pack(push,1)
	    struct Texture {
		    char name[40];
	    };

	    struct Vertex {
		    float x;
		    float y;
		    float z;
		    inline operator float* () { return &x; }
		    inline operator const float* () const { return &x; }
	    };

	    struct TVertex {
		    unsigned int color;
		    float u, v;
		    inline operator float* () { return &u; }
		    inline operator const float* () const { return &u; }
	    };

	    /** Triangle face */
	    struct Face {
		    unsigned short vertidx[3];
		    unsigned short tvertidx[3];
		    unsigned short texid;
		    unsigned short __padding;
		    int twoSide;
		    int smoothGroup;
	    };

	    struct PosKeyframe {
		    int frame;
		    float px;
		    float py;
		    float pz;
		    inline operator float* () { return &px; }
		    inline operator const float* () const { return &px; }
	    };

	    struct RotKeyframe {
		    int frame;
		    float qx;
		    float qy;
		    float qz;
		    float qw;
		    inline operator float* () { return &qx; }
		    inline operator const float* () const { return &qx; }
	    };

	    struct VolumeBox {
		    Vertex size;
		    Vertex pos;
		    Vertex rot;
		    int flag; //< if (flag != 0) size += {3,0,3};
	    };

	    struct BoundingBox {
		    Vertex max;
		    Vertex min;
		    Vertex offset;
		    Vertex range;
	    };

	    class Node {
	    public:
		    const BoundingBox& BoundBox() const;

		    Node();
		    ~Node();
            
		    void Reset();
		    bool ReadStream(std::istream& s, const sr::ObjectVersion& ver, bool main);

		    void CalcBoundingBox();

            std::size_t FaceCount() const { return faces.size(); }
            const Face& GetFace(const std::size_t index) const { return faces[index]; }
            
            std::size_t VertexCount() const { return vertices.size(); }
            const Vertex& GetVertex(const std::size_t index) const { return vertices[index]; }
                        
            std::size_t TextureVertexCount() const { return tvertices.size(); }
            const TVertex& GetTextureVertex(const std::size_t index) const { return tvertices[index]; }
                               
            glm::mat4 Rotation() const
            {
                glm::mat4 mat(1.0);

                if(rotaxis.x || rotaxis.y || rotaxis.z)
                {
                    const float degrees = glm::degrees(rotangle);
                    mat = glm::rotate(mat, degrees, glm::vec3(rotaxis.x, rotaxis.y, rotaxis.z));
                }

                return mat;
            }

        public:            
		    bool is_main;
		    bool is_only;

            char name[40];
            std::vector<PosKeyframe> posKeyframes;

            glm::mat4x4 Origin() const
            {
                glm::mat4x4 matrix(1.0);
                auto mat = glm::value_ptr(matrix);
                                
	            mat[0] = offsetMT[0];
	            mat[1] = offsetMT[1];
	            mat[2] = offsetMT[2];
	            mat[3] = 0;

	            mat[4] = offsetMT[3];
	            mat[5] = offsetMT[4];
	            mat[6] = offsetMT[5];
	            mat[7] = 0;

	            mat[8] = offsetMT[6];
	            mat[9] = offsetMT[7];
	            mat[10] = offsetMT[8];
	            mat[11] = 0;

	            mat[12] = 0;
	            mat[13] = 0;
	            mat[14] = 0;
	            mat[15] = 1;

                return matrix;

/*                if(rotaxis.x || rotaxis.y || rotaxis.z)
                {
                    mat = glm::rotate(mat, rotangle, glm::vec3(rotaxis.x, rotaxis.y, rotaxis.z));
                }

                if(scale.x != 1 || scale.y != 1 || scale.z != 1)
                {
                    mat = glm::scale(mat, glm::vec3(scale.x, scale.y, scale.z));
                }
                
                mat = glm::translate(mat, glm::vec3(pos.x, pos.y, pos.z));  */ 
            }

	    private:
		    char parentname[40];
		    std::vector<int> textures; //< texture indexes
		    float offsetMT[12]; //< 3x4 matrix that identifies the axis and origin of this node
		    Vertex pos;
		    float rotangle; //< angle around the axis of rotation in radians
		    Vertex rotaxis; //< axis of rotation
		    Vertex scale;
		    std::vector<Vertex> vertices;
		    std::vector<TVertex> tvertices;
		    std::vector<Face> faces;
		    std::vector<RotKeyframe> rotKeyframes;
            
		    BoundingBox box;
	    };
    #pragma pack(pop)

    public:
        ModelInfo();
	    virtual ~ModelInfo();

        std::size_t TextureCount() const { return textures.size(); }        
        std::string GetTexture(const std::size_t index) const { return std::string(textures[index].name); }

        std::size_t NodeCount() const { return nodes.size(); }
        const Node& GetNode(const std::size_t index) const { return nodes[index]; }

        bool ReadStream(std::istream&);
        void ExtractFiles(sr::GravityResourceFile&, sr::FileSystem&);
    private:
	    void Reset();
		void CalcBoundingBox();

	    int m_animLen;
	    int m_shadeType;
	    unsigned char m_alpha;
	    char m_reserved[16];
	    std::vector<Texture> textures;
	    char m_mainNode[40];
	    std::vector<Node> nodes;
	    std::vector<VolumeBox> m_volumeBoxes;
    };

    typedef std::shared_ptr<sr::ModelInfo> ModelInfoPtr;
}
