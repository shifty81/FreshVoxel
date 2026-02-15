#include "renderer/Material.h"

#include "renderer/TextureManager.h"

namespace fresh
{

Material::Material(const std::string& name) : name(name) {}

void Material::setAlbedoTexture(const std::string& path)
{
    albedoMap = TextureManager::getInstance().loadTexture(path);
}

void Material::setNormalMap(const std::string& path)
{
    normalMap = TextureManager::getInstance().loadTexture(path);
}

void Material::setMetallicRoughnessMap(const std::string& path)
{
    metallicRoughnessMap = TextureManager::getInstance().loadTexture(path);
}

void Material::setAOMap(const std::string& path)
{
    aoMap = TextureManager::getInstance().loadTexture(path);
}

void Material::setEmissiveMap(const std::string& path)
{
    emissiveMap = TextureManager::getInstance().loadTexture(path);
}

void Material::setCellShading(bool enabled, int levels)
{
    enableCellShading = enabled;
    cellShadingLevels = levels;
}

bool Material::hasTextures() const
{
    return albedoMap || normalMap || metallicRoughnessMap || aoMap || emissiveMap;
}

} // namespace fresh
