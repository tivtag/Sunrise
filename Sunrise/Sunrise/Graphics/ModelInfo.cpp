#include "ModelInfo.hpp"

#include "..\FileSystem.hpp"
#include "..\GravityResourceFile.hpp"

sr::ModelInfo::ModelInfo()
{
}

sr::ModelInfo::~ModelInfo()
{
    Reset();
}

void sr::ModelInfo::Reset() 
{
    valid = false;
	textures.clear();
	nodes.clear();
	m_volumeBoxes.clear();
}

bool sr::ModelInfo::ReadStream(std::istream& s) 
{
	Reset();
	if (!ReadHeader(s)) {
		return false;
	}

	if (!CheckHeader("GRSM")) {
		//_log(ROINT__DEBUG, "Invalid RSM header (%c%c%c%c)", magic[0], magic[1], magic[2], magic[3]);
		return false;
	}

	if (version.cver.major == 1 && version.cver.minor >= 1 && version.cver.minor <= 5) {
		// supported [1.1 1.5]
	}
	else {
		//_log(ROINT__DEBUG, "Unsupported RSM version (%u.%u)", m_version.cver.major, m_version.cver.minor);
		return false;
	}

	int i, n;
	s.read((char*)&m_animLen, sizeof(int));
	s.read((char*)&m_shadeType, sizeof(int));
	if (IsCompatibleWith(1,4)) {
		s.read((char*)&m_alpha, sizeof(unsigned char));
	}
	else {
		m_alpha = 0xFF;
	}
	s.read(m_reserved, sizeof(m_reserved));

	// read textures
	s.read((char*)&n, sizeof(int));
	if (s.fail()) {
		Reset();
		return false;
	}
	if (n > 0) {
		textures.resize((unsigned int)n);
		for (i = 0; i < n; i++) {
			Texture& tex = textures[i];
			s.read((char*)&tex, sizeof(Texture));
			tex.name[39] = 0;
		}
	}

	// read nodes
	s.read(m_mainNode, sizeof(m_mainNode));
	s.read((char*)&n, sizeof(int));
	if (s.fail()) {
		Reset();
		return false;
	}

	if (n > 0) {
		nodes.resize((unsigned int)n);
		for (i = 0; i < n; i++) {
			Node& node = nodes[i];
			if (!node.ReadStream(s, version, i == 0)) {
				Reset();
				return false;
			}
		}
	}

	if (nodes.size() == 1)
		nodes[0].is_only = true;

	// read position animation of main node
	if (!IsCompatibleWith(1,5)) {
		Node* mainnode = NULL;
		for (i = 0; i < (int)nodes.size(); i++) {
			if (strcmp(nodes[i].name, m_mainNode) == 0) {
				mainnode = &nodes[i];
				break;
			}
		}

		if (mainnode == NULL) {
			//_log(ROINT__DEBUG, "RSM main node not found (%s)", m_mainNode);
			Reset();
			return false;
		}
		s.read((char*)&n, sizeof(int));

		if (s.fail()) {
			Reset();
			return false;
		}

		if (n > 0) {
			mainnode->posKeyframes.resize((unsigned int)n);
			for (i = 0; i < n; i++) {
				PosKeyframe& frame = mainnode->posKeyframes[i];
				s.read((char*)&frame, sizeof(PosKeyframe));
			}
		}
	}

	// read volume boxes
	s.read((char*)&n, sizeof(int));
	if (s.fail()) {
		Reset();
		return false;
	}
	if (n > 0) {
		m_volumeBoxes.resize((unsigned int)n);
		if (IsCompatibleWith(1,3)) {
			for (i = 0; i < n; i++) {
				VolumeBox& box = m_volumeBoxes[i];
				s.read((char*)&box, sizeof(VolumeBox));
			}
		}
		else {
			for (i = 0; i < n; i++) {
				VolumeBox& box = m_volumeBoxes[i];
				s.read((char*)&box, sizeof(VolumeBox) - sizeof(int));
				box.flag = 0;
			}
		}
	}

	CalcBoundingBox();

	if (s.fail()) {
		Reset();
		return false;
	}
	
    valid = true;
	return true;
}

void sr::ModelInfo::CalcBoundingBox() {
//#if 1
//	unsigned int i;
//
//	nodes[0].CalcBoundingBox();
//	for (i = 1; i < nodes.size(); i++)
//		if (strcmp(nodes[i].parentname, nodes[0].name) == 0)
//			nodes[i].CalcBoundingBox();
//
//	for (i = 0; i < 3; i++) {
//		box.max[i] = nodes[0].getBoundingBox().max[i];
//		box.min[i] = nodes[0].getBoundingBox().min[i];
//	/*	for (unsigned int j = 1; j < nodes.size(); j++) {
//			if (strcmp(nodes[i].parentname, nodes[0].name) == 0){
//				box.max[i] = MAX(nodes[j].getBoundingBox().max[i], box.max[i]);
//				box.min[i] = MIN(nodes[j].getBoundingBox().min[i], box.min[i]);
//			}
//		}*/
//		box.offset[i] = (box.max[i] + box.min[i]) / 2.0f;
//		box.range[i] = (box.max[i] - box.min[i]) / 2.0f;
//	}
//#else
//	unsigned int i, j;
//	BoundingBox& meshbox = box;
//	box.max.x = -999999;
//	box.max.y = -999999;
//	box.max.z = -999999;
//
//	box.min.x = 999999;
//	box.min.y = 999999;
//	box.min.z = 999999;
//
//	box.range.x = 0;
//	box.range.y = 0;
//	box.range.z = 0;
//	for (i = 0; i < nodes.size(); i++) {
//		nodes[i].calcBoundingBox(nodes[0].transf);
//		meshbox = nodes[i].getBoundingBox();
//
//		for(j = 0; j < 3; j++) {
//			if (meshbox.max[j] > box.max[j])
//				box.max[j] = meshbox.max[j];
//			if (meshbox.min[j] < box.min[j])
//				box.min[j] = meshbox.min[j];
//		}
//	}
//
//	for(j = 0; j < 3; j++)
//		// box.range[j] = box.max[j] - box.min[j];
//		box.range[j] = (box.max[j] + box.min[j])/2.0f;
//#endif
}


