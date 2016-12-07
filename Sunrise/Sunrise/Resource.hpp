#pragma once

#include <string>
#include <boost\noncopyable.hpp>

namespace sr
{
	class Resource : public boost::noncopyable
	{
	public:
        bool Loaded() const { return loaded; }
        void Loaded(const bool state) { loaded = state; }
        
        const std::string& Name() const { return name; }
		void Name(const std::string& name) { this->name = name; }
        
		virtual ~Resource();
		virtual void Unload() = 0;
	protected:
		Resource() : name(""), loaded(false) { }
		Resource(const std::string& _name) : name(_name), loaded(false) { }

	private:
		std::string name;
        bool loaded;
	};
}
