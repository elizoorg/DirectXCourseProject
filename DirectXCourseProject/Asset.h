#pragma once
#include <string>
class Asset
{
public:
	Asset(const std::string& path) : m_path(path) {};

	virtual ~Asset() = default;

	const std::string& GetPath() const { return m_path; }

	virtual void Load() = 0;
	virtual void Unload() = 0;
	virtual bool IsLoaded() const = 0;

private:
	std::string m_path;

};

