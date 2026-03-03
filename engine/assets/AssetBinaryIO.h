#pragma once
#include "graphvm/GraphVM.h"
#include "assets/AssetFormat.h"
#include <string>

namespace fresh::asset {

class AssetBinaryIO {
public:
    static bool writeGraph(const std::string& path, const vm::Bytecode& prog);
    static bool readGraph(const std::string& path, vm::Bytecode& prog);
};

} // namespace fresh::asset
