#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
《幻境传说》数据库功能测试脚本
用于验证数据库的各种功能是否正常工作
"""

import sqlite3
import json
import os
import sys
from datetime import datetime

class DatabaseTester:
    def __init__(self, database_path: str):
        self.database_path = database_path
        self.conn = None
        self.cursor = None
        
    def connect(self):
        """连接到数据库"""
        try:
            self.conn = sqlite3.connect(self.database_path)
            self.cursor = self.conn.cursor()
            print(f"✓ 成功连接到数据库: {self.database_path}")
            return True
        except Exception as e:
            print(f"✗ 连接数据库失败: {e}")
            return False
    
    def disconnect(self):
        """断开数据库连接"""
        if self.conn:
            self.conn.close()
            print("数据库连接已关闭")
    
    def test_table_structure(self):
        """测试表结构"""
        print("\n=== 测试表结构 ===")
        
        expected_tables = [
            'characters', 'character_attributes', 'skills', 'character_skills',
            'equipment', 'character_equipment', 'items', 'character_inventory',
            'quests', 'character_quests', 'levels', 'character_level_progress',
            'battle_records', 'status_effects', 'achievements', 'character_achievements',
            'save_data', 'statistics', 'config', 'game_logs', 'database_version'
        ]
        
        self.cursor.execute("SELECT name FROM sqlite_master WHERE type='table'")
        existing_tables = [row[0] for row in self.cursor.fetchall()]
        
        print(f"期望的表数量: {len(expected_tables)}")
        print(f"实际的表数量: {len(existing_tables)}")
        
        missing_tables = set(expected_tables) - set(existing_tables)
        extra_tables = set(existing_tables) - set(expected_tables)
        
        if missing_tables:
            print(f"✗ 缺少的表: {missing_tables}")
        else:
            print("✓ 所有必需的表都存在")
        
        if extra_tables:
            print(f"⚠ 额外的表: {extra_tables}")
        
        return len(missing_tables) == 0
    
    def test_sample_queries(self):
        """测试示例查询"""
        print("\n=== 测试示例查询 ===")
        
        queries = [
            {
                'name': '获取所有战士角色',
                'sql': 'SELECT COUNT(*) FROM characters WHERE class = "warrior"',
                'expected_min': 1
            },
            {
                'name': '获取所有任务',
                'sql': 'SELECT COUNT(*) FROM quests',
                'expected_min': 1
            },
            {
                'name': '获取所有关卡',
                'sql': 'SELECT COUNT(*) FROM levels',
                'expected_min': 1
            },
            {
                'name': '获取所有成就',
                'sql': 'SELECT COUNT(*) FROM achievements',
                'expected_min': 1
            },
            {
                'name': '获取战斗记录',
                'sql': 'SELECT COUNT(*) FROM battle_records',
                'expected_min': 1
            }
        ]
        
        all_passed = True
        
        for query in queries:
            try:
                self.cursor.execute(query['sql'])
                result = self.cursor.fetchone()[0]
                
                if result >= query['expected_min']:
                    print(f"✓ {query['name']}: {result} 条记录")
                else:
                    print(f"✗ {query['name']}: 只有 {result} 条记录，期望至少 {query['expected_min']} 条")
                    all_passed = False
            except Exception as e:
                print(f"✗ {query['name']}: 查询失败 - {e}")
                all_passed = False
        
        return all_passed
    
    def run_all_tests(self):
        """运行所有测试"""
        print("开始数据库功能测试...")
        
        if not self.connect():
            return False
        
        try:
            tests = [
                self.test_table_structure,
                self.test_sample_queries
            ]
            
            passed_tests = 0
            total_tests = len(tests)
            
            for test in tests:
                try:
                    if test():
                        passed_tests += 1
                except Exception as e:
                    print(f"✗ 测试 {test.__name__} 失败: {e}")
            
            print(f"\n=== 测试结果 ===")
            print(f"通过测试: {passed_tests}/{total_tests}")
            
            if passed_tests == total_tests:
                print("🎉 所有测试通过！数据库功能正常。")
                return True
            else:
                print("⚠ 部分测试失败，请检查数据库。")
                return False
                
        except Exception as e:
            print(f"✗ 测试过程中出错: {e}")
            return False
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
    
    print(f"测试数据库: {database_path}")
    
    # 检查数据库文件是否存在
    if not os.path.exists(database_path):
        print(f"✗ 数据库文件不存在: {database_path}")
        print("请先运行数据库初始化脚本:")
        print("python database_seeder.py <database_path> init")
        return False
    
    # 创建测试器并运行测试
    tester = DatabaseTester(database_path)
    success = tester.run_all_tests()
    
    if success:
        print("\n✅ 数据库测试完成，所有功能正常！")
    else:
        print("\n❌ 数据库测试完成，发现问题需要修复。")
    
    return success

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1) 