import json
import os
import random
from typing import Dict, List, Any

class ConfigGenerator:
    def __init__(self):
        self.config_path = "../resources/config/"
        
    def generate_character_config(self, character_type: str, level: int) -> Dict[str, Any]:
        """生成角色配置"""
        base_stats = {
            "warrior": {"health": 120, "mana": 30, "attack": 20, "defense": 15},
            "mage": {"health": 80, "mana": 100, "attack": 10, "defense": 8},
            "assassin": {"health": 90, "mana": 40, "attack": 25, "defense": 10}
        }
        
        stats = base_stats.get(character_type, base_stats["warrior"]).copy()
        
        # 根据等级调整属性
        for stat, value in stats.items():
            stats[stat] = int(value * (1 + (level - 1) * 0.1))
        
        return {
            "type": character_type,
            "level": level,
            "stats": stats,
            "skills": self._generate_skills(character_type, level)
        }
    
    def _generate_skills(self, character_type: str, level: int) -> List[str]:
        """生成技能列表"""
        skill_templates = {
            "warrior": ["basic_attack", "heavy_strike", "defense_stance"],
            "mage": ["basic_attack", "fireball", "ice_shield"],
            "assassin": ["basic_attack", "stealth", "backstab"]
        }
        
        skills = skill_templates.get(character_type, [])
        available_skills = skills[:min(level, len(skills))]
        
        return available_skills
    
    def generate_level_config(self, level_id: int, difficulty: float = 1.0) -> Dict[str, Any]:
        """生成关卡配置"""
        enemy_types = ["goblin", "wolf", "bandit", "skeleton"]
        enemy_count = int(3 + level_id * 0.5)
        
        enemies = []
        for i in range(enemy_count):
            enemy_type = random.choice(enemy_types)
            enemy_level = max(1, level_id - 1 + random.randint(-1, 1))
            
            enemies.append({
                "type": enemy_type,
                "level": enemy_level,
                "position": [random.randint(100, 700), random.randint(100, 500)]
            })
        
        return {
            "level_id": level_id,
            "name": f"Level {level_id}",
            "difficulty": difficulty,
            "enemies": enemies,
            "rewards": {
                "experience": 50 + level_id * 20,
                "gold": 20 + level_id * 10,
                "items": self._generate_rewards(level_id)
            },
            "objectives": [
                {
                    "type": "defeat_all_enemies",
                    "description": "击败所有敌人"
                }
            ]
        }
    
    def _generate_rewards(self, level_id: int) -> List[str]:
        """生成奖励物品"""
        common_items = ["health_potion", "mana_potion"]
        rare_items = ["iron_sword", "leather_armor", "magic_ring"]
        
        rewards = []
        if random.random() < 0.7:  # 70%概率获得普通物品
            rewards.append(random.choice(common_items))
        
        if random.random() < 0.3:  # 30%概率获得稀有物品
            rewards.append(random.choice(rare_items))
        
        return rewards
    
    def save_config(self, config: Dict[str, Any], filename: str):
        """保存配置到文件"""
        file_path = os.path.join(self.config_path, filename)
        os.makedirs(os.path.dirname(file_path), exist_ok=True)
        
        with open(file_path, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=2)
    
    def generate_all_configs(self):
        """生成所有配置文件"""
        # 生成角色配置
        for character_type in ["warrior", "mage", "assassin"]:
            for level in range(1, 21):
                config = self.generate_character_config(character_type, level)
                filename = f"characters/{character_type}_level_{level}.json"
                self.save_config(config, filename)
        
        # 生成关卡配置
        for level_id in range(1, 31):
            config = self.generate_level_config(level_id)
            filename = f"levels/level_{level_id}.json"
            self.save_config(config, filename)

if __name__ == "__main__":
    generator = ConfigGenerator()
    generator.generate_all_configs()
    print("所有配置文件生成完成！")