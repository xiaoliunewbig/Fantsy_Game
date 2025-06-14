#include "models/CharacterData.h"
#include <iostream>

namespace Common {

std::string CharacterData::toJson() const {
    // 简单实现，实际项目中应使用JSON库
    return "{\"id\":\"" + id + "\",\"name\":\"" + name + "\"}";
}

CharacterData CharacterData::fromJson(const std::string& json) {
    // 简单实现，实际项目中应使用JSON库
    CharacterData data;
    // 解析json字符串
    return data;
}

} // namespace Common
