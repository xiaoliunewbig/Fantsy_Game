#!/usr/bin/env python3
"""
Fantasy Legend MCP Server
基于 interactive-feedback-mcp 的简化版本
"""

import os
import sys
import json
import subprocess
import tempfile
from pathlib import Path
from typing import Annotated, Dict

from fastmcp import FastMCP
from pydantic import Field

# 创建MCP服务器实例
mcp = FastMCP("Fantasy Legend MCP", log_level="INFO")

# 项目根目录
PROJECT_ROOT = Path(__file__).parent.parent
CLIENT_DIR = PROJECT_ROOT / "client"
SERVER_DIR = PROJECT_ROOT / "server"

def run_command(cmd: list, cwd: str = None) -> Dict[str, any]:
    """运行命令并返回结果"""
    try:
        result = subprocess.run(
            cmd, 
            cwd=cwd, 
            capture_output=True, 
            text=True, 
            timeout=300
        )
        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode
        }
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }

@mcp.tool()
def compile_client(
    build_type: Annotated[str, Field(description="Build type: debug, release, or clean")] = "debug"
) -> Dict[str, str]:
    """编译客户端项目"""
    try:
        build_dir = CLIENT_DIR / "build"
        build_dir.mkdir(exist_ok=True)
        
        if build_type == "clean":
            import shutil
            if build_dir.exists():
                shutil.rmtree(build_dir)
            return {"success": True, "message": "Build directory cleaned"}
        
        # 配置CMake
        cmake_cmd = ["cmake", ".."]
        if build_type == "release":
            cmake_cmd.extend(["-DCMAKE_BUILD_TYPE=Release"])
        else:
            cmake_cmd.extend(["-DCMAKE_BUILD_TYPE=Debug"])
        
        result = run_command(cmake_cmd, str(build_dir))
        if not result["success"]:
            return {"success": False, "error": f"CMake configuration failed: {result.get('stderr', '')}"}
        
        # 编译
        make_cmd = ["make", "-j4"]
        result = run_command(make_cmd, str(build_dir))
        
        if result["success"]:
            return {"success": True, "message": f"Client compiled successfully in {build_type} mode"}
        else:
            return {"success": False, "error": f"Compilation failed: {result.get('stderr', '')}"}
            
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def compile_server(
    build_type: Annotated[str, Field(description="Build type: debug, release, or clean")] = "debug"
) -> Dict[str, str]:
    """编译服务器项目"""
    try:
        build_dir = SERVER_DIR / "build"
        build_dir.mkdir(exist_ok=True)
        
        if build_type == "clean":
            # 清理构建目录
            import shutil
            if build_dir.exists():
                shutil.rmtree(build_dir)
            return {"success": True, "message": "Build directory cleaned"}
        
        # 配置CMake
        cmake_cmd = ["cmake", ".."]
        if build_type == "release":
            cmake_cmd.extend(["-DCMAKE_BUILD_TYPE=Release"])
        else:
            cmake_cmd.extend(["-DCMAKE_BUILD_TYPE=Debug"])
        
        result = run_command(cmake_cmd, str(build_dir))
        if not result["success"]:
            return {"success": False, "error": f"CMake configuration failed: {result.get('stderr', '')}"}
        
        # 编译
        make_cmd = ["make", "-j4"]
        result = run_command(make_cmd, str(build_dir))
        
        if result["success"]:
            return {"success": True, "message": f"Server compiled successfully in {build_type} mode"}
        else:
            return {"success": False, "error": f"Compilation failed: {result.get('stderr', '')}"}
            
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def run_client() -> Dict[str, str]:
    """运行客户端"""
    try:
        client_exe = CLIENT_DIR / "build" / "bin" / "FantasyLegend_Client"
        if not client_exe.exists():
            return {"success": False, "error": "Client executable not found. Please compile first."}
        
        result = run_command([str(client_exe)], capture_output=False)
        return {"success": result["success"], "message": "Client started"}
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def run_server() -> Dict[str, str]:
    """运行服务器"""
    try:
        server_exe = SERVER_DIR / "build" / "bin" / "FantasyLegend_Server"
        if not server_exe.exists():
            return {"success": False, "error": "Server executable not found. Please compile first."}
        
        result = run_command([str(server_exe)], capture_output=False)
        return {"success": result["success"], "message": "Server started"}
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def check_compilation_errors() -> Dict[str, str]:
    """检查编译错误"""
    try:
        errors = []
        
        # 检查客户端编译错误
        client_build = CLIENT_DIR / "build"
        if client_build.exists():
            result = run_command(["make", "-j1"], cwd=client_build)
            if not result["success"]:
                errors.append(f"Client: {result.get('stderr', 'Unknown error')}")
        
        # 检查服务器编译错误
        server_build = SERVER_DIR / "build"
        if server_build.exists():
            result = run_command(["make", "-j1"], cwd=server_build)
            if not result["success"]:
                errors.append(f"Server: {result.get('stderr', 'Unknown error')}")
        
        if errors:
            return {"success": False, "errors": errors}
        else:
            return {"success": True, "message": "No compilation errors found"}
            
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def get_project_status() -> Dict[str, str]:
    """获取项目状态信息"""
    try:
        status = {
            "project_root": str(PROJECT_ROOT),
            "client_dir": str(CLIENT_DIR),
            "server_dir": str(SERVER_DIR),
            "client_compiled": (CLIENT_DIR / "build" / "bin" / "FantasyLegend_Client").exists(),
            "server_compiled": (SERVER_DIR / "build" / "bin" / "FantasyLegend_Server").exists(),
            "qt_path": "/home/pck/tools/qtlib"
        }
        
        # 检查CMakeLists.txt文件
        status["client_cmake_exists"] = (CLIENT_DIR / "CMakeLists.txt").exists()
        status["server_cmake_exists"] = (SERVER_DIR / "CMakeLists.txt").exists()
        
        return {"success": True, "status": status}
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def fix_include_paths() -> Dict[str, str]:
    """修复头文件包含路径问题"""
    try:
        fixed_files = []
        
        # 查找所有需要修复的源文件
        for root, dirs, files in os.walk(CLIENT_DIR):
            for file in files:
                if file.endswith('.cpp'):
                    file_path = Path(root) / file
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                    
                    # 修复包含路径
                    original_content = content
                    content = content.replace('#include "include/', '#include "')
                    
                    if content != original_content:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(content)
                        fixed_files.append(str(file_path))
        
        return {
            "success": True, 
            "message": f"Fixed include paths in {len(fixed_files)} files",
            "fixed_files": fixed_files
        }
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def create_missing_files() -> Dict[str, str]:
    """创建缺失的文件"""
    try:
        created_files = []
        
        # 检查并创建必要的目录
        dirs_to_create = [
            CLIENT_DIR / "assets" / "ui" / "styles",
            CLIENT_DIR / "assets" / "images",
            CLIENT_DIR / "assets" / "sounds",
            CLIENT_DIR / "logs"
        ]
        
        for dir_path in dirs_to_create:
            dir_path.mkdir(parents=True, exist_ok=True)
            created_files.append(f"Directory: {dir_path}")
        
        # 创建默认样式文件
        style_file = CLIENT_DIR / "assets" / "ui" / "styles" / "main.qss"
        if not style_file.exists():
            default_style = """
/* Fantasy Legend 默认样式 */
QMainWindow {
    background-color: #2a2a2a;
    color: #ffffff;
}

QMenuBar {
    background-color: #3a3a3a;
    color: #ffffff;
    border-bottom: 1px solid #555555;
}

QPushButton {
    background-color: #4a4a4a;
    color: white;
    border: 2px solid #666666;
    border-radius: 5px;
    padding: 8px 16px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #666666;
    border-color: #888888;
}

QPushButton:pressed {
    background-color: #333333;
    border-color: #555555;
}
"""
            with open(style_file, 'w', encoding='utf-8') as f:
                f.write(default_style)
            created_files.append(f"File: {style_file}")
        
        return {
            "success": True,
            "message": f"Created {len(created_files)} items",
            "created_items": created_files
        }
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def interactive_feedback(
    project_directory: Annotated[str, Field(description="项目目录路径")],
    summary: Annotated[str, Field(description="变更摘要")],
) -> Dict[str, str]:
    """请求用户交互反馈"""
    try:
        # 使用现有的feedback_ui.py
        feedback_ui_path = PROJECT_ROOT / "temp" / "cursor_MCP" / "interactive-feedback-mcp" / "feedback_ui.py"
        
        if not feedback_ui_path.exists():
            return {
                "success": False, 
                "error": f"Feedback UI not found at {feedback_ui_path}"
            }
        
        # 创建临时文件用于结果
        with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as tmp:
            output_file = tmp.name
        
        try:
            # 运行feedback_ui.py
            args = [
                sys.executable,
                "-u",
                str(feedback_ui_path),
                "--project-directory", project_directory,
                "--prompt", summary,
                "--output-file", output_file
            ]
            
            result = subprocess.run(
                args,
                check=False,
                shell=False,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL,
                close_fds=True
            )
            
            if result.returncode != 0:
                return {"success": False, "error": f"Feedback UI failed with code {result.returncode}"}
            
            # 读取结果
            with open(output_file, 'r') as f:
                feedback_result = json.load(f)
            
            os.unlink(output_file)
            return {"success": True, "feedback": feedback_result}
            
        except Exception as e:
            if os.path.exists(output_file):
                os.unlink(output_file)
            return {"success": False, "error": str(e)}
            
    except Exception as e:
        return {"success": False, "error": str(e)}

if __name__ == "__main__":
    mcp.run(transport="stdio") 