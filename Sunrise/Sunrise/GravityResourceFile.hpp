#pragma once

#include <string>
#include <fstream>

namespace sr
{
    class GravityResourceFile 
    {
    public:
#pragma pack(push,1)
	    /** Header common to all GRF files */
	    struct Header 
        {
		    char signature[16];
		    unsigned char allowEncryption[14];
		    unsigned int fileTableOffset;
		    unsigned int number1;
		    unsigned int number2;
		    unsigned int version;
	    };

	    /** Holds a sequence of compressed (and uncompressed) FileTableItem structures */
	    struct FileTableHeader
        {
		    unsigned int compressedLength;
		    unsigned int uncompressedLength;
		    unsigned char* body;
		    unsigned char* uncompressedBody;

            FileTableHeader()
                : compressedLength(0), uncompressedLength(0), body(nullptr), uncompressedBody(nullptr)
            {
            }
	    };
#pragma pack(pop)

        /** Presents information on each file inside of the GRF */
	    class FileTableItem 
        {
	    public:
		    FileTableItem();
		    FileTableItem(const FileTableItem&);
		    ~FileTableItem();
		    virtual bool readStream(std::istream& ss) = 0;
		    FileTableItem& operator=(const FileTableItem&);
            
		    std::string filename;
		    int compressedLength;
		    int compressedLengthAligned;
		    int uncompressedLength;
		    /**
		     * Bitmask indicating what this file is
		     * <pre>
		     * 0x01 -> File
		     * 0x02 -> MIXCRYPT
		     * 0x03 -> DES
		     * 0x05 -> ??
		     * </pre>
		     *
		     * Source: OpenKore project
		     */
		    char flags;
		    int offset;
		    int cycle; // for DES Decoding purposes
	    };

	    class FileTableItem_Ver1 : public FileTableItem 
        {
	    public:
		    virtual bool readStream(std::istream& ss);
	    };

	    class FileTableItem_Ver2 : public FileTableItem 
        {
	    public:
		    virtual bool readStream(std::istream& ss);
	    };

    public:
	    GravityResourceFile();
	    ~GravityResourceFile();

	    bool Open(const std::string&);
	    void Close();

        bool HasFile(const std::string&) const;

        std::size_t FileCount() const;
        std::string GetFileName(const std::size_t) const;
        const FileTableItem& GetItem(const std::size_t) const;
                
	    /* Reads a file from the GRF and writes into the stream. Returns false if error. */
	    bool WriteFile(const std::string&, std::ostream&);

	    /** Saves the file resourceFileName into the file targetFileName */
	    bool SaveFile(const std::string& resourceFileName, const std::string& targetFileName);
	    bool SaveFile(const std::string& f, const std::wstring& filename);

    private:
	    Header header;
	    bool opened;
	    int fileCount;
	    std::ifstream stream;
	    FileTableHeader fileTableHeader;

        FileTableItem* items;
    };
}
