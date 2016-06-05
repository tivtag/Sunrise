#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../GravityFile.hpp"

namespace sr
{
    class FileSystem;
    class GravityResourceFile;

    class WorldInfo : public GravityFile, boost::noncopyable
    {
    public:
	    enum ObjectType
        {
		    ModelType = 1,
		    LightType = 2,
		    SoundType = 3,
		    EffectType = 4
	    };

#pragma pack(push,1)
	    /** Abstract RSW object. */
	    class Object 
        {
	    protected:
		    Object(ObjectType);
	    public:
		    virtual ~Object();

		    static Object* readStream(std::istream& s, const ObjectVersion& ver);
		    bool writeStream(std::ostream& s, const ObjectVersion& ver) const;
		    virtual void Dump(std::ostream& s = std::cout, const std::string& pfx = "") const = 0;

		    /** Returns the object type */
		    ObjectType getType() const;

		    /** Check if we are the same type as the parameter */
		    bool isType(ObjectType) const;

		    /** Returns a copy of this object */
		    Object* Copy() const;

	    protected:
		    ObjectType m_type;
	    };

	    /** Model RSW object. */
	    class ModelObject : public Object
        {
	    public:
		    ModelObject();
		    ModelObject(const ModelObject& obj);
		    virtual ~ModelObject();

		    bool readData(std::istream& s, const ObjectVersion& ver);
		    bool writeData(std::ostream& s, const ObjectVersion& ver) const;
		    virtual void Dump(std::ostream& s = std::cout, const std::string& pfx = "") const;

		    ModelObject& operator = (const ModelObject& obj);

            std::string ModelName() const { return std::string(modelName); }

            glm::vec3 Position() const
            {
                return glm::vec3(pos[0], pos[1], pos[2]);
            }

            glm::vec3 Scale() const
            {
                return glm::vec3(scale[0], scale[1], scale[2]);
            }
                        
            glm::mat4 Rotation() const
            {
                glm::mat4 matrix(1.0f);

                matrix = glm::rotate(matrix, -rot[0], glm::vec3(1.0f, 0.0f, 0.0f));
                matrix = glm::rotate(matrix, rot[1], glm::vec3(0.0f, 1.0f, 0.0f));
                matrix = glm::rotate(matrix, -rot[2], glm::vec3(0.0f, 0.0f, 1.0f));

                return matrix;
            }

        public:
		    char name[40];
		    int animType;
		    float animSpeed;
		    int blockType;
		    char modelName[80]; //< RSM filename
		    char nodeName[80];
		    float pos[3];
		    float rot[3];
		    float scale[3];
	    };

	    /** Light source RSW object. */
	    class LightObject : public Object
        {
	    public:
		    LightObject();
		    LightObject(const LightObject& obj);
		    virtual ~LightObject();

		    bool readData(std::istream& s, const ObjectVersion& ver);
		    bool writeData(std::ostream& s, const ObjectVersion& ver) const;
		    virtual void Dump(std::ostream& s = std::cout, const std::string& pfx = "") const;

		    LightObject& operator = (const LightObject& obj);

		    char name[80];
		    float pos[3];
		    int red;
		    int green;
		    int blue;
		    float range;
	    };

	    /** Sound source RSW object. */
	    class SoundObject : public Object 
        {
	    public:
		    SoundObject();
		    SoundObject(const SoundObject& obj);
		    virtual ~SoundObject();

		    bool readData(std::istream& s, const ObjectVersion& ver);
		    bool writeData(std::ostream& s, const ObjectVersion& ver) const;
		    virtual void Dump(std::ostream& s = std::cout, const std::string& pfx = "") const;

		    SoundObject& operator = (const SoundObject& obj);

		    char name[80];
		    char waveName[80];
		    float pos[3];
		    float vol;
		    int width;
		    int height;
		    float range;
		    float cycle;
	    };

	    /** Effect RSW object. */
	    class EffectObject : public Object 
        {
	    public:
		    EffectObject();
		    EffectObject(const EffectObject& obj);
		    virtual ~EffectObject();

		    bool readData(std::istream& s, const ObjectVersion& ver);
		    bool writeData(std::ostream& s, const ObjectVersion& ver) const;
		    virtual void Dump(std::ostream& s = std::cout, const std::string& pfx = "") const;

		    EffectObject& operator = (const EffectObject& obj);

		    char name[80];
		    float pos[3];
		    int type;
		    float emitSpeed;
		    float param[4];
	    };

	    struct Ground {
		    int top;
		    int bottom;
		    int left;
		    int right;
	    };

	    struct Light {
		    int longitude; //< degrees
		    int latitude; //< degrees
		    float diffuse[3]; //< color
		    float ambient[3]; //< color
		    float ignored; //< ignored, float?
	    };

	    struct Water {
		    float level;
		    int type;
		    float waveHeight;
		    float waveSpeed;
		    float wavePitch;
		    int animSpeed;
	    };

	    struct QuadTreeNode {
		    float max[3];
		    float min[3];
		    float halfSize[3];
		    float center[3];
		    unsigned int child[4]; //< index of child node (generated), 0 for no child
	    };
#pragma pack(pop)

    public:
        WorldInfo();
        virtual ~WorldInfo();
	    virtual bool ReadStream(std::istream&);
        
        void ExtractFiles(sr::GravityResourceFile&, sr::FileSystem&);

        std::vector<ModelObject*> Models() const;
    private:
        void Reset();

    private:
        char iniFile[40];
	    char gndFile[40];
	    char gatFile[40];
	    char scrFile[40];
	    Water water;
	    Light light;
	    Ground ground;

	    std::vector<Object*> objects;
	    std::vector<QuadTreeNode> quadTree; //< 0 or 1365 entries (4^0 + 4^1 + 4^2 + 4^3 + 4^4 + 4^5, quadtree with 6 levels, depth-first ordering)
    };
}
