#include "WorldInfo.hpp"
#include <map>
#include "..\FileSystem.hpp"
#include "..\GravityResourceFile.hpp"

bool sr::WorldInfo::ReadStream(std::istream& s) {
	Reset();
	if (!ReadHeader(s)) {
		return(false);
	}

	if (!CheckHeader("GRSW")) {
		return(false);
	}

	if (version.cver.major == 1 && version.cver.minor >= 2 && version.cver.minor <= 9) {
		// supported [1.2 1.9]
	}
	else if (version.cver.major == 2 && version.cver.minor <= 1) {
		// supported [2.0 2.1]
	}
	else {
		return(false);// unsupported version
	}

	// read filenames
	s.read(iniFile, 40);
	s.read(gndFile, 40);
	if (IsCompatibleWith(1, 4)) {
		s.read(gatFile, 40);
	}
	else {
		memset(gatFile, 0, 40);
	}
	s.read(scrFile, 40);
	iniFile[39] = gndFile[39] = gatFile[39] = scrFile[39] = 0;

	// read water
	if (IsCompatibleWith(1,3))
    {
		s.read((char*)&water.level, sizeof(float));
	}
	else 
    {
		water.level = 0.0f;
	}

	if (IsCompatibleWith(1,8))
    {
		s.read((char*)&water.type, sizeof(int));
		s.read((char*)&water.waveHeight, sizeof(float));
		s.read((char*)&water.waveSpeed, sizeof(float));
		s.read((char*)&water.wavePitch, sizeof(float));
	}
	else 
    {
		water.type = 0;
		water.waveHeight = 1.0f;
		water.waveSpeed = 2.0f;
		water.wavePitch = 50.0f;
	}

	if (IsCompatibleWith(1,9)) 
    {
		s.read((char*)&water.animSpeed, sizeof(int));
	}
	else
    {
		water.animSpeed = 3;
	}

	// read light
	if(IsCompatibleWith(1,5))
    {
		s.read((char*)&light.longitude, sizeof(int));
		s.read((char*)&light.latitude, sizeof(int));
		s.read((char*)&light.diffuse, sizeof(float) * 3);
		s.read((char*)&light.ambient, sizeof(float) * 3);
	}
	else
    {
		light.longitude = 45;
		light.latitude = 45;
		light.diffuse[0] = light.diffuse[1] = light.diffuse[2] = 1.0f;
		light.ambient[0] = light.ambient[1] = light.ambient[2] = 0.3f;
	}

	if (IsCompatibleWith(1,7)) 
    {
		s.read((char*)&light.ignored, sizeof(float));
	}

	// read ground
	if (IsCompatibleWith(1,6)) 
    {
		s.read((char*)&ground, sizeof(Ground));
	}
	else 
    {
		ground.top = -500;
		ground.bottom = 500;
		ground.left = -500;
		ground.right = 500;
	}

	// read objects
	int i, nObjects;
	s.read((char*)&nObjects, sizeof(int));
	if (nObjects > 0)
		objects.resize((unsigned int)nObjects);

	for(i = 0; i < nObjects; ++i) 
    {
		objects[i] = Object::readStream(s, version);

		if (objects[i] == NULL)
        {
			Reset();
			return false;
		}
	}

	// read quadtree
	if (IsCompatibleWith(2,1))
    {
		//unsigned int i = 0;
		//quadTree.resize(1365);// 4^0 + 4^1 + 4^2 + 4^3 + 4^4 + 4^5
		//ReadQuadTree(s, 0, i);
	}

	if (s.fail()) 
    {
		Reset();
		return false;
	}

	valid = true;
	return true;
}

sr::WorldInfo::WorldInfo()
{
    Reset();
}

sr::WorldInfo::~WorldInfo()
{
    Reset();
}

void sr::WorldInfo::Reset() 
{
	valid = false;
	memset(iniFile, 0, 40);
	memset(gndFile, 0, 40);
	memset(gatFile, 0, 40);
	memset(scrFile, 0, 40);

	unsigned int i, n = objects.size();
	for (i = 0; i < n; i++) {
		delete objects[i];
	}

	objects.clear();
	quadTree.clear();
}

std::vector<sr::WorldInfo::ModelObject*> sr::WorldInfo::Models() const
{
    std::vector<sr::WorldInfo::ModelObject*> models;

    for(auto itr = std::begin(objects); itr != std::end(objects); ++itr)
    {
        Object* obj = *itr;

        if(obj->isType(ModelType))
        {
            models.push_back(static_cast<sr::WorldInfo::ModelObject*>(obj));
        }
    }

    return models;
}

