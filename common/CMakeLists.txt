cmake_minimum_required(VERSION 3.16)
project(FantasyLegend_Common VERSION 1.0.0 LANGUAGES CXX)

# 设置源文件
set(SOURCES
    src/protocol/Messages.cpp
    src/models/CharacterModel.cpp
    src/models/GameStateModel.cpp
    src/utils/JsonUtils.cpp
    src/utils/StringUtils.cpp
    src/utils/TimeUtils.cpp
)

set(HEADERS
    include/protocol/MessageTypes.h
    include/protocol/GameMessages.h
    include/protocol/NetworkProtocol.h
    include/models/CharacterData.h
    include/models/SkillData.h
    include/models/ItemData.h
    include/utils/JsonUtils.h
    include/utils/StringUtils.h
    include/utils/TimeUtils.h
)

# 创建静态库
add_library(FantasyLegend_Common STATIC ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(FantasyLegend_Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 导出包含目录
set(FANTASY_LEGEND_COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE) 