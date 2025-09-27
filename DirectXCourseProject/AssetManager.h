#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

#include "Asset.h"

class AssetManager
{
public:
	static AssetManager& Get();


	template<typename T>
	std::shared_ptr<T> Load(const std::string& filename);

	template<typename T>
	std::shared_ptr<T> Get(const std::string& filename);

	void UnloadUnusedAssets();
	void UnloadAllAssets();

	void AddSearchPath(const std::string& path);
	std::string ResolvePath(const std::string& relativePath) const;
	private:
		AssetManager();
		~AssetManager();

		std::unordered_map<std::string, std::weak_ptr<Asset>> m_assetCache;
		std::vector<std::string> mSearchPaths;
		std::mutex mMutex;

};