void sr::ModelInfo::Node::Reset() {
	memset(name, 0, sizeof(name));
	memset(parentname, 0, sizeof(parentname));
    textures.clear();
    memset((char*)&offsetMT, 0, sizeof(offsetMT));
	memset((char*)&pos, 0, sizeof(pos));
	rotangle = 0;
	memset((char*)&rotaxis, 0, sizeof(rotaxis));
	scale.x = scale.y = scale.z = 1;
    vertices.clear();
    tvertices.clear();
    faces.clear();
	posKeyframes.clear();
    rotKeyframes.clear();
}

bool sr::ModelInfo::Node::ReadStream(std::istream& s, const sr::ObjectVersion& ver, bool main) 
{
	Reset();
	s.read(name, 40);
	s.read(parentname, 40);

	is_main = main;

	int i, n;
	// read texture indexes
	s.read((char*)&n, sizeof(int));
	if (s.fail())
		return(false);
	if (n > 0) {
		textures.resize((unsigned int)n);
		for (i = 0; i < n; i++) {
			int& index = textures[i];
			s.read((char*)&index, sizeof(int));
		}
	}

	s.read((char*)&offsetMT, sizeof(offsetMT));
	s.read((char*)&pos, sizeof(pos));
	s.read((char*)&rotangle, sizeof(float));
	s.read((char*)&rotaxis, sizeof(rotaxis));
	s.read((char*)&scale, sizeof(scale));

	// read vertex indexes
	s.read((char*)&n, sizeof(int));
	if(s.fail())
		return false;

	if (n > 0) {
		vertices.resize((unsigned int)n);
		for (i = 0; i < n; i++) {
			Vertex& vert = vertices[i];
			s.read((char*)&vert, sizeof(Vertex));
		}
	}

	// read texture vertex indexes
	s.read((char*)&n, sizeof(int));
	if (s.fail())
		return false;

	if (n > 0) {
		tvertices.resize((unsigned int)n);
		if (ver.cver.major > 1 || (ver.cver.major == 1 && ver.cver.minor >= 2)) {
			for (i = 0; i < n; i++) {
				TVertex& tvert = tvertices[i];
				s.read((char*)&tvert, sizeof(TVertex));
			}
		} else {
			for (i = 0; i < n; i++) {
				TVertex& tvert = tvertices[i];
				tvert.color = 0xFFFFFFFF;
				s.read((char*)&tvert.u, sizeof(float));
				s.read((char*)&tvert.v, sizeof(float));
			}
		}
	}

	// read faces
	s.read((char*)&n, sizeof(int));
	if (s.fail())
		return false;

	if (n > 0) {
		faces.resize((unsigned int)n);
		if (ver.cver.major > 1 || (ver.cver.major == 1 && ver.cver.minor >= 2)) {
			for (i = 0; i < n; i++) {
				Face& face = faces[i];
				s.read((char*)&face, sizeof(Face));
			}
		} else {
			for (i = 0; i < n; i++) {
				Face& face = faces[i];
				s.read((char*)&face, sizeof(Face) - sizeof(int));
				face.smoothGroup = 0;
			}
		}
	}

	// read position keyframes
	if (ver.cver.major > 1 || (ver.cver.major == 1 && ver.cver.minor >= 5)) {
		s.read((char*)&n, sizeof(int));
		if (s.fail())
			return false;

		if (n > 0) {
			posKeyframes.resize((unsigned int)n);
			for (i = 0; i < n; i++) {
				PosKeyframe& frame = posKeyframes[i];
				s.read((char*)&frame, sizeof(PosKeyframe));
			}
		}
	}

	// read rotation keyframes
	s.read((char*)&n, sizeof(int));
	if (s.fail())
		return false;
	if (n > 0) {
		rotKeyframes.resize((unsigned int)n);
		for (i = 0; i < n; i++) {
			RotKeyframe& frame = rotKeyframes[i];
			s.read((char*)&frame, sizeof(RotKeyframe));
		}
	}

	return !s.fail();
}

void sr::ModelInfo::Node::CalcBoundingBox()
{
}

sr::ModelInfo::Node::Node()
{
}

sr::ModelInfo::Node::~Node()
{
    Reset();
}

void sr::ModelInfo::ExtractFiles(sr::GravityResourceFile& grf, sr::FileSystem& fileSystem)
{
    for(std::size_t i = 0; i < TextureCount(); ++i)
    {
        std::string texture = "data\\texture\\" + GetTexture(i);
            
        if( grf.HasFile(texture) )
        {
            std::cout << "found " << texture << std::endl;
        
            std::string targetFile = fileSystem.GetFullPath( texture );
            std::string targetDir = fileSystem.GetFolder( targetFile );
            fileSystem.CreateFolder(targetDir);
        
            grf.SaveFile( texture, targetFile );
        }
        else
        {
            std::cout << "missing " << texture << std::endl;
        }
    }         
}