void sr::WorldInfo::ExtractFiles(sr::GravityResourceFile& grf, sr::FileSystem& fileSystem)
{
    auto models = Models();

    std::map<std::string, sr::WorldInfo::ModelObject*> maps;

    for(auto itr = std::begin(models); itr != std::end(models); ++itr)
    {
        ModelObject* const model = *itr;

        if(maps.find(model->ModelName()) == maps.end() )
        {
            const std::string resourceName = "data\\model\\" + model->ModelName();

            if( grf.HasFile(resourceName) )
            {
                std::cout << "found " << resourceName << std::endl;
        
                std::string targetFile = fileSystem.GetFullPath( resourceName );
                std::string targetDir = fileSystem.GetFolder( targetFile );
                fileSystem.CreateFolder(targetDir);
        
                grf.SaveFile( resourceName, targetFile );
            }
            else
            {
                std::cout << "missing " << resourceName << std::endl;
            }

            maps.insert(std::make_pair(model->ModelName(), model));
        }
    }
}


// ===== OBJECT
sr::WorldInfo::Object::Object(sr::WorldInfo::ObjectType t) : m_type(t) {
}

sr::WorldInfo::Object::~Object() {
}

sr::WorldInfo::Object* sr::WorldInfo::Object::readStream(std::istream& s, const sr::ObjectVersion& ver) {
	int objtype;
	s.read((char*)&objtype, sizeof(int));
	if (s.fail())
		return(NULL);

	switch ((ObjectType)objtype) {
		case ModelType:
			{
				ModelObject* obj = new ModelObject();
				if (!obj->readData(s, ver)) {
					delete obj;
					return(NULL);
				}
				return obj;
			}
			break;
		case LightType:
			{
				LightObject* obj = new LightObject();
				if (!obj->readData(s, ver)) {
					delete obj;
					return(NULL);
				}
				return obj;
			}
			break;
		case SoundType:
			{
				SoundObject* obj = new SoundObject();
				if (!obj->readData(s, ver)) {
					delete obj;
					return(NULL);
				}
				return obj;
			}
			break;
		case EffectType:
			{
				EffectObject* obj = new EffectObject();
				if (!obj->readData(s, ver)) {
					delete obj;
					return(NULL);
				}
				return obj;
			}
			break;
		default:
			fprintf(stderr, "ERROR! invalid object %d\n", objtype);
			return(NULL);
	}
}

bool sr::WorldInfo::Object::writeStream(std::ostream& s, const sr::ObjectVersion& ver) const
{
	s.write((char*)&m_type, sizeof(int));
	if (s.fail())
		return(false);

	bool ret = false;
	switch (m_type) 
    {
		case ModelType:
			ret = ((const ModelObject*)this)->writeData(s, ver);
			break;
		case LightType:
			ret = ((const LightObject*)this)->writeData(s, ver);
			break;
		case SoundType:
			ret = ((const SoundObject*)this)->writeData(s, ver);
			break;
		case EffectType:
			ret = ((const EffectObject*)this)->writeData(s, ver);
			break;
	}

	return ret;
}

sr::WorldInfo::ObjectType sr::WorldInfo::Object::getType() const
{
	return(m_type);
}

bool sr::WorldInfo::Object::isType(sr::WorldInfo::ObjectType t) const 
{
	return(m_type == t);
}

sr::WorldInfo::Object* sr::WorldInfo::Object::Copy() const
{
	Object* ret = NULL;
	switch(m_type) 
    {
		case ModelType:
			ret = new ModelObject(*((const ModelObject*)this));
			break;

		case LightType:
			ret = new LightObject(*((const LightObject*)this));
			break;

		case SoundType:
			ret = new SoundObject(*((const SoundObject*)this));
			break;

		case EffectType:
			ret = new EffectObject(*((const EffectObject*)this));
			break;
	}

	return ret;
}


// ===== MODEL
sr::WorldInfo::ModelObject::ModelObject() : Object(sr::WorldInfo::ModelType)
{
	memset(name, 0, sizeof(name));
	animType = 0;
	animSpeed = 1.0f;
	blockType = 0;
	memset(modelName, 0, sizeof(modelName));
	memset(nodeName, 0, sizeof(nodeName));
	pos[0] = pos[1] = pos[2] = 0.0f;
	rot[0] = rot[1] = rot[2] = 0.0f;
	scale[0] = scale[1] = scale[2] = 0.0f;
}

sr::WorldInfo::ModelObject::ModelObject(const ModelObject& obj) : Object(sr::WorldInfo::ModelType)
{
	*this = obj;
}

sr::WorldInfo::ModelObject::~ModelObject() {
}

