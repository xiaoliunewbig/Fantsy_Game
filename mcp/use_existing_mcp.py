#!/usr/bin/env python3
"""
直接使用现有的 interactive-feedback-mcp
"""

import sys
import os
from pathlib import Path

# 添加现有的MCP路径
existing_mcp_path = Path(__file__).parent.parent / "temp" / "cursor_MCP" / "interactive-feedback-mcp"
sys.path.insert(0, str(existing_mcp_path))

# 导入并运行现有的MCP服务器
from server import mcp

if __name__ == "__main__":
    mcp.run(transport="stdio") 