/**
 * @file ScriptResource.cpp
 * @brief 脚本资源类实现
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#include "utils/resources/ScriptResource.h"
#include "utils/Logger.h"
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <fstream>
#include <sstream>

namespace Fantasy {

namespace py = pybind11;

// 初始化Python解释器
namespace {
    struct PythonInterpreter {
        PythonInterpreter() {
            try {
                py::initialize_interpreter();
            } catch (const std::exception& e) {
                FANTASY_LOG_ERROR("Failed to initialize Python interpreter: {}", e.what());
            }
        }
        ~PythonInterpreter() {
            try {
                py::finalize_interpreter();
            } catch (const std::exception& e) {
                FANTASY_LOG_ERROR("Failed to finalize Python interpreter: {}", e.what());
            }
        }
    };
    static PythonInterpreter interpreter;
}

ScriptResource::ScriptResource(const std::filesystem::path& path)
    : path_(path)
    , state_(ResourceState::UNLOADED)
    , size_(0)
    , scriptType_(ScriptType::CUSTOM) {
    id_ = path.filename().string();
    try {
        lastModified_ = std::filesystem::last_write_time(path);
        size_ = std::filesystem::file_size(path);
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get file info for script {}: {}", path.string(), e.what());
    }
}

const std::string& ScriptResource::getId() const {
    return id_;
}

ResourceType ScriptResource::getType() const {
    return ResourceType::SCRIPT;
}

ResourceState ScriptResource::getState() const {
    return state_;
}

const std::filesystem::path& ScriptResource::getPath() const {
    return path_;
}

std::filesystem::file_time_type ScriptResource::getLastModifiedTime() const {
    return lastModified_;
}

std::uintmax_t ScriptResource::getSize() const {
    return size_;
}

bool ScriptResource::load() {
    if (state_ == ResourceState::LOADED) {
        return true;
    }

    try {
        state_ = ResourceState::LOADING;

        // 读取脚本文件内容
        std::ifstream file(path_);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open script file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string scriptContent = buffer.str();

        // 根据文件扩展名或内容判断脚本类型
        if (path_.extension() == ".quest.py") {
            scriptType_ = ScriptType::QUEST;
        } else if (path_.extension() == ".dialogue.py") {
            scriptType_ = ScriptType::DIALOGUE;
        } else if (path_.extension() == ".event.py") {
            scriptType_ = ScriptType::EVENT;
        } else if (path_.extension() == ".ai.py") {
            scriptType_ = ScriptType::AI;
        } else if (path_.extension() == ".system.py") {
            scriptType_ = ScriptType::SYSTEM;
        }

        // 创建Python模块
        py::module_ sys = py::module_::import("sys");
        if (!pythonPath_.empty()) {
            sys.attr("path").attr("append")(pythonPath_.string());
        }

        // 编译并执行脚本
        py::module_ module = py::module_::import("importlib.util");
        py::object spec = module.attr("spec_from_file_location")(id_, path_.string());
        py::object pyModule = module.attr("module_from_spec")(spec);
        sys.attr("modules")[id_] = pyModule;
        spec.attr("loader").attr("exec_module")(pyModule);

        module_ = pyModule;
        state_ = ResourceState::LOADED;
        error_.clear();

        FANTASY_LOG_INFO("Successfully loaded script: {}", path_.string());
        return true;
    } catch (const std::exception& e) {
        state_ = ResourceState::ERROR;
        error_ = e.what();
        FANTASY_LOG_ERROR("Failed to load script {}: {}", path_.string(), e.what());
        return false;
    }
}

void ScriptResource::unload() {
    if (state_ != ResourceState::LOADED) {
        return;
    }

    try {
        // 从sys.modules中移除模块
        py::module_::import("sys").attr("modules").attr("pop")(id_);
        module_ = py::module_();
        state_ = ResourceState::UNLOADED;
        error_.clear();

        FANTASY_LOG_INFO("Successfully unloaded script: {}", path_.string());
    } catch (const std::exception& e) {
        state_ = ResourceState::ERROR;
        error_ = e.what();
        FANTASY_LOG_ERROR("Failed to unload script {}: {}", path_.string(), e.what());
    }
}

bool ScriptResource::reload() {
    unload();
    return load();
}

bool ScriptResource::needsReload() const {
    try {
        return std::filesystem::last_write_time(path_) > lastModified_;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to check if script needs reload: {}", e.what());
        return false;
    }
}

ScriptResource::ScriptType ScriptResource::getScriptType() const {
    return scriptType_;
}

py::module_ ScriptResource::getModule() const {
    return module_;
}

bool ScriptResource::hasFunction(const std::string& functionName) const {
    if (!module_) {
        return false;
    }
    try {
        return py::hasattr(module_, functionName.c_str()) &&
               py::isinstance<py::function>(module_.attr(functionName.c_str()));
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to check function existence: {}", e.what());
        return false;
    }
}

bool ScriptResource::hasVariable(const std::string& name) const {
    if (!module_) {
        return false;
    }
    try {
        return py::hasattr(module_, name.c_str());
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to check variable existence: {}", e.what());
        return false;
    }
}

const std::string& ScriptResource::getError() const {
    return error_;
}

// ScriptResourceLoader实现
ScriptResourceLoader::ScriptResourceLoader(const std::filesystem::path& pythonPath)
    : pythonPath_(pythonPath) {
}

bool ScriptResourceLoader::supportsType(ResourceType type) const {
    return type == ResourceType::SCRIPT;
}

std::shared_ptr<IResource> ScriptResourceLoader::load(
    const std::filesystem::path& path,
    ResourceType type) {
    if (!supportsType(type)) {
        return nullptr;
    }

    auto resource = std::make_shared<ScriptResource>(path);
    if (!resource->load()) {
        return nullptr;
    }
    return resource;
}

void ScriptResourceLoader::loadAsync(
    const std::filesystem::path& path,
    ResourceType type,
    std::function<void(std::shared_ptr<IResource>)> callback) {
    if (!supportsType(type)) {
        callback(nullptr);
        return;
    }

    // 在后台线程中加载资源
    std::thread([path, callback]() {
        auto resource = std::make_shared<ScriptResource>(path);
        if (resource->load()) {
            callback(resource);
        } else {
            callback(nullptr);
        }
    }).detach();
}

} // namespace Fantasy