bool sr::WorldInfo::ModelObject::readData(std::istream& s, const sr::ObjectVersion& ver) 
{
	if ((ver.cver.major == 1 && ver.cver.minor >= 3) || ver.cver.major > 1)
    {
		s.read(name, 40);
		name[39] = 0;
		s.read((char*)&animType, sizeof(int));
		s.read((char*)&animSpeed, sizeof(float));
		if (animSpeed < 0.0f || animSpeed >= 100.0f)
			animSpeed = 1.0f;// not [0 100[
		s.read((char*)&blockType, sizeof(int));
	}
	else
    {
		name[0] = 0;
		animType = 0;
		animSpeed = 1.0f;
		blockType = 0;
	}

	s.read(modelName, 80);
	s.read(nodeName, 80);
	s.read((char*)&pos, sizeof(float) * 3);
	s.read((char*)&rot, sizeof(float) * 3);
	s.read((char*)&scale, sizeof(float) * 3);
	return !s.fail();
}

bool sr::WorldInfo::ModelObject::writeData(std::ostream& s, const sr::ObjectVersion& ver) const
{
	if ((ver.cver.major == 1 && ver.cver.minor >= 3) || ver.cver.major > 1) {
		s.write(name, 40);
		s.write((char*)&animType, sizeof(int));
		s.write((char*)&animSpeed, sizeof(float));
		s.write((char*)&blockType, sizeof(int));
	}

	s.write(modelName, 80);
	s.write(nodeName, 80);
	s.write((char*)&pos, sizeof(float) * 3);
	s.write((char*)&rot, sizeof(float) * 3);
	s.write((char*)&scale, sizeof(float) * 3);
	return !s.fail();
}

void sr::WorldInfo::ModelObject::Dump(std::ostream& o, const std::string& pfx) const
{
	o << pfx << "ModelObject" << std::endl;
	o << pfx << "\tName: " << name << std::endl;
	o << pfx << "\tAnim type: " << animType << std::endl;
	o << pfx << "\tAnim speed: " << animSpeed << std::endl;
	o << pfx << "\tBlock type: " << blockType << std::endl;
	o << pfx << "\tModel name: " << modelName << std::endl;
	o << pfx << "\tNode name: " << nodeName << std::endl;
	o << pfx << "\tPos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
	o << pfx << "\tRot: " << rot[0] << ", " << rot[1] << ", " << rot[2] << std::endl;
	o << pfx << "\tScale: " << scale[0] << ", " << scale[1] << ", " << scale[2] << std::endl;
}

sr::WorldInfo::ModelObject& sr::WorldInfo::ModelObject::operator=(const ModelObject& obj) 
{
	memcpy(name, obj.name, sizeof(name));
	animType = obj.animType;
	animSpeed = obj.animSpeed;
	blockType = obj.blockType;
	memcpy(modelName, obj.modelName, sizeof(modelName));
	memcpy(nodeName, obj.nodeName, sizeof(nodeName));
	memcpy(pos, obj.pos, sizeof(pos));
	memcpy(rot, obj.rot, sizeof(rot));
	memcpy(scale, obj.scale, sizeof(scale));
	return *this;
}


// ===== LIGHT
sr::WorldInfo::LightObject::LightObject() : Object(sr::WorldInfo::LightType) 
{
}

sr::WorldInfo::LightObject::LightObject(const LightObject& obj) : Object(sr::WorldInfo::LightType)
{
	*this = obj;
}

sr::WorldInfo::LightObject::~LightObject()
{
}

bool sr::WorldInfo::LightObject::readData(std::istream& s, const sr::ObjectVersion& ver)
{
	s.read((char*)&name, 80);
	name[79] = 0;
	s.read((char*)&pos, sizeof(float) * 3);
	s.read((char*)&red, sizeof(int));
	s.read((char*)&green, sizeof(int));
	s.read((char*)&blue, sizeof(int));
	s.read((char*)&range, sizeof(float));
	return(!s.fail());
}

bool sr::WorldInfo::LightObject::writeData(std::ostream& s, const sr::ObjectVersion& ver) const
{
	s.write((char*)&name, 80);
	s.write((char*)&pos, sizeof(float) * 3);
	s.write((char*)&red, sizeof(int));
	s.write((char*)&green, sizeof(int));
	s.write((char*)&blue, sizeof(int));
	s.write((char*)&range, sizeof(float));
	return(!s.fail());
}

void sr::WorldInfo::LightObject::Dump(std::ostream& o, const std::string& pfx) const
{
	o << pfx << "LightObject" << std::endl;
	o << pfx << "\tName: " << name << std::endl;
	o << pfx << "\tPos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
	o << pfx << "\tRed: " << red << std::endl;
	o << pfx << "\tGreen: " << green << std::endl;
	o << pfx << "\tBlue: " << blue << std::endl;
	o << pfx << "\tRange: " << range << std::endl;
}

sr::WorldInfo::LightObject& sr::WorldInfo::LightObject::operator=(const LightObject& obj)
{
	memcpy(name, obj.name, sizeof(name));
	memcpy(pos, obj.pos, sizeof(pos));
	red = obj.red;
	green = obj.green;
	blue = obj.blue;
	range = obj.range;
	return *this;
}

