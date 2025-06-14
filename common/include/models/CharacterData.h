#pragma once

#include <string>
#include <vector>
#include <map>

namespace Common {

struct CharacterData {
    std::string id;
    std::string name;
    std::string characterClass;
    int level;
    int experience;
    int health;
    int maxHealth;
    int mana;
    int maxMana;
    int attack;
    int defense;
    int speed;
    float criticalRate;
    float criticalDamage;
    
    // 序列化/反序列化方法
    std::string toJson() const;
    static CharacterData fromJson(const std::string& json);
};

} // namespace Common
