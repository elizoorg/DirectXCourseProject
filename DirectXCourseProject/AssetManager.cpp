#include "AssetManager.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;


AssetManager& AssetManager::Get()
{
	static AssetManager instance;
	return instance;
}

void AssetManager::UnloadUnusedAssets()
{
	std::lock_guard<std::mutex> lock(mMutex);

	for (auto it = m_assetCache.begin(); it != m_assetCache.end();)
	{
		if (it->second.expired())
		{
			it= m_assetCache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void AssetManager::UnloadAllAssets()
{
	std::lock_guard<std::mutex> lock(mMutex);
	m_assetCache.clear();
}

void AssetManager::AddSearchPath(const std::string& path)
{
	if (std::find(mSearchPaths.begin(), mSearchPaths.end(), path) == mSearchPaths.end())
	{
		mSearchPaths.push_back(path);
	}
}

std::string AssetManager::ResolvePath(const std::string& relativePath) const
{
	for (const auto& path : mSearchPaths)
	{
		
		fs::path fullPath = fs::path(path) / relativePath;
		if (fs::exists(fullPath))
		{
			return fullPath.string();
		}
	}
	return "";
}

AssetManager::AssetManager()
{
	AddSearchPath("Assets");
	AddSearchPath("Content");
}

AssetManager::~AssetManager()
{
	UnloadAllAssets();
}

template<typename T>
inline std::shared_ptr<T> AssetManager::Load(const std::string& filename)
{
	std::string path = ResolvePath(filename);
	if(path.empty())
		return nullptr;

	std::lock_guard<std::mutex> lock(mMutex);

	auto cached = m_assetCache.find(path);
	if (cached != m_assetCache.end())
		if(auto asset = cached->second.lock())
		return std::dynamic_pointer_cast<T>(cached->second);


	std::shared_ptr<T> asset = LoadAsset<T>(path);
	if (asset && asset->Load()) {
		m_assetCache[path] = asset;
		return asset;
	}

	return nullptr;

}

template<typename T>
inline std::shared_ptr<T> AssetManager::Get(const std::string& filename)
{
	std::string path = ResolvePath(filename);
	if (path.empty()) {
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(mMutex);

	auto cached = m_assetCache.find(path);
	if (cached != m_assetCache.end())
		if (auto asset = cached->second.lock())
		return std::dynamic_pointer_cast<T>(cached->second);


	return nullptr;
}

