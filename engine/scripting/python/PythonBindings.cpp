/**
 * @file PythonBindings.cpp
 * @brief Python bindings implementation using pybind11
 */

#include "scripting/python/PythonBindings.h"
#include <iostream>

// Check if Python development headers are available
#ifdef FRESH_PYTHON_AVAILABLE
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace fresh {
namespace python {

static bool s_initialized = false;
static std::string s_lastError;
static py::scoped_interpreter* s_guard = nullptr;

bool PythonBindings::initialize() {
    if (s_initialized) {
        return true;
    }

    try {
        // Initialize Python interpreter
        s_guard = new py::scoped_interpreter();
        s_initialized = true;
        
        std::cout << "[PythonBindings] Initialized with pybind11" << std::endl;
        return true;
    } catch (const std::exception& e) {
        s_lastError = std::string("Failed to initialize Python: ") + e.what();
        std::cerr << "[PythonBindings] " << s_lastError << std::endl;
        return false;
    }
}

void PythonBindings::shutdown() {
    if (!s_initialized) {
        return;
    }

    delete s_guard;
    s_guard = nullptr;
    s_initialized = false;
    
    std::cout << "[PythonBindings] Shutdown complete" << std::endl;
}

bool PythonBindings::isInitialized() {
    return s_initialized;
}

bool PythonBindings::executeScript(const std::string& filepath) {
    if (!s_initialized) {
        s_lastError = "Python not initialized";
        return false;
    }

    try {
        py::eval_file(filepath);
        return true;
    } catch (const py::error_already_set& e) {
        s_lastError = std::string("Failed to execute script: ") + e.what();
        std::cerr << "[PythonBindings] " << s_lastError << std::endl;
        return false;
    }
}

bool PythonBindings::executeCode(const std::string& code) {
    if (!s_initialized) {
        s_lastError = "Python not initialized";
        return false;
    }

    try {
        py::exec(code);
        return true;
    } catch (const py::error_already_set& e) {
        s_lastError = std::string("Failed to execute code: ") + e.what();
        std::cerr << "[PythonBindings] " << s_lastError << std::endl;
        return false;
    }
}

std::string PythonBindings::getLastError() {
    return s_lastError;
}

// AssetProcessing implementation
bool AssetProcessing::loadTexture(const std::string& filepath) {
    std::cout << "[AssetProcessing] Loading texture: " << filepath << std::endl;
    // Actual implementation would load texture
    return true;
}

std::vector<std::string> AssetProcessing::getSupportedTextureFormats() {
    return {"png", "jpg", "tga", "bmp", "dds"};
}

int AssetProcessing::processDirectory(const std::string& directory, const std::string& filePattern) {
    std::cout << "[AssetProcessing] Processing directory: " << directory 
              << " with pattern: " << filePattern << std::endl;
    // Actual implementation would process files
    return 0;
}

// WorldDataAccess implementation
bool WorldDataAccess::loadWorld(const std::string& worldPath) {
    std::cout << "[WorldDataAccess] Loading world: " << worldPath << std::endl;
    // Actual implementation would load world
    return true;
}

std::string WorldDataAccess::getWorldInfo(const std::string& worldPath) {
    return "World: " + worldPath + ", Chunks: 0, Size: 0 MB";
}

int WorldDataAccess::getChunkCount() {
    return 0;
}

// BuildAutomation implementation
bool BuildAutomation::validateAssets(const std::string& directory) {
    std::cout << "[BuildAutomation] Validating assets in: " << directory << std::endl;
    // Actual implementation would validate
    return true;
}

std::string BuildAutomation::generateManifest(const std::string& directory) {
    return "{}"; // JSON manifest
}

} // namespace python
} // namespace fresh

// pybind11 module definition
PYBIND11_MODULE(fresh_engine, m) {
    m.doc() = "Fresh Engine Python Bindings";

    // AssetProcessing bindings
    py::class_<fresh::python::AssetProcessing>(m, "AssetProcessing")
        .def_static("load_texture", &fresh::python::AssetProcessing::loadTexture)
        .def_static("get_supported_texture_formats", 
                   &fresh::python::AssetProcessing::getSupportedTextureFormats)
        .def_static("process_directory", 
                   &fresh::python::AssetProcessing::processDirectory);

    // WorldDataAccess bindings
    py::class_<fresh::python::WorldDataAccess>(m, "WorldDataAccess")
        .def_static("load_world", &fresh::python::WorldDataAccess::loadWorld)
        .def_static("get_world_info", &fresh::python::WorldDataAccess::getWorldInfo)
        .def_static("get_chunk_count", &fresh::python::WorldDataAccess::getChunkCount);

    // BuildAutomation bindings
    py::class_<fresh::python::BuildAutomation>(m, "BuildAutomation")
        .def_static("validate_assets", &fresh::python::BuildAutomation::validateAssets)
        .def_static("generate_manifest", &fresh::python::BuildAutomation::generateManifest);
}

#else // FRESH_PYTHON_AVAILABLE not defined

// Stub implementation when Python is not available
#include "scripting/python/PythonBindings.h"

namespace fresh {
namespace python {

static std::string s_lastError = "Python support not compiled in";

bool PythonBindings::initialize() {
    std::cout << "[PythonBindings] Python support not compiled in (stub implementation)" << std::endl;
    return false;
}

void PythonBindings::shutdown() {}
bool PythonBindings::isInitialized() { return false; }
bool PythonBindings::executeScript(const std::string&) { return false; }
bool PythonBindings::executeCode(const std::string&) { return false; }
std::string PythonBindings::getLastError() { return s_lastError; }

// Stub implementations
bool AssetProcessing::loadTexture(const std::string&) { return false; }
std::vector<std::string> AssetProcessing::getSupportedTextureFormats() { return {}; }
int AssetProcessing::processDirectory(const std::string&, const std::string&) { return 0; }

bool WorldDataAccess::loadWorld(const std::string&) { return false; }
std::string WorldDataAccess::getWorldInfo(const std::string&) { return ""; }
int WorldDataAccess::getChunkCount() { return 0; }

bool BuildAutomation::validateAssets(const std::string&) { return false; }
std::string BuildAutomation::generateManifest(const std::string&) { return ""; }

} // namespace python
} // namespace fresh

#endif // FRESH_PYTHON_AVAILABLE
