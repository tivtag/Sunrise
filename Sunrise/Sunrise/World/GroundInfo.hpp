#pragma once
#include <iostream>

union ObjectVersion
{
	short sver;
	struct _cver {
		unsigned char major;
		unsigned char minor;
	} cver;
};


/** Array.
 * The data type must support 'a = b'.
 */
template <typename T>
class Arr {
protected:
	unsigned int _size;
	T* _data;
public:
	typedef T type;

	Arr() {
		_size = 0;
		_data = 0;
	}

	~Arr() {
		clear();
	}

	void resize(unsigned int n) {
		T* newData = 0;
		if (n > 0) {
			newData = new T[n];
			if (_size > 0) {
				unsigned int i, end = (_size < n? _size: n);
				for (i = 0; i < end; i++) {
					newData[i] = _data[i];
				}
				delete[] _data;
			}
		}
		_size = n;
		_data = newData;
	}

	void resize(unsigned int n, const T& value) {
		T* newData = 0;
		if (n > 0) {
			unsigned int i = 0;
			newData = new T[n];
			if (_size > 0) {
				unsigned int end = (_size < n? _size: n);
				for (; i < end; i++) {
					newData[i] = _data[i];
				}
				delete[] _data;
			}
			for (; i < n; i++) {
				newData[i] = value;
			}
		}
		_size = n;
		_data = newData;
	}

	unsigned int size() const {
		return(_size);
	}

	void operator = (const Arr<T>& a) {
		clear();
		_size = a._size;
		if (_size > 0) {
			_data = new T[_size];
			for (unsigned int i = 0; i < _size; i++) {
				_data[i] = a._data[i];
			}
		}
	}

	type& operator[] (unsigned int i) {
		return(_data[i]);
	}

	const type& operator[] (unsigned int i) const {
		return(_data[i]);
	}

	void clear() {
		if (_size > 0) {
			delete[] _data;
			_data = 0;
			_size = 0;
		}
	}
};

class GroundInfo
{
public:
#pragma pack(push,1)
	/// Lightmap, represents the shadow projected on a surface.
	struct Lightmap {
		unsigned char brightness[8][8];

		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			inline operator unsigned char* () { return &r; }
			inline operator const unsigned char* () const { return &r; }
		} 
		color[8][8];
	};

	/// Surface, represents the visual aspect of a quad.
	/// The vertices to use depend on which side of the cell is using the surface.
	/// top = T0->T1->T2->T3
	/// front = T2->T3->F0->F1
	/// right = T3->T1->R2->R0
	/// 2---3
	/// | F | (cell y+1)
	/// 0---1
	/// 2---3 2---3
	/// | T | | R | (cell x+1)
	/// 0---1 0---1
	struct Surface {
		float u[4]; //< west->east, south->north ordering; 0=left 1=right
		float v[4]; //< west->east, south->north ordering; 0=up 1=down
		short textureId; //< -1 for none
//		unsigned short lightmapId; //< -1 for none?
		short lightmapId; //< -1 for none?

		struct {
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
			inline operator unsigned char* () { return &b; }
			inline operator const unsigned char* () const { return &b; }
		}
		color;//< BGRA -- "A" seems to be ignored by the official client
	};

	struct Cell {
		float height[4]; //< west->east, south->north ordering
		int topSurfaceId; //< -1 for none
		int frontSurfaceId; //< -1 for none
		int rightSurfaceId; //< -1 for none
	};
#pragma pack(pop)
	
public:
	GroundInfo();
	bool ReadStream(std::istream&);

	~GroundInfo()
	{
		Reset();
	}

	unsigned int Width() const { return width; }
	unsigned int Height() const { return width; }

	const Cell& GetCell(unsigned int cellx, unsigned int celly) const;
	const Surface& GetSurface(unsigned int idx) const;

    std::size_t TextureCount() const { return textureCount; }    
    const char* GetTexture(const std::size_t index) const
    {
        if(index >= 0 && index < textureCount)
		    return &textures[index * textureSize];
        else
	        return "";
    }

private:
	void Reset();
	bool ReadHeader(std::istream&);
	bool CheckHeader(const std::string&) const;

private:
	unsigned int width;
	unsigned int height;
	float zoom;

	unsigned int textureCount;
	unsigned int textureSize;
	char* textures;

	Arr<Lightmap> lightmaps;
	Arr<Surface> surfaces;
	Arr<Cell> cells; //< west->east, south->north ordering

	unsigned char magicSize;
	char magic[4];
	ObjectVersion version;
	bool valid;
};
