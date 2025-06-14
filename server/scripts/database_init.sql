-- 《幻境传说》数据库初始化脚本
-- 版本: 1.0
-- 创建时间: 2025-06-13

-- 启用外键约束
PRAGMA foreign_keys = ON;

-- 启用WAL模式以提高并发性能
PRAGMA journal_mode = WAL;

-- 设置缓存大小（以页为单位，每页4KB）
PRAGMA cache_size = 10000;

-- 设置同步模式
PRAGMA synchronous = NORMAL;

-- 创建数据库版本控制表
CREATE TABLE IF NOT EXISTS database_version (
    version INTEGER PRIMARY KEY,
    description TEXT,
    applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 插入初始版本记录
INSERT OR IGNORE INTO database_version (version, description) VALUES (1, 'Initial database schema');

-- 1. 角色表
CREATE TABLE IF NOT EXISTS characters (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    class TEXT NOT NULL,
    level INTEGER DEFAULT 1,
    experience INTEGER DEFAULT 0,
    health INTEGER DEFAULT 100,
    max_health INTEGER DEFAULT 100,
    mana INTEGER DEFAULT 50,
    max_mana INTEGER DEFAULT 50,
    attack INTEGER DEFAULT 15,
    defense INTEGER DEFAULT 10,
    speed INTEGER DEFAULT 5,
    critical_rate REAL DEFAULT 0.05,
    critical_damage REAL DEFAULT 1.5,
    position_x REAL DEFAULT 0,
    position_y REAL DEFAULT 0,
    direction TEXT DEFAULT 'down',
    is_player BOOLEAN DEFAULT FALSE,
    is_alive BOOLEAN DEFAULT TRUE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 2. 角色属性表
CREATE TABLE IF NOT EXISTS character_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    attribute_name TEXT NOT NULL,
    base_value REAL DEFAULT 0,
    bonus_value REAL DEFAULT 0,
    final_value REAL DEFAULT 0,
    source TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    UNIQUE(character_id, attribute_name, source)
);

-- 3. 技能表
CREATE TABLE IF NOT EXISTS skills (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    element TEXT,
    class_requirement TEXT,
    level_requirement INTEGER DEFAULT 1,
    mana_cost INTEGER DEFAULT 0,
    cooldown REAL DEFAULT 0,
    damage REAL DEFAULT 0,
    healing REAL DEFAULT 0,
    range INTEGER DEFAULT 1,
    area_of_effect INTEGER DEFAULT 0,
    target_type TEXT DEFAULT 'enemy',
    effects TEXT,
    icon_path TEXT,
    animation_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 4. 角色技能表
CREATE TABLE IF NOT EXISTS character_skills (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    skill_id TEXT NOT NULL,
    skill_level INTEGER DEFAULT 1,
    is_equipped BOOLEAN DEFAULT FALSE,
    cooldown_remaining REAL DEFAULT 0,
    experience INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (skill_id) REFERENCES skills(id) ON DELETE CASCADE,
    UNIQUE(character_id, skill_id)
);

-- 5. 装备表
CREATE TABLE IF NOT EXISTS equipment (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    slot TEXT NOT NULL,
    rarity TEXT DEFAULT 'common',
    level_requirement INTEGER DEFAULT 1,
    class_requirement TEXT,
    durability INTEGER DEFAULT 100,
    max_durability INTEGER DEFAULT 100,
    attributes TEXT,
    special_effects TEXT,
    icon_path TEXT,
    model_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 6. 角色装备表
CREATE TABLE IF NOT EXISTS character_equipment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    equipment_id TEXT NOT NULL,
    slot TEXT NOT NULL,
    is_equipped BOOLEAN DEFAULT FALSE,
    durability INTEGER DEFAULT 100,
    enchant_level INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (equipment_id) REFERENCES equipment(id) ON DELETE CASCADE,
    UNIQUE(character_id, slot)
);

-- 7. 物品表
CREATE TABLE IF NOT EXISTS items (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    rarity TEXT DEFAULT 'common',
    stackable BOOLEAN DEFAULT TRUE,
    max_stack INTEGER DEFAULT 99,
    sell_price INTEGER DEFAULT 0,
    buy_price INTEGER DEFAULT 0,
    effects TEXT,
    icon_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 8. 角色背包表
CREATE TABLE IF NOT EXISTS character_inventory (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    item_id TEXT NOT NULL,
    quantity INTEGER DEFAULT 1,
    slot_index INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (item_id) REFERENCES items(id) ON DELETE CASCADE,
    UNIQUE(character_id, slot_index)
);

-- 9. 任务表
CREATE TABLE IF NOT EXISTS quests (
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    chapter INTEGER DEFAULT 1,
    level_requirement INTEGER DEFAULT 1,
    prerequisites TEXT,
    objectives TEXT,
    rewards TEXT,
    time_limit INTEGER DEFAULT 0,
    repeatable BOOLEAN DEFAULT FALSE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 10. 角色任务表
CREATE TABLE IF NOT EXISTS character_quests (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    quest_id TEXT NOT NULL,
    status TEXT DEFAULT 'not_started',
    progress TEXT,
    start_time DATETIME,
    complete_time DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (quest_id) REFERENCES quests(id) ON DELETE CASCADE,
    UNIQUE(character_id, quest_id)
);

-- 11. 关卡表
CREATE TABLE IF NOT EXISTS levels (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    chapter INTEGER DEFAULT 1,
    difficulty REAL DEFAULT 1.0,
    level_requirement INTEGER DEFAULT 1,
    enemies TEXT,
    boss TEXT,
    rewards TEXT,
    requirements TEXT,
    map_data TEXT,
    background_music TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 12. 角色关卡进度表
CREATE TABLE IF NOT EXISTS character_level_progress (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    level_id TEXT NOT NULL,
    status TEXT DEFAULT 'locked',
    completion_time INTEGER DEFAULT 0,
    score INTEGER DEFAULT 0,
    stars INTEGER DEFAULT 0,
    attempts INTEGER DEFAULT 0,
    best_time INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (level_id) REFERENCES levels(id) ON DELETE CASCADE,
    UNIQUE(character_id, level_id)
);

-- 13. 战斗记录表
CREATE TABLE IF NOT EXISTS battle_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    level_id TEXT,
    battle_type TEXT NOT NULL,
    opponent_type TEXT,
    result TEXT NOT NULL,
    duration INTEGER DEFAULT 0,
    damage_dealt INTEGER DEFAULT 0,
    damage_taken INTEGER DEFAULT 0,
    skills_used TEXT,
    combo_max INTEGER DEFAULT 0,
    experience_gained INTEGER DEFAULT 0,
    items_dropped TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (level_id) REFERENCES levels(id) ON DELETE SET NULL
);

-- 14. 状态效果表
CREATE TABLE IF NOT EXISTS status_effects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    effect_type TEXT NOT NULL,
    effect_name TEXT NOT NULL,
    value REAL DEFAULT 0,
    duration REAL DEFAULT 0,
    remaining_time REAL DEFAULT 0,
    stack_count INTEGER DEFAULT 1,
    source TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE
);

-- 15. 成就表
CREATE TABLE IF NOT EXISTS achievements (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    type TEXT NOT NULL,
    category TEXT,
    condition_data TEXT,
    reward_data TEXT,
    icon_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 16. 角色成就表
CREATE TABLE IF NOT EXISTS character_achievements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    achievement_id TEXT NOT NULL,
    unlocked BOOLEAN DEFAULT FALSE,
    unlock_date DATETIME,
    progress TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (achievement_id) REFERENCES achievements(id) ON DELETE CASCADE,
    UNIQUE(character_id, achievement_id)
);

-- 17. 游戏存档表
CREATE TABLE IF NOT EXISTS save_data (
    slot_id TEXT PRIMARY KEY,
    player_name TEXT NOT NULL,
    character_id TEXT NOT NULL,
    chapter INTEGER DEFAULT 1,
    level_id TEXT,
    play_time INTEGER DEFAULT 0,
    save_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    game_data TEXT,
    settings_data TEXT,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    FOREIGN KEY (level_id) REFERENCES levels(id) ON DELETE SET NULL
);

-- 18. 统计数据表
CREATE TABLE IF NOT EXISTS statistics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT NOT NULL,
    stat_key TEXT NOT NULL,
    stat_value REAL DEFAULT 0,
    stat_type TEXT DEFAULT 'integer',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE,
    UNIQUE(character_id, stat_key)
);

-- 19. 配置表
CREATE TABLE IF NOT EXISTS config (
    key TEXT PRIMARY KEY,
    value TEXT,
    type TEXT DEFAULT 'string',
    description TEXT,
    category TEXT DEFAULT 'general',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 20. 日志表
CREATE TABLE IF NOT EXISTS game_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id TEXT,
    log_level TEXT NOT NULL,
    log_category TEXT NOT NULL,
    message TEXT NOT NULL,
    data TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE SET NULL
);

-- 创建索引
-- 角色相关索引
CREATE INDEX IF NOT EXISTS idx_characters_class ON characters(class);
CREATE INDEX IF NOT EXISTS idx_characters_level ON characters(level);
CREATE INDEX IF NOT EXISTS idx_characters_is_player ON characters(is_player);
CREATE INDEX IF NOT EXISTS idx_characters_class_level ON characters(class, level);
CREATE INDEX IF NOT EXISTS idx_characters_position ON characters(position_x, position_y);

-- 角色属性索引
CREATE INDEX IF NOT EXISTS idx_character_attributes_character_id ON character_attributes(character_id);
CREATE INDEX IF NOT EXISTS idx_character_attributes_name ON character_attributes(attribute_name);

-- 技能相关索引
CREATE INDEX IF NOT EXISTS idx_skills_type ON skills(type);
CREATE INDEX IF NOT EXISTS idx_skills_element ON skills(element);
CREATE INDEX IF NOT EXISTS idx_skills_class_requirement ON skills(class_requirement);

-- 角色技能索引
CREATE INDEX IF NOT EXISTS idx_character_skills_character_id ON character_skills(character_id);
CREATE INDEX IF NOT EXISTS idx_character_skills_equipped ON character_skills(is_equipped);
CREATE INDEX IF NOT EXISTS idx_character_skills_character_equipped ON character_skills(character_id, is_equipped);

-- 装备相关索引
CREATE INDEX IF NOT EXISTS idx_equipment_type ON equipment(type);
CREATE INDEX IF NOT EXISTS idx_equipment_slot ON equipment(slot);
CREATE INDEX IF NOT EXISTS idx_equipment_rarity ON equipment(rarity);

-- 角色装备索引
CREATE INDEX IF NOT EXISTS idx_character_equipment_character_id ON character_equipment(character_id);
CREATE INDEX IF NOT EXISTS idx_character_equipment_equipped ON character_equipment(is_equipped);
CREATE INDEX IF NOT EXISTS idx_character_equipment_character_equipped ON character_equipment(character_id, is_equipped);

-- 物品相关索引
CREATE INDEX IF NOT EXISTS idx_items_type ON items(type);
CREATE INDEX IF NOT EXISTS idx_items_rarity ON items(rarity);
CREATE INDEX IF NOT EXISTS idx_items_type_rarity ON items(type, rarity);

-- 角色背包索引
CREATE INDEX IF NOT EXISTS idx_character_inventory_character_id ON character_inventory(character_id);
CREATE INDEX IF NOT EXISTS idx_character_inventory_item_id ON character_inventory(item_id);
CREATE INDEX IF NOT EXISTS idx_character_inventory_character_item ON character_inventory(character_id, item_id);

-- 任务相关索引
CREATE INDEX IF NOT EXISTS idx_quests_type ON quests(type);
CREATE INDEX IF NOT EXISTS idx_quests_chapter ON quests(chapter);
CREATE INDEX IF NOT EXISTS idx_quests_type_chapter ON quests(type, chapter);

-- 角色任务索引
CREATE INDEX IF NOT EXISTS idx_character_quests_character_id ON character_quests(character_id);
CREATE INDEX IF NOT EXISTS idx_character_quests_status ON character_quests(status);
CREATE INDEX IF NOT EXISTS idx_character_quests_character_status ON character_quests(character_id, status);

-- 关卡相关索引
CREATE INDEX IF NOT EXISTS idx_levels_type ON levels(type);
CREATE INDEX IF NOT EXISTS idx_levels_chapter ON levels(chapter);
CREATE INDEX IF NOT EXISTS idx_levels_difficulty ON levels(difficulty);
CREATE INDEX IF NOT EXISTS idx_levels_type_chapter_difficulty ON levels(type, chapter, difficulty);

-- 角色关卡进度索引
CREATE INDEX IF NOT EXISTS idx_character_level_progress_character_id ON character_level_progress(character_id);
CREATE INDEX IF NOT EXISTS idx_character_level_progress_status ON character_level_progress(status);
CREATE INDEX IF NOT EXISTS idx_character_level_progress_character_status ON character_level_progress(character_id, status);

-- 战斗记录索引
CREATE INDEX IF NOT EXISTS idx_battle_records_character_id ON battle_records(character_id);
CREATE INDEX IF NOT EXISTS idx_battle_records_result ON battle_records(result);
CREATE INDEX IF NOT EXISTS idx_battle_records_created_at ON battle_records(created_at);
CREATE INDEX IF NOT EXISTS idx_battle_records_character_date ON battle_records(character_id, created_at);

-- 状态效果索引
CREATE INDEX IF NOT EXISTS idx_status_effects_character_id ON status_effects(character_id);
CREATE INDEX IF NOT EXISTS idx_status_effects_type ON status_effects(effect_type);
CREATE INDEX IF NOT EXISTS idx_status_effects_character_time ON status_effects(character_id, remaining_time);

-- 成就相关索引
CREATE INDEX IF NOT EXISTS idx_achievements_type ON achievements(type);
CREATE INDEX IF NOT EXISTS idx_achievements_category ON achievements(category);

-- 角色成就索引
CREATE INDEX IF NOT EXISTS idx_character_achievements_character_id ON character_achievements(character_id);
CREATE INDEX IF NOT EXISTS idx_character_achievements_unlocked ON character_achievements(unlocked);
CREATE INDEX IF NOT EXISTS idx_character_achievements_character_unlocked ON character_achievements(character_id, unlocked);

-- 存档相关索引
CREATE INDEX IF NOT EXISTS idx_save_data_character_id ON save_data(character_id);
CREATE INDEX IF NOT EXISTS idx_save_data_save_date ON save_data(save_date);

-- 统计数据索引
CREATE INDEX IF NOT EXISTS idx_statistics_character_id ON statistics(character_id);
CREATE INDEX IF NOT EXISTS idx_statistics_key ON statistics(stat_key);

-- 配置索引
CREATE INDEX IF NOT EXISTS idx_config_category ON config(category);

-- 日志索引
CREATE INDEX IF NOT EXISTS idx_game_logs_character_id ON game_logs(character_id);
CREATE INDEX IF NOT EXISTS idx_game_logs_level ON game_logs(log_level);
CREATE INDEX IF NOT EXISTS idx_game_logs_category ON game_logs(log_category);
CREATE INDEX IF NOT EXISTS idx_game_logs_created_at ON game_logs(created_at);

-- 创建常用视图
CREATE VIEW IF NOT EXISTS character_summary AS
SELECT 
    c.id, c.name, c.class, c.level, c.health, c.mana,
    COUNT(cs.skill_id) as skill_count,
    COUNT(ce.equipment_id) as equipment_count,
    COUNT(cq.quest_id) as active_quest_count
FROM characters c
LEFT JOIN character_skills cs ON c.id = cs.character_id AND cs.is_equipped = 1
LEFT JOIN character_equipment ce ON c.id = ce.character_id AND ce.is_equipped = 1
LEFT JOIN character_quests cq ON c.id = cq.character_id AND cq.status = 'in_progress'
GROUP BY c.id;

-- 创建角色统计视图
CREATE VIEW IF NOT EXISTS character_stats AS
SELECT 
    c.id,
    c.name,
    c.class,
    c.level,
    COUNT(br.id) as total_battles,
    SUM(CASE WHEN br.result = 'victory' THEN 1 ELSE 0 END) as victories,
    SUM(CASE WHEN br.result = 'defeat' THEN 1 ELSE 0 END) as defeats,
    SUM(br.experience_gained) as total_experience_gained,
    SUM(br.damage_dealt) as total_damage_dealt,
    SUM(br.damage_taken) as total_damage_taken,
    MAX(br.combo_max) as best_combo
FROM characters c
LEFT JOIN battle_records br ON c.id = br.character_id
GROUP BY c.id;

-- 创建任务进度视图
CREATE VIEW IF NOT EXISTS quest_progress AS
SELECT 
    cq.character_id,
    cq.quest_id,
    q.title,
    q.type,
    q.chapter,
    cq.status,
    cq.start_time,
    cq.complete_time,
    CASE 
        WHEN cq.complete_time IS NOT NULL 
        THEN (julianday(cq.complete_time) - julianday(cq.start_time)) * 24 * 60 * 60
        ELSE NULL 
    END as completion_time_seconds
FROM character_quests cq
JOIN quests q ON cq.quest_id = q.id;

-- 创建关卡进度视图
CREATE VIEW IF NOT EXISTS level_progress AS
SELECT 
    clp.character_id,
    clp.level_id,
    l.name as level_name,
    l.type as level_type,
    l.chapter,
    l.difficulty,
    clp.status,
    clp.completion_time,
    clp.score,
    clp.stars,
    clp.attempts,
    clp.best_time
FROM character_level_progress clp
JOIN levels l ON clp.level_id = l.id;

-- 插入默认配置数据
INSERT OR IGNORE INTO config (key, value, type, description, category) VALUES
('game_version', '1.0.0', 'string', '游戏版本', 'system'),
('auto_save_interval', '300', 'integer', '自动保存间隔（秒）', 'gameplay'),
('difficulty_level', 'normal', 'string', '游戏难度', 'gameplay'),
('sound_volume', '80', 'integer', '音效音量', 'audio'),
('music_volume', '70', 'integer', '音乐音量', 'audio'),
('graphics_quality', 'high', 'string', '图形质量', 'graphics'),
('fullscreen', 'false', 'boolean', '全屏模式', 'graphics'),
('language', 'zh_CN', 'string', '游戏语言', 'localization'),
('tutorial_completed', 'false', 'boolean', '教程完成状态', 'gameplay'),
('first_launch', 'true', 'boolean', '首次启动', 'system');

-- 插入默认成就数据
INSERT OR IGNORE INTO achievements (id, name, description, type, category, condition_data, reward_data) VALUES
('first_victory', '初次胜利', '赢得第一场战斗', 'combat', 'beginner', '{"battles_won": 1}', '{"experience": 100, "gold": 50}'),
('level_10', '等级提升', '角色达到10级', 'progression', 'beginner', '{"character_level": 10}', '{"experience": 500, "skill_points": 1}'),
('quest_master', '任务大师', '完成10个任务', 'story', 'intermediate', '{"quests_completed": 10}', '{"experience": 1000, "reputation": 50}'),
('combo_master', '连击大师', '达成10连击', 'combat', 'intermediate', '{"max_combo": 10}', '{"experience": 300, "skill_points": 1}'),
('explorer', '探索者', '解锁5个新区域', 'exploration', 'intermediate', '{"areas_unlocked": 5}', '{"experience": 800, "gold": 200}'),
('collector', '收藏家', '收集100件物品', 'collection', 'advanced', '{"items_collected": 100}', '{"experience": 1500, "special_item": "collector_badge"}'),
('perfect_warrior', '完美战士', '以满血状态完成关卡', 'combat', 'advanced', '{"perfect_clear": 1}', '{"experience": 2000, "achievement_points": 100}'),
('speed_runner', '速通专家', '在5分钟内完成关卡', 'combat', 'expert', '{"level_time": 300}', '{"experience": 3000, "special_title": "Speed Runner"}');

-- 插入默认技能数据
INSERT OR IGNORE INTO skills (id, name, description, type, element, class_requirement, level_requirement, mana_cost, cooldown, damage, healing, range, target_type, effects) VALUES
-- 战士技能
('basic_attack', '基础攻击', '对敌人造成基础伤害', 'active', NULL, 'warrior', 1, 0, 0, 20, 0, 1, 'enemy', '{"damage_type": "physical"}'),
('heavy_strike', '重击', '造成150%伤害的重击', 'active', NULL, 'warrior', 3, 10, 3, 30, 0, 1, 'enemy', '{"damage_type": "physical", "stun_chance": 0.2}'),
('defense_stance', '防御姿态', '提高防御力', 'active', NULL, 'warrior', 5, 15, 10, 0, 0, 0, 'self', '{"defense_boost": 50, "duration": 10}'),
('whirlwind', '旋风斩', '对周围敌人造成伤害', 'active', NULL, 'warrior', 7, 25, 8, 25, 0, 2, 'enemy', '{"damage_type": "physical", "area_effect": true}'),
('berserker_rage', '狂暴', '攻击力大幅提升', 'ultimate', NULL, 'warrior', 10, 50, 60, 0, 0, 0, 'self', '{"attack_boost": 100, "duration": 15}'),

-- 法师技能
('magic_missile', '魔法飞弹', '发射魔法飞弹', 'active', NULL, 'mage', 1, 5, 1, 15, 0, 3, 'enemy', '{"damage_type": "magic"}'),
('fireball', '火球术', '发射火球造成火属性伤害', 'active', 'fire', 'mage', 3, 15, 4, 25, 0, 4, 'enemy', '{"damage_type": "fire", "burn_chance": 0.3}'),
('ice_shield', '冰盾', '创造冰盾保护自己', 'active', 'ice', 'mage', 5, 20, 12, 0, 0, 0, 'self', '{"shield_value": 100, "duration": 8}'),
('lightning_bolt', '闪电箭', '发射闪电造成雷属性伤害', 'active', 'thunder', 'mage', 7, 25, 6, 35, 0, 5, 'enemy', '{"damage_type": "thunder", "stun_chance": 0.4}'),
('meteor_shower', '流星雨', '召唤流星雨攻击敌人', 'ultimate', 'fire', 'mage', 10, 60, 90, 50, 0, 6, 'enemy', '{"damage_type": "fire", "area_effect": true}'),

-- 刺客技能
('stealth', '潜行', '进入隐身状态', 'active', NULL, 'assassin', 1, 10, 15, 0, 0, 0, 'self', '{"stealth": true, "duration": 10}'),
('backstab', '背刺', '从背后攻击造成高伤害', 'active', NULL, 'assassin', 3, 15, 5, 40, 0, 1, 'enemy', '{"damage_type": "physical", "critical_chance": 0.8}'),
('poison_dart', '毒镖', '发射带毒的飞镖', 'active', NULL, 'assassin', 5, 12, 3, 15, 0, 4, 'enemy', '{"damage_type": "physical", "poison_chance": 0.6}'),
('shadow_step', '影步', '瞬间移动到目标位置', 'active', NULL, 'assassin', 7, 20, 8, 0, 0, 5, 'enemy', '{"teleport": true, "damage_bonus": 50}'),
('death_blow', '致命一击', '对敌人造成致命伤害', 'ultimate', NULL, 'assassin', 10, 40, 120, 100, 0, 1, 'enemy', '{"damage_type": "physical", "instant_kill_chance": 0.1}');

-- 插入默认装备数据
INSERT OR IGNORE INTO equipment (id, name, description, type, slot, rarity, level_requirement, class_requirement, attributes, special_effects) VALUES
-- 武器
('iron_sword', '铁剑', '普通的铁制长剑', 'weapon', 'weapon', 'common', 1, 'warrior', '{"attack": 15}', '{}'),
('steel_sword', '钢剑', '精制的钢制长剑', 'weapon', 'weapon', 'uncommon', 5, 'warrior', '{"attack": 25}', '{"critical_rate": 0.05}'),
('magic_staff', '魔法杖', '蕴含魔力的法杖', 'weapon', 'weapon', 'common', 1, 'mage', '{"attack": 8, "mana": 20}', '{"spell_power": 10}'),
('fire_staff', '火焰法杖', '增强火系魔法的法杖', 'weapon', 'weapon', 'uncommon', 5, 'mage', '{"attack": 12, "mana": 30}', '{"fire_damage": 15}'),
('dagger', '匕首', '锋利的短匕首', 'weapon', 'weapon', 'common', 1, 'assassin', '{"attack": 12, "speed": 5}', '{"critical_rate": 0.1}'),
('shadow_blade', '影刃', '暗影刺客的专用武器', 'weapon', 'weapon', 'uncommon', 5, 'assassin', '{"attack": 18, "speed": 8}', '{"stealth_bonus": 0.2}'),

-- 防具
('leather_armor', '皮甲', '轻便的皮革护甲', 'armor', 'chest', 'common', 1, NULL, '{"defense": 8}', '{"speed": 2}'),
('iron_armor', '铁甲', '坚固的铁制护甲', 'armor', 'chest', 'uncommon', 5, 'warrior', '{"defense": 15}', '{}'),
('cloth_robe', '布袍', '法师的魔法长袍', 'armor', 'chest', 'common', 1, 'mage', '{"defense": 5, "mana": 15}', '{"spell_power": 5}'),
('shadow_cloak', '影袍', '刺客的隐匿斗篷', 'armor', 'chest', 'uncommon', 5, 'assassin', '{"defense": 8, "speed": 5}', '{"stealth_bonus": 0.15}'),

-- 饰品
('health_ring', '生命戒指', '增加生命值', 'accessory', 'ring', 'common', 1, NULL, '{"max_health": 20}', '{}'),
('mana_ring', '法力戒指', '增加魔法值', 'accessory', 'ring', 'common', 1, NULL, '{"max_mana": 15}', '{}'),
('strength_ring', '力量戒指', '增加攻击力', 'accessory', 'ring', 'uncommon', 5, NULL, '{"attack": 8}', '{}'),
('defense_ring', '防御戒指', '增加防御力', 'accessory', 'ring', 'uncommon', 5, NULL, '{"defense": 8}', '{}');

-- 插入默认物品数据
INSERT OR IGNORE INTO items (id, name, description, type, rarity, effects) VALUES
-- 消耗品
('health_potion', '生命药水', '恢复50点生命值', 'consumable', 'common', '{"heal": 50}'),
('mana_potion', '法力药水', '恢复30点魔法值', 'consumable', 'common', '{"restore_mana": 30}'),
('strength_potion', '力量药水', '临时增加攻击力', 'consumable', 'uncommon', '{"attack_boost": 20, "duration": 300}'),
('defense_potion', '防御药水', '临时增加防御力', 'consumable', 'uncommon', '{"defense_boost": 15, "duration": 300}'),

-- 材料
('iron_ore', '铁矿', '用于制作铁制装备', 'material', 'common', '{}'),
('magic_crystal', '魔法水晶', '用于制作魔法装备', 'material', 'uncommon', '{}'),
('poison_essence', '毒液精华', '用于制作毒药', 'material', 'rare', '{}'),
('fire_essence', '火焰精华', '用于制作火系装备', 'material', 'rare', '{}'),

-- 货币
('gold_coin', '金币', '通用货币', 'currency', 'common', '{}'),
('silver_coin', '银币', '次要货币', 'currency', 'common', '{}'),
('premium_gem', '宝石', '高级货币', 'currency', 'rare', '{}');

-- 插入默认任务数据
INSERT OR IGNORE INTO quests (id, title, description, type, chapter, level_requirement, objectives, rewards) VALUES
('tutorial_combat', '战斗教程', '学习基础战斗技巧', 'main', 1, 1, '[{"type": "kill", "target": "training_dummy", "count": 1}]', '{"experience": 50, "gold": 20}'),
('first_quest', '第一个任务', '完成你的第一个任务', 'main', 1, 1, '[{"type": "talk", "target": "village_elder", "count": 1}]', '{"experience": 100, "gold": 50}'),
('collect_materials', '收集材料', '收集一些基础材料', 'side', 1, 2, '[{"type": "collect", "item": "iron_ore", "count": 5}]', '{"experience": 80, "items": ["health_potion"]}'),
('defeat_goblins', '击败哥布林', '清理村庄附近的哥布林', 'main', 1, 3, '[{"type": "kill", "target": "goblin", "count": 3}]', '{"experience": 150, "gold": 100}');

-- 插入默认关卡数据
INSERT OR IGNORE INTO levels (id, name, description, type, chapter, difficulty, level_requirement, enemies, rewards) VALUES
('tutorial_level', '教程关卡', '学习基础操作的关卡', 'main', 1, 1.0, 1, '[{"type": "training_dummy", "count": 1, "position": [100, 100]}]', '{"experience": 50, "gold": 20}'),
('village_outskirts', '村庄外围', '村庄附近的区域', 'main', 1, 1.2, 2, '[{"type": "goblin", "count": 2, "position": [200, 150]}, {"type": "wolf", "count": 1, "position": [300, 200]}]', '{"experience": 100, "gold": 50}'),
('forest_entrance', '森林入口', '进入森林的第一个区域', 'main', 1, 1.5, 3, '[{"type": "goblin", "count": 3, "position": [150, 100]}, {"type": "wolf", "count": 2, "position": [250, 150]}]', '{"experience": 150, "gold": 80}'),
('goblin_camp', '哥布林营地', '哥布林的临时营地', 'main', 1, 2.0, 4, '[{"type": "goblin", "count": 4, "position": [100, 100]}, {"type": "goblin_chief", "count": 1, "position": [200, 150]}]', '{"experience": 200, "gold": 120, "items": ["iron_sword"]}');

-- 创建触发器用于自动更新时间戳
CREATE TRIGGER IF NOT EXISTS update_characters_timestamp 
AFTER UPDATE ON characters
BEGIN
    UPDATE characters SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_attributes_timestamp 
AFTER UPDATE ON character_attributes
BEGIN
    UPDATE character_attributes SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_skills_timestamp 
AFTER UPDATE ON character_skills
BEGIN
    UPDATE character_skills SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_equipment_timestamp 
AFTER UPDATE ON character_equipment
BEGIN
    UPDATE character_equipment SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_inventory_timestamp 
AFTER UPDATE ON character_inventory
BEGIN
    UPDATE character_inventory SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_quests_timestamp 
AFTER UPDATE ON character_quests
BEGIN
    UPDATE character_quests SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_character_level_progress_timestamp 
AFTER UPDATE ON character_level_progress
BEGIN
    UPDATE character_level_progress SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_status_effects_timestamp 
AFTER UPDATE ON status_effects
BEGIN
    UPDATE status_effects SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_statistics_timestamp 
AFTER UPDATE ON statistics
BEGIN
    UPDATE statistics SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_config_timestamp 
AFTER UPDATE ON config
BEGIN
    UPDATE config SET updated_at = CURRENT_TIMESTAMP WHERE key = NEW.key;
END;

-- 创建数据完整性检查触发器
CREATE TRIGGER IF NOT EXISTS check_character_health
BEFORE UPDATE ON characters
BEGIN
    SELECT CASE 
        WHEN NEW.health < 0 THEN RAISE(ABORT, 'Health cannot be negative')
        WHEN NEW.health > NEW.max_health THEN RAISE(ABORT, 'Health cannot exceed max health')
        WHEN NEW.mana < 0 THEN RAISE(ABORT, 'Mana cannot be negative')
        WHEN NEW.mana > NEW.max_mana THEN RAISE(ABORT, 'Mana cannot exceed max mana')
    END;
END;

CREATE TRIGGER IF NOT EXISTS check_skill_level
BEFORE UPDATE ON character_skills
BEGIN
    SELECT CASE 
        WHEN NEW.skill_level < 1 THEN RAISE(ABORT, 'Skill level cannot be less than 1')
        WHEN NEW.skill_level > 10 THEN RAISE(ABORT, 'Skill level cannot exceed 10')
    END;
END;

CREATE TRIGGER IF NOT EXISTS check_equipment_durability
BEFORE UPDATE ON character_equipment
BEGIN
    SELECT CASE 
        WHEN NEW.durability < 0 THEN RAISE(ABORT, 'Durability cannot be negative')
        WHEN NEW.durability > 100 THEN RAISE(ABORT, 'Durability cannot exceed 100')
    END;
END;

-- 创建自动清理过期状态效果的触发器
CREATE TRIGGER IF NOT EXISTS cleanup_expired_effects
AFTER UPDATE ON status_effects
BEGIN
    DELETE FROM status_effects WHERE remaining_time <= 0;
END;

-- 创建自动更新角色属性的触发器
CREATE TRIGGER IF NOT EXISTS update_character_final_attributes
AFTER INSERT OR UPDATE OR DELETE ON character_attributes
BEGIN
    UPDATE character_attributes 
    SET final_value = base_value + bonus_value 
    WHERE character_id = COALESCE(NEW.character_id, OLD.character_id);
END;

-- 创建自动记录战斗日志的触发器
CREATE TRIGGER IF NOT EXISTS log_battle_record
AFTER INSERT ON battle_records
BEGIN
    INSERT INTO game_logs (character_id, log_level, log_category, message, data)
    VALUES (
        NEW.character_id,
        'info',
        'combat',
        'Battle completed: ' || NEW.result,
        json_object(
            'battle_type', NEW.battle_type,
            'duration', NEW.duration,
            'damage_dealt', NEW.damage_dealt,
            'damage_taken', NEW.damage_taken,
            'experience_gained', NEW.experience_gained
        )
    );
END;

-- 创建自动更新统计数据的触发器
CREATE TRIGGER IF NOT EXISTS update_battle_statistics
AFTER INSERT ON battle_records
BEGIN
    -- 更新总战斗次数
    INSERT OR REPLACE INTO statistics (character_id, stat_key, stat_value, stat_type)
    VALUES (NEW.character_id, 'total_battles', 
        COALESCE((SELECT stat_value FROM statistics WHERE character_id = NEW.character_id AND stat_key = 'total_battles'), 0) + 1,
        'integer');
    
    -- 更新胜利次数
    IF NEW.result = 'victory' THEN
        INSERT OR REPLACE INTO statistics (character_id, stat_key, stat_value, stat_type)
        VALUES (NEW.character_id, 'battles_won', 
            COALESCE((SELECT stat_value FROM statistics WHERE character_id = NEW.character_id AND stat_key = 'battles_won'), 0) + 1,
            'integer');
    END IF;
    
    -- 更新总经验获得
    INSERT OR REPLACE INTO statistics (character_id, stat_key, stat_value, stat_type)
    VALUES (NEW.character_id, 'total_experience_gained', 
        COALESCE((SELECT stat_value FROM statistics WHERE character_id = NEW.character_id AND stat_key = 'total_experience_gained'), 0) + NEW.experience_gained,
        'integer');
    
    -- 更新最大连击数
    IF NEW.combo_max > COALESCE((SELECT stat_value FROM statistics WHERE character_id = NEW.character_id AND stat_key = 'max_combo'), 0) THEN
        INSERT OR REPLACE INTO statistics (character_id, stat_key, stat_value, stat_type)
        VALUES (NEW.character_id, 'max_combo', NEW.combo_max, 'integer');
    END IF;
END;

-- 创建自动检查成就的触发器
CREATE TRIGGER IF NOT EXISTS check_achievements
AFTER UPDATE ON statistics
BEGIN
    -- 检查首次胜利成就
    IF NEW.stat_key = 'battles_won' AND NEW.stat_value = 1 THEN
        INSERT OR IGNORE INTO character_achievements (character_id, achievement_id, unlocked, unlock_date)
        VALUES (NEW.character_id, 'first_victory', TRUE, CURRENT_TIMESTAMP);
    END IF;
    
    -- 检查等级成就
    IF NEW.stat_key = 'character_level' AND NEW.stat_value >= 10 THEN
        INSERT OR IGNORE INTO character_achievements (character_id, achievement_id, unlocked, unlock_date)
        VALUES (NEW.character_id, 'level_10', TRUE, CURRENT_TIMESTAMP);
    END IF;
    
    -- 检查连击成就
    IF NEW.stat_key = 'max_combo' AND NEW.stat_value >= 10 THEN
        INSERT OR IGNORE INTO character_achievements (character_id, achievement_id, unlocked, unlock_date)
        VALUES (NEW.character_id, 'combo_master', TRUE, CURRENT_TIMESTAMP);
    END IF;
END;

-- 优化数据库
VACUUM;
ANALYZE;

-- 完成初始化
SELECT 'Database initialization completed successfully!' as status; 