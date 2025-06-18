#!/usr/bin/env python3
"""
Fantasy Legend Simple MCP Server
直接使用现有的 interactive-feedback-mcp
"""

import os
import sys
import json
import tempfile
import subprocess
from pathlib import Path
from typing import Annotated, Dict

from fastmcp import FastMCP
from pydantic import Field

# 创建MCP服务器实例
mcp = FastMCP("Fantasy Legend Simple MCP", log_level="INFO")

# 项目根目录
PROJECT_ROOT = Path(__file__).parent.parent
FEEDBACK_UI_PATH = PROJECT_ROOT / "temp" / "cursor_MCP" / "interactive-feedback-mcp" / "feedback_ui.py"

def launch_feedback_ui(project_directory: str, summary: str) -> dict[str, str]:
    """启动反馈UI"""
    # 创建临时文件用于结果
    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as tmp:
        output_file = tmp.name

    try:
        # 运行feedback_ui.py
        args = [
            sys.executable,
            "-u",
            str(FEEDBACK_UI_PATH),
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
            raise Exception(f"Feedback UI failed with code {result.returncode}")

        # 读取结果
        with open(output_file, 'r') as f:
            result = json.load(f)
        os.unlink(output_file)
        return result
        
    except Exception as e:
        if os.path.exists(output_file):
            os.unlink(output_file)
        raise e

def first_line(text: str) -> str:
    """获取第一行"""
    return text.split("\n")[0].strip()

@mcp.tool()
def interactive_feedback(
    project_directory: Annotated[str, Field(description="项目目录路径")],
    summary: Annotated[str, Field(description="变更摘要")],
) -> Dict[str, str]:
    """请求用户交互反馈"""
    try:
        if not FEEDBACK_UI_PATH.exists():
            return {
                "success": False,
                "error": f"Feedback UI not found at {FEEDBACK_UI_PATH}"
            }
        
        result = launch_feedback_ui(first_line(project_directory), first_line(summary))
        return {"success": True, "feedback": result}
        
    except Exception as e:
        return {"success": False, "error": str(e)}

@mcp.tool()
def get_project_info() -> Dict[str, str]:
    """获取项目信息"""
    try:
        info = {
            "project_root": str(PROJECT_ROOT),
            "feedback_ui_path": str(FEEDBACK_UI_PATH),
            "feedback_ui_exists": FEEDBACK_UI_PATH.exists(),
            "client_dir": str(PROJECT_ROOT / "client"),
            "server_dir": str(PROJECT_ROOT / "server")
        }
        return {"success": True, "info": info}
    except Exception as e:
        return {"success": False, "error": str(e)}

if __name__ == "__main__":
    mcp.run(transport="stdio") 