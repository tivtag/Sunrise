#include "GravityResourceFile.hpp"

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "zlib\zlib.h"
#include "DES.hpp"

sr::GravityResourceFile::GravityResourceFile()
    : opened(false), items(nullptr)
{
	memset((char*)&header, 0, sizeof(Header));
}

sr::GravityResourceFile::~GravityResourceFile()
{
    Close();
}

void sr::GravityResourceFile::Close() 
{
	if (!opened) 
    {
		std::cerr << "GRF already closed." << std::endl;
		return;
	}

	delete[] items;
	items = nullptr;

	if(stream.is_open())
	    stream.close();

	opened = false;
}

bool sr::GravityResourceFile::Open(const std::string& fileName) 
{
	if( opened )
    {
		std::cerr << "Can't open GRF file. Object already in use. Close it first." << std::endl;
		return(false);
	}

	stream.open(fileName.c_str(), std::ios_base::binary);

	if( !stream.is_open() )
    {
		std::cerr << "Can't open GRF file " << fileName << std::endl;
		stream.clear(); // Clear bad flag bit
		return false;
	}

    stream.read((char*)&header, sizeof(Header));
	fileCount = header.number2 - header.number1 - 7;

    // Go to the starting offset to read the GRF index.
	stream.seekg(header.fileTableOffset, std::ios_base::cur);
	if( stream.eof() )
    {
		stream.close();
		std::cerr << "Can't parse GRF file " << fileName << "Error seeking to position " << 46 + header.fileTableOffset << std::endl;
		return(false);
	}

    std::stringstream ss;
	if( (header.version & 0xFF00) == 0x0100) 
    {
		std::size_t cur = stream.tellg();
		std::size_t fileSize = (std::size_t)stream.seekg(0, std::ios::end).tellg();

		stream.seekg(cur, std::ios::beg);
		int size = fileSize - cur;
		
        unsigned char* body = new unsigned char[size];
		stream.read((char*)body, size);
		ss.write((char*)body, size);
		delete[] body;

		items = new FileTableItem_Ver1[fileCount];
	}
	else if( (header.version & 0xFF00) == 0x0200 )
    {
	    stream.read((char*)&fileTableHeader, sizeof(unsigned int) * 2);

	    fileTableHeader.body = new unsigned char[fileTableHeader.compressedLength];
	    fileTableHeader.uncompressedBody = new unsigned char[fileTableHeader.uncompressedLength];
	    stream.read((char*)fileTableHeader.body, fileTableHeader.compressedLength);
	    unsigned long ul = fileTableHeader.uncompressedLength;

	    uncompress(fileTableHeader.uncompressedBody, &ul, fileTableHeader.body, fileTableHeader.compressedLength);

	    if( ul == 0 )
        {
		    std::cerr << "Cannot uncompress FileTableHeader";
		    return false;
	    }

	    if( ul != fileTableHeader.uncompressedLength )
        {
		    std::cerr << "GRF Warning: Uncompressed lengths for FileTableHeader differ! (" << fileTableHeader.uncompressedLength << "/" << ul << ")" << std::endl;
	    }

	    ss.write((char*)fileTableHeader.uncompressedBody, fileTableHeader.uncompressedLength);

	    // Delete unneeded data
	    delete[] fileTableHeader.body;
	    fileTableHeader.body = nullptr;

	    delete[] fileTableHeader.uncompressedBody;
	    fileTableHeader.uncompressedBody = nullptr;

		items = new FileTableItem_Ver2[fileCount];
	}

	int i;
	for( i = 0; i < fileCount; ++ i)
		items[i].readStream(ss);

	opened = true;
	return true;
}

bool sr::GravityResourceFile::HasFile(const std::string& fileName) const 
{	
    for(int i = 0; i < fileCount; ++i)
    {
        const std::string itemFileName = items[i].filename;

        //if( itemFileName.find( "texture" ) != std::string::npos )
        //    std::cout << itemFileName << std::endl;

		if( boost::iequals(itemFileName, fileName) )
			return true;
    }

	return false;
}


std::size_t sr::GravityResourceFile::FileCount() const
{
    if(opened)
        return fileCount;
    else
        return 0;
}

std::string sr::GravityResourceFile::GetFileName(const std::size_t index) const
{
    if(!opened || index < 0 || index >= fileCount)
		return "";

	return items[index].filename;
}

const sr::GravityResourceFile::FileTableItem& sr::GravityResourceFile::GetItem(const std::size_t index) const
{
    return items[index];
}

sr::GravityResourceFile::FileTableItem::FileTableItem()
    : filename(""), compressedLength(0), compressedLengthAligned(0), uncompressedLength(0), flags(0), offset(0), cycle(0)
{
}

sr::GravityResourceFile::FileTableItem::FileTableItem(const FileTableItem& f)
{
    filename = f.filename;

	compressedLength = f.compressedLength;
	compressedLengthAligned = f.compressedLengthAligned;
	uncompressedLength = f.uncompressedLength;
	flags = f.flags;
	offset = f.offset;
	cycle = f.cycle;
}

sr::GravityResourceFile::FileTableItem::~FileTableItem() 
{
}

