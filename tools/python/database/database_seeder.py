#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
《幻境传说》数据库数据生成器
用于生成测试数据和示例数据
"""

import sqlite3
import json
import random
import uuid
from datetime import datetime, timedelta
from typing import Dict, List, Any, Optional

class DatabaseSeeder:
    def __init__(self, database_path: str):
        self.database_path = database_path
        self.conn = None
        self.cursor = None
        
    def connect(self):
        """连接到数据库"""
        try:
            self.conn = sqlite3.connect(self.database_path)
            self.cursor = self.conn.cursor()
            print(f"成功连接到数据库: {self.database_path}")
            return True
        except Exception as e:
            print(f"连接数据库失败: {e}")
            return False
    
    def disconnect(self):
        """断开数据库连接"""
        if self.conn:
            self.conn.close()
            print("数据库连接已关闭")
    
    def execute_script(self, script_path: str):
        """执行SQL脚本"""
        try:
            with open(script_path, 'r', encoding='utf-8') as f:
                script = f.read()
            self.cursor.executescript(script)
            self.conn.commit()
            print(f"成功执行脚本: {script_path}")
            return True
        except Exception as e:
            print(f"执行脚本失败: {e}")
            return False
    
    def generate_sample_characters(self, count: int = 10):
        """生成示例角色数据"""
        print(f"生成 {count} 个示例角色...")
        
        classes = ['warrior', 'mage', 'assassin']
        names = [
            '阿尔文', '贝拉', '卡洛斯', '黛安娜', '艾瑞克',
            '菲奥娜', '加布里埃尔', '海伦娜', '伊万', '朱莉娅',
            '凯文', '露娜', '马库斯', '娜塔莎', '奥利弗',
            '佩妮', '昆汀', '罗莎', '塞巴斯蒂安', '特蕾莎'
        ]
        
        for i in range(count):
            character_id = f"char_{uuid.uuid4().hex[:8]}"
            name = random.choice(names)
            class_type = random.choice(classes)
            
            # 根据职业设置基础属性
            if class_type == 'warrior':
                health = random.randint(120, 150)
                mana = random.randint(30, 50)
                attack = random.randint(20, 25)
                defense = random.randint(15, 20)
            elif class_type == 'mage':
                health = random.randint(80, 100)
                mana = random.randint(80, 120)
                attack = random.randint(10, 15)
                defense = random.randint(8, 12)
            else:  # assassin
                health = random.randint(90, 110)
                mana = random.randint(40, 60)
                attack = random.randint(25, 30)
                defense = random.randint(10, 15)
            
            level = random.randint(1, 20)
            experience = random.randint(0, level * 100)
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO characters 
                (id, name, class, level, experience, health, max_health, mana, max_mana, 
                 attack, defense, speed, critical_rate, critical_damage, is_player)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                character_id, name, class_type, level, experience,
                health, health, mana, mana, attack, defense,
                random.randint(5, 8), random.uniform(0.05, 0.15),
                random.uniform(1.5, 2.0), i == 0  # 第一个角色设为玩家
            ))
            
            # 为角色添加技能
            self._add_skills_to_character(character_id, class_type, level)
            
            # 为角色添加装备
            self._add_equipment_to_character(character_id, class_type, level)
            
            # 为角色添加物品
            self._add_items_to_character(character_id)
            
            # 为角色添加统计数据
            self._add_statistics_to_character(character_id, level)
        
        self.conn.commit()
        print(f"成功生成 {count} 个角色")
    
    def _add_skills_to_character(self, character_id: str, class_type: str, level: int):
        """为角色添加技能"""
        # 获取该职业的技能
        self.cursor.execute("""
            SELECT id FROM skills 
            WHERE class_requirement = ? AND level_requirement <= ?
        """, (class_type, level))
        
        skills = self.cursor.fetchall()
        
        for skill in skills[:min(5, len(skills))]:  # 最多5个技能
            skill_id = skill[0]
            skill_level = random.randint(1, min(level // 2 + 1, 10))
            is_equipped = random.choice([True, False])
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO character_skills 
                (character_id, skill_id, skill_level, is_equipped)
                VALUES (?, ?, ?, ?)
            """, (character_id, skill_id, skill_level, is_equipped))
    
    def _add_equipment_to_character(self, character_id: str, class_type: str, level: int):
        """为角色添加装备"""
        slots = ['weapon', 'chest', 'ring']
        
        for slot in slots:
            # 获取适合的装备
            self.cursor.execute("""
                SELECT id FROM equipment 
                WHERE slot = ? AND level_requirement <= ? 
                AND (class_requirement IS NULL OR class_requirement = ?)
                ORDER BY RANDOM() LIMIT 1
            """, (slot, level, class_type))
            
            result = self.cursor.fetchone()
            if result:
                equipment_id = result[0]
                durability = random.randint(80, 100)
                enchant_level = random.randint(0, 3)
                
                self.cursor.execute("""
                    INSERT OR REPLACE INTO character_equipment 
                    (character_id, equipment_id, slot, is_equipped, durability, enchant_level)
                    VALUES (?, ?, ?, ?, ?, ?)
                """, (character_id, equipment_id, slot, True, durability, enchant_level))
    
    def _add_items_to_character(self, character_id: str):
        """为角色添加物品"""
        # 获取一些随机物品
        self.cursor.execute("""
            SELECT id FROM items 
            WHERE type IN ('consumable', 'material') 
            ORDER BY RANDOM() LIMIT 5
        """)
        
        items = self.cursor.fetchall()
        
        for i, item in enumerate(items):
            item_id = item[0]
            quantity = random.randint(1, 10)
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO character_inventory 
                (character_id, item_id, quantity, slot_index)
                VALUES (?, ?, ?, ?)
            """, (character_id, item_id, quantity, i))
    
    def _add_statistics_to_character(self, character_id: str, level: int):
        """为角色添加统计数据"""
        stats = {
            'total_battles': random.randint(10, 100),
            'battles_won': random.randint(5, 80),
            'total_experience_gained': random.randint(1000, 10000),
            'max_combo': random.randint(1, 15),
            'items_collected': random.randint(20, 200),
            'quests_completed': random.randint(5, 30),
            'levels_completed': random.randint(5, 25)
        }
        
        for stat_key, stat_value in stats.items():
            self.cursor.execute("""
                INSERT OR REPLACE INTO statistics 
                (character_id, stat_key, stat_value, stat_type)
                VALUES (?, ?, ?, ?)
            """, (character_id, stat_key, stat_value, 'integer'))
    
    def generate_sample_quests(self, count: int = 20):
        """生成示例任务数据"""
        print(f"生成 {count} 个示例任务...")
        
        quest_templates = [
            {
                'title': '击败{enemy}',
                'description': '在{location}击败{count}个{enemy}',
                'type': 'main',
                'objectives': [{'type': 'kill', 'target': '{enemy}', 'count': '{count}'}]
            },
            {
                'title': '收集{item}',
                'description': '收集{count}个{item}',
                'type': 'side',
                'objectives': [{'type': 'collect', 'item': '{item}', 'count': '{count}'}]
            },
            {
                'title': '探索{location}',
                'description': '探索{location}区域',
                'type': 'exploration',
                'objectives': [{'type': 'reach', 'location': '{location}'}]
            }
        ]
        
        enemies = ['哥布林', '狼', '强盗', '骷髅', '巨魔', '龙']
        locations = ['森林', '洞穴', '城堡', '村庄', '沙漠', '雪山']
        items = ['铁矿', '魔法水晶', '毒液精华', '火焰精华', '金币', '宝石']
        
        for i in range(count):
            quest_id = f"quest_{uuid.uuid4().hex[:8]}"
            template = random.choice(quest_templates)
            
            # 填充模板
            enemy = random.choice(enemies)
            location = random.choice(locations)
            item = random.choice(items)
            count = random.randint(1, 5)
            
            title = template['title'].format(enemy=enemy, location=location, item=item, count=count)
            description = template['description'].format(enemy=enemy, location=location, item=item, count=count)
            
            chapter = random.randint(1, 6)
            level_requirement = random.randint(1, 20)
            
            objectives = []
            for obj in template['objectives']:
                obj_copy = obj.copy()
                for key, value in obj_copy.items():
                    if isinstance(value, str):
                        obj_copy[key] = value.format(enemy=enemy, location=location, item=item, count=count)
                objectives.append(obj_copy)
            
            rewards = {
                'experience': random.randint(50, 500),
                'gold': random.randint(20, 200),
                'items': random.sample(items, random.randint(0, 2))
            }
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO quests 
                (id, title, description, type, chapter, level_requirement, objectives, rewards)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                quest_id, title, description, template['type'], chapter, level_requirement,
                json.dumps(objectives, ensure_ascii=False),
                json.dumps(rewards, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个任务")
    
    def generate_sample_levels(self, count: int = 15):
        """生成示例关卡数据"""
        print(f"生成 {count} 个示例关卡...")
        
        level_templates = [
            {
                'name': '{location}探索',
                'description': '探索{location}区域，击败遇到的敌人',
                'type': 'main'
            },
            {
                'name': '{enemy}巢穴',
                'description': '深入{enemy}的巢穴，击败首领',
                'type': 'boss'
            },
            {
                'name': '{location}挑战',
                'description': '在{location}中完成挑战任务',
                'type': 'challenge'
            }
        ]
        
        locations = ['森林', '洞穴', '城堡', '村庄', '沙漠', '雪山', '地下城', '神殿']
        enemies = ['哥布林', '狼', '强盗', '骷髅', '巨魔', '龙', '恶魔', '天使']
        
        for i in range(count):
            level_id = f"level_{uuid.uuid4().hex[:8]}"
            template = random.choice(level_templates)
            
            location = random.choice(locations)
            enemy = random.choice(enemies)
            
            name = template['name'].format(location=location, enemy=enemy)
            description = template['description'].format(location=location, enemy=enemy)
            
            chapter = random.randint(1, 6)
            difficulty = random.uniform(1.0, 3.0)
            level_requirement = random.randint(1, 20)
            
            # 生成敌人配置
            enemy_count = random.randint(2, 6)
            enemies_config = []
            for j in range(enemy_count):
                enemies_config.append({
                    'type': random.choice(enemies),
                    'count': random.randint(1, 3),
                    'position': [random.randint(100, 700), random.randint(100, 500)]
                })
            
            # 生成奖励配置
            rewards = {
                'experience': random.randint(100, 1000),
                'gold': random.randint(50, 500),
                'items': random.sample(['iron_sword', 'magic_staff', 'health_potion', 'mana_potion'], random.randint(1, 3))
            }
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO levels 
                (id, name, description, type, chapter, difficulty, level_requirement, enemies, rewards)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                level_id, name, description, template['type'], chapter, difficulty, level_requirement,
                json.dumps(enemies_config, ensure_ascii=False),
                json.dumps(rewards, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个关卡")
    
    def generate_sample_battle_records(self, count: int = 50):
        """生成示例战斗记录"""
        print(f"生成 {count} 个示例战斗记录...")
        
        # 获取所有角色ID
        self.cursor.execute("SELECT id FROM characters")
        character_ids = [row[0] for row in self.cursor.fetchall()]
        
        battle_types = ['level', 'arena', 'boss', 'pvp']
        results = ['victory', 'defeat', 'draw']
        opponent_types = ['goblin', 'wolf', 'bandit', 'skeleton', 'boss', 'player']
        
        for i in range(count):
            character_id = random.choice(character_ids)
            battle_type = random.choice(battle_types)
            result = random.choice(results)
            opponent_type = random.choice(opponent_types)
            
            duration = random.randint(30, 600)  # 30秒到10分钟
            damage_dealt = random.randint(100, 2000)
            damage_taken = random.randint(50, 1500)
            combo_max = random.randint(1, 20)
            experience_gained = random.randint(10, 200)
            
            skills_used = random.sample(['basic_attack', 'heavy_strike', 'fireball', 'stealth'], random.randint(1, 3))
            items_dropped = random.sample(['health_potion', 'mana_potion', 'iron_ore'], random.randint(0, 2))
            
            # 随机选择关卡（可能为空）
            level_id = None
            if battle_type == 'level':
                self.cursor.execute("SELECT id FROM levels ORDER BY RANDOM() LIMIT 1")
                result_level = self.cursor.fetchone()
                if result_level:
                    level_id = result_level[0]
            
            self.cursor.execute("""
                INSERT INTO battle_records 
                (character_id, level_id, battle_type, opponent_type, result, duration,
                 damage_dealt, damage_taken, skills_used, combo_max, experience_gained, items_dropped)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                character_id, level_id, battle_type, opponent_type, result, duration,
                damage_dealt, damage_taken, json.dumps(skills_used, ensure_ascii=False),
                combo_max, experience_gained, json.dumps(items_dropped, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个战斗记录")
    
    def generate_sample_quest_progress(self, count: int = 30):
        """生成示例任务进度"""
        print(f"生成 {count} 个示例任务进度...")
        
        # 获取角色和任务
        self.cursor.execute("SELECT id FROM characters")
        character_ids = [row[0] for row in self.cursor.fetchall()]
        
        self.cursor.execute("SELECT id FROM quests")
        quest_ids = [row[0] for row in self.cursor.fetchall()]
        
        statuses = ['not_started', 'in_progress', 'completed']
        
        for i in range(count):
            character_id = random.choice(character_ids)
            quest_id = random.choice(quest_ids)
            status = random.choice(statuses)
            
            start_time = None
            complete_time = None
            progress = {}
            
            if status == 'in_progress':
                start_time = datetime.now() - timedelta(days=random.randint(1, 7))
                progress = {'completed_objectives': random.randint(0, 2)}
            elif status == 'completed':
                start_time = datetime.now() - timedelta(days=random.randint(1, 7))
                complete_time = start_time + timedelta(hours=random.randint(1, 24))
                progress = {'completed_objectives': 3, 'all_completed': True}
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO character_quests 
                (character_id, quest_id, status, progress, start_time, complete_time)
                VALUES (?, ?, ?, ?, ?, ?)
            """, (
                character_id, quest_id, status,
                json.dumps(progress, ensure_ascii=False),
                start_time, complete_time
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个任务进度")
    
    def generate_sample_level_progress(self, count: int = 25):
        """生成示例关卡进度"""
        print(f"生成 {count} 个示例关卡进度...")
        
        # 获取角色和关卡
        self.cursor.execute("SELECT id FROM characters")
        character_ids = [row[0] for row in self.cursor.fetchall()]
        
        self.cursor.execute("SELECT id FROM levels")
        level_ids = [row[0] for row in self.cursor.fetchall()]
        
        statuses = ['locked', 'unlocked', 'completed', 'perfect']
        
        for i in range(count):
            character_id = random.choice(character_ids)
            level_id = random.choice(level_ids)
            status = random.choice(statuses)
            
            completion_time = 0
            score = 0
            stars = 0
            attempts = 0
            best_time = 0
            
            if status in ['completed', 'perfect']:
                completion_time = random.randint(60, 600)  # 1-10分钟
                score = random.randint(1000, 10000)
                stars = random.randint(1, 3)
                attempts = random.randint(1, 5)
                best_time = completion_time
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO character_level_progress 
                (character_id, level_id, status, completion_time, score, stars, attempts, best_time)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                character_id, level_id, status, completion_time, score, stars, attempts, best_time
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个关卡进度")
    
    def generate_sample_achievements(self, count: int = 20):
        """生成示例成就解锁记录"""
        print(f"生成 {count} 个示例成就解锁记录...")
        
        # 获取角色和成就
        self.cursor.execute("SELECT id FROM characters")
        character_ids = [row[0] for row in self.cursor.fetchall()]
        
        self.cursor.execute("SELECT id FROM achievements")
        achievement_ids = [row[0] for row in self.cursor.fetchall()]
        
        for i in range(count):
            character_id = random.choice(character_ids)
            achievement_id = random.choice(achievement_ids)
            unlocked = random.choice([True, False])
            
            unlock_date = None
            if unlocked:
                unlock_date = datetime.now() - timedelta(days=random.randint(1, 30))
            
            progress = {'progress': random.randint(0, 100)}
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO character_achievements 
                (character_id, achievement_id, unlocked, unlock_date, progress)
                VALUES (?, ?, ?, ?, ?)
            """, (
                character_id, achievement_id, unlocked, unlock_date,
                json.dumps(progress, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个成就解锁记录")
    
    def generate_sample_save_data(self, count: int = 5):
        """生成示例存档数据"""
        print(f"生成 {count} 个示例存档...")
        
        # 获取玩家角色
        self.cursor.execute("SELECT id, name FROM characters WHERE is_player = 1")
        player_characters = self.cursor.fetchall()
        
        if not player_characters:
            print("没有找到玩家角色，跳过存档生成")
            return
        
        for i in range(count):
            slot_id = f"save_slot_{i+1}"
            character_id, player_name = random.choice(player_characters)
            
            chapter = random.randint(1, 6)
            play_time = random.randint(3600, 72000)  # 1-20小时
            
            game_data = {
                'current_level': f"level_{random.randint(1, 15)}",
                'inventory': {'gold': random.randint(100, 1000)},
                'settings': {'volume': random.randint(50, 100)}
            }
            
            settings_data = {
                'graphics_quality': random.choice(['low', 'medium', 'high']),
                'sound_volume': random.randint(50, 100),
                'music_volume': random.randint(50, 100)
            }
            
            self.cursor.execute("""
                INSERT OR REPLACE INTO save_data 
                (slot_id, player_name, character_id, chapter, play_time, game_data, settings_data)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            """, (
                slot_id, player_name, character_id, chapter, play_time,
                json.dumps(game_data, ensure_ascii=False),
                json.dumps(settings_data, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个存档")
    
    def generate_sample_logs(self, count: int = 100):
        """生成示例日志"""
        print(f"生成 {count} 个示例日志...")
        
        # 获取角色ID
        self.cursor.execute("SELECT id FROM characters")
        character_ids = [row[0] for row in self.cursor.fetchall()]
        
        log_levels = ['debug', 'info', 'warning', 'error']
        log_categories = ['combat', 'quest', 'level', 'system', 'user']
        
        messages = [
            '角色升级了', '战斗胜利', '任务完成', '获得物品', '装备损坏',
            '技能学习', '关卡解锁', '成就达成', '存档保存', '游戏启动'
        ]
        
        for i in range(count):
            character_id = random.choice(character_ids) if random.choice([True, False]) else None
            log_level = random.choice(log_levels)
            log_category = random.choice(log_categories)
            message = random.choice(messages)
            
            data = {
                'timestamp': datetime.now().isoformat(),
                'user_id': character_id,
                'session_id': f"session_{uuid.uuid4().hex[:8]}"
            }
            
            self.cursor.execute("""
                INSERT INTO game_logs 
                (character_id, log_level, log_category, message, data)
                VALUES (?, ?, ?, ?, ?)
            """, (
                character_id, log_level, log_category, message,
                json.dumps(data, ensure_ascii=False)
            ))
        
        self.conn.commit()
        print(f"成功生成 {count} 个日志")
    
    def generate_all_sample_data(self):
        """生成所有示例数据"""
        print("开始生成所有示例数据...")
        
        if not self.connect():
            return False
        
        try:
            # 生成各种示例数据
            self.generate_sample_characters(15)
            self.generate_sample_quests(25)
            self.generate_sample_levels(20)
            self.generate_sample_battle_records(80)
            self.generate_sample_quest_progress(50)
            self.generate_sample_level_progress(40)
            self.generate_sample_achievements(30)
            self.generate_sample_save_data(8)
            self.generate_sample_logs(150)
            
            print("所有示例数据生成完成！")
            return True
            
        except Exception as e:
            print(f"生成示例数据时出错: {e}")
            return False
        finally:
            self.disconnect()
    
    def show_database_stats(self):
        """显示数据库统计信息"""
        if not self.connect():
            return
        
        try:
            tables = [
                'characters', 'character_skills', 'character_equipment', 'character_inventory',
                'quests', 'character_quests', 'levels', 'character_level_progress',
                'battle_records', 'status_effects', 'achievements', 'character_achievements',
                'save_data', 'statistics', 'config', 'game_logs'
            ]
            
            print("\n=== 数据库统计信息 ===")
            for table in tables:
                self.cursor.execute(f"SELECT COUNT(*) FROM {table}")
                count = self.cursor.fetchone()[0]
                print(f"{table}: {count} 条记录")
            
            # 显示一些有趣的统计
            print("\n=== 详细统计 ===")
            
            # 角色职业分布
            self.cursor.execute("""
                SELECT class, COUNT(*) as count 
                FROM characters 
                GROUP BY class
            """)
            print("角色职业分布:")
            for row in self.cursor.fetchall():
                print(f"  {row[0]}: {row[1]} 个")
            
            # 任务类型分布
            self.cursor.execute("""
                SELECT type, COUNT(*) as count 
                FROM quests 
                GROUP BY type
            """)
            print("\n任务类型分布:")
            for row in self.cursor.fetchall():
                print(f"  {row[0]}: {row[1]} 个")
            
            # 关卡类型分布
            self.cursor.execute("""
                SELECT type, COUNT(*) as count 
                FROM levels 
                GROUP BY type
            """)
            print("\n关卡类型分布:")
            for row in self.cursor.fetchall():
                print(f"  {row[0]}: {row[1]} 个")
            
            # 战斗结果分布
            self.cursor.execute("""
                SELECT result, COUNT(*) as count 
                FROM battle_records 
                GROUP BY result
            """)
            print("\n战斗结果分布:")
            for row in self.cursor.fetchall():
                print(f"  {row[0]}: {row[1]} 次")
            
        except Exception as e:
            print(f"获取统计信息时出错: {e}")
        finally:
            self.disconnect()

def main():
    """主函数"""
    import sys
    import os
    
    # 获取数据库路径
    if len(sys.argv) > 1:
        database_path = sys.argv[1]
    else:
        # 默认数据库路径
        database_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'game_data.db')
    
    print(f"使用数据库路径: {database_path}")
    
    # 创建数据生成器
    seeder = DatabaseSeeder(database_path)
    
    # 检查命令行参数
    if len(sys.argv) > 2:
        command = sys.argv[2]
        
        if command == 'init':
            # 初始化数据库
            script_path = os.path.join(os.path.dirname(__file__), '..', 'resources', 'scripts', 'database_init.sql')
            if seeder.execute_script(script_path):
                print("数据库初始化成功")
            else:
                print("数据库初始化失败")
        
        elif command == 'seed':
            # 生成示例数据
            seeder.generate_all_sample_data()
        
        elif command == 'stats':
            # 显示统计信息
            seeder.show_database_stats()
        
        elif command == 'full':
            # 完整流程：初始化 + 生成数据
            script_path = os.path.join(os.path.dirname(__file__), '..', 'resources', 'scripts', 'database_init.sql')
            if seeder.execute_script(script_path):
                print("数据库初始化成功")
                seeder.generate_all_sample_data()
                seeder.show_database_stats()
            else:
                print("数据库初始化失败")
        
        else:
            print("未知命令。可用命令: init, seed, stats, full")
    else:
        print("用法: python database_seeder.py <database_path> <command>")
        print("命令:")
        print("  init  - 初始化数据库结构")
        print("  seed  - 生成示例数据")
        print("  stats - 显示数据库统计")
        print("  full  - 完整流程（初始化+生成数据）")

if __name__ == "__main__":
    main() 