// ===== SOUND
sr::WorldInfo::SoundObject::SoundObject() : Object(sr::WorldInfo::SoundType) 
{
}

sr::WorldInfo::SoundObject::SoundObject(const SoundObject& obj) : Object(sr::WorldInfo::SoundType) 
{
	*this = obj;
}

sr::WorldInfo::SoundObject::~SoundObject()
{
}

bool sr::WorldInfo::SoundObject::readData(std::istream& s, const sr::ObjectVersion& ver)
{
	s.read(name, 80);
	s.read(waveName, 80);
	name[79] = waveName[79] = 0;
	s.read((char*)&pos, sizeof(float) * 3);
	s.read((char*)&vol, sizeof(float));
	s.read((char*)&width, sizeof(int));
	s.read((char*)&height, sizeof(int));
	s.read((char*)&range, sizeof(float));
	if (ver.cver.major >= 2)
		s.read((char*)&cycle, sizeof(float));
	else
		cycle = 4.0f;

	return !s.fail();
}

bool sr::WorldInfo::SoundObject::writeData(std::ostream& s, const sr::ObjectVersion& ver) const
{
	s.write(name, 80);
	s.write(waveName, 80);
	s.write((char*)&pos, sizeof(float) * 3);
	s.write((char*)&vol, sizeof(float));
	s.write((char*)&width, sizeof(int));
	s.write((char*)&height, sizeof(int));
	s.write((char*)&range, sizeof(float));
	if (ver.cver.major >= 2)
		s.write((char*)&cycle, sizeof(float));

	return !s.fail();
}

void sr::WorldInfo::SoundObject::Dump(std::ostream& o, const std::string& pfx) const
{
	o << pfx << "SoundObject" << std::endl;
	o << pfx << "\tName: " << name << std::endl;
	o << pfx << "\tWave name: " << waveName << std::endl;
	o << pfx << "\tPos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
	o << pfx << "\tVol: " << vol << std::endl;
	o << pfx << "\tWidth: " << width << std::endl;
	o << pfx << "\tHeight: " << height << std::endl;
	o << pfx << "\tRange: " << range << std::endl;
	o << pfx << "\tCycle: " << cycle << std::endl;
}

sr::WorldInfo::SoundObject& sr::WorldInfo::SoundObject::operator = (const SoundObject& obj) 
{
	memcpy(name, obj.name, sizeof(name));
	memcpy(waveName, obj.waveName, sizeof(waveName));
	memcpy(pos, obj.pos, sizeof(pos));
	vol = obj.vol;
	width = obj.width;
	height = obj.height;
	range = obj.range;
	cycle = obj.cycle;
	return *this;
}


// ===== EFFECT
sr::WorldInfo::EffectObject::EffectObject() : Object(sr::WorldInfo::EffectType) 
{
}

sr::WorldInfo::EffectObject::EffectObject(const EffectObject& obj) : Object(sr::WorldInfo::EffectType)
{
	*this = obj;
}

sr::WorldInfo::EffectObject::~EffectObject() 
{
}

bool sr::WorldInfo::EffectObject::readData(std::istream& s, const sr::ObjectVersion& ver) 
{
	s.read(name, 80);
	name[79] = 0;
	s.read((char*)&pos, sizeof(float) * 3);
	s.read((char*)&type, sizeof(int));
	s.read((char*)&emitSpeed, sizeof(float));
	s.read((char*)&param, sizeof(float) * 4);
	return(!s.fail());
}

bool sr::WorldInfo::EffectObject::writeData(std::ostream& s, const sr::ObjectVersion& ver) const 
{
	s.write(name, 80);
	s.write((char*)&pos, sizeof(float) * 3);
	s.write((char*)&type, sizeof(int));
	s.write((char*)&emitSpeed, sizeof(float));
	s.write((char*)&param, sizeof(float) * 4);
	return !s.fail();
}

void sr::WorldInfo::EffectObject::Dump(std::ostream& o, const std::string& pfx) const 
{
	o << pfx << "EffectObject" << std::endl;
	o << pfx << "\tName: " << name << std::endl;
	o << pfx << "\tPos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
	o << pfx << "\tType: " << type << std::endl;
	o << pfx << "\tEmit speed: " << emitSpeed << std::endl;
	o << pfx << "\tParam: " << param[0] << ", " << param[1] << ", " << param[2] << ", " << param[3] << std::endl;
}

sr::WorldInfo::EffectObject& sr::WorldInfo::EffectObject::operator=(const EffectObject& obj)
{
	memcpy(name, obj.name, sizeof(name));
	memcpy(pos, obj.pos, sizeof(pos));
	type = obj.type;
	emitSpeed = obj.emitSpeed;
	memcpy(param, obj.param, sizeof(param));
	return(*this);
}