bool sr::GravityResourceFile::FileTableItem_Ver1::readStream(std::istream& ss)
{
	char buf[256];
	int idx;

	unsigned int len;
	ss.read((char*)&len, sizeof(len));
	ss.seekg(2, std::ios::cur);
	ss.read(buf, len-6);
    sr::des::decodeFilename((unsigned char*)&buf[0], len);
	idx = len;

	cycle = 0;

	buf[idx] = 0;
    this->filename = std::string( buf );

	unsigned int len1, len2, len3;
	ss.seekg(4, std::ios::cur);

	ss.read((char*)&len1, sizeof(unsigned int));
	ss.read((char*)&len2, sizeof(unsigned int));
	ss.read((char*)&len3, sizeof(unsigned int));
	compressedLength = len1 - len3 - 0x2CB;
	compressedLengthAligned = len2 - 0x92CB;
	uncompressedLength = len3;
	ss.read(&flags, 1);
	ss.read((char*)&offset, sizeof(unsigned int));

	// Setup for decryption
	static const char *suffix[] = {".act", ".gat", ".gnd", ".str"};
	bool a = true;

	if(flags == 0)
    {
		flags = 2;
	}
	else 
    {
        const char* filename = this->filename.c_str();

		for(int i=0; i<4; ++i)
        {
			if (strnicmp(strrchr(filename, '.'), suffix[i], 4) == 0) {
				a = false;
				break;
			}
		}

		if(a) 
        {
			int lop;
			int srccount;
			int srclen = compressedLength;
			for (lop = 10, srccount = 1; srclen >= lop; lop = lop * 10, srccount++);
			cycle = srccount;
		}
	}

	return(true);
}

bool sr::GravityResourceFile::FileTableItem_Ver2::readStream(std::istream& ss) 
{
	char buf[256];
	int idx;
	char c;
	idx = 0;
	c = -1;
	while (c != 0) 
    {
		ss.get(c);
		buf[idx++] = c;
	}

	cycle = 0;

	buf[idx] = 0;
	filename = std::string(buf);

	ss.read((char*)&compressedLength, sizeof(unsigned int));
	ss.read((char*)&compressedLengthAligned, sizeof(unsigned int));
	ss.read((char*)&uncompressedLength, sizeof(unsigned int));
	ss.read(&flags, 1);
	ss.read((char*)&offset, sizeof(unsigned int));

	// Setup for decryption
	if(flags == 3) 
    {
		int lop;
		int srccount;
		int srclen = compressedLength;
		for (lop = 10, srccount = 1; srclen >= lop; lop = lop * 10, srccount++);
		cycle = srccount;
	}

	return true;
}

sr::GravityResourceFile::FileTableItem& sr::GravityResourceFile::FileTableItem::operator=(const FileTableItem& f) 
{
    filename = f.filename;
    	
    compressedLength = f.compressedLength;
	compressedLengthAligned = f.compressedLengthAligned;
	uncompressedLength = f.uncompressedLength;
	flags = f.flags;
	offset = f.offset;
	cycle = f.cycle;
	return *this;
}


bool sr::GravityResourceFile::WriteFile(const std::string& s, std::ostream& out)
{
	if( !opened )
		return false;

	// search for file
	for (int i = 0; i < fileCount; ++i)
    {
        FileTableItem& item = items[i];

		if(boost::iequals(item.filename, s))
        {
			unsigned char *body;
			unsigned char *uncompressed;
			body = new unsigned char[item.compressedLengthAligned + 1024]; // 1K extra for safety
			uncompressed = new unsigned char[item.uncompressedLength + 1024]; // 1K extra for safety

			unsigned long ul;

			stream.seekg(item.offset + 46);
			stream.read((char*)body, item.compressedLengthAligned);

            if ((item.flags == 3) || (item.flags == 5) || ((header.version & 0xFF00) == 0x0100))
            {
				// DES encoded. Let's decode!
				sr::des::decode(body, item.compressedLengthAligned, item.cycle);
			}

			ul = item.uncompressedLength;
			int r;
			if ((r = uncompress(uncompressed, &ul, body, item.compressedLengthAligned)) != Z_OK)
            {
				switch(r) 
                {
					case Z_MEM_ERROR:
						std::cerr << "Error uncompressing data Z_MEM_ERROR";
						break;
					case Z_BUF_ERROR:
						std::cerr <<  "Error uncompressing data Z_BUF_ERROR";
						break;
					case Z_STREAM_ERROR:
						std::cerr << "Error uncompressing data Z_STREAM_ERROR";
						break;
					case Z_DATA_ERROR:
						std::cerr <<  "Error uncompressing data Z_DATA_ERROR";
						break;
					default:
						std::cerr <<  "Unknown error when uncompressing data: " << r;
				}
                std::cerr << std::endl;

				delete[] uncompressed;
				delete[] body;
				return false;
			}

			// _logif(((long)ul != m_items[i].uncompressedLength), ROINT__ERROR, "GRF Warning: Uncompressed lengths for file %s differs!", m_items[i].filename)

			out.write((char*)uncompressed, ul);

			delete[] body;
			delete[] uncompressed;
			return true;
		}
	}
	return false;
}

bool sr::GravityResourceFile::SaveFile(const std::string& s, const std::string& filename) 
{
	if( !opened )
		return false;

	std::ofstream out(filename, std::ios_base::out | std::ios_base::binary);
    
    if( out.is_open() )
    {
	    bool r = WriteFile(s, out);
	    out.close();

        if(!r)
        {
            std::cerr << "Could not write GRF " << s << " in file " << filename << std::endl;
        }
	    return r;
    }
    else
    {
        std::cerr << "Could not save GRF " << s << " in file " << filename << std::endl;
        return false;
    }
}

bool sr::GravityResourceFile::SaveFile(const std::string& s, const std::wstring& filename) 
{
	if (!opened)
		return(false);

	std::size_t len = MB_CUR_MAX*filename.length();

	char* mbFilename = new char[len + 1];
	len = wcstombs(mbFilename, filename.c_str(), len);
	if (len == -1) 
    {
        // invalid multibyte character
		delete[] mbFilename;
		return false;
	}

	mbFilename[len] = '\0';

	std::ofstream out(mbFilename, std::ios_base::out | std::ios_base::binary);
	delete[] mbFilename;
	bool r = WriteFile(s, out);
	out.close();
	return r;
}