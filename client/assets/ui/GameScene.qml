import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: gameScene
    anchors.fill: parent
    
    // 游戏区域
    Rectangle {
        id: gameArea
        anchors.fill: parent
        anchors.bottomMargin: 200
        color: "#2a2a2a"
        
        // 角色显示
        Repeater {
            model: characterList
            delegate: CharacterSprite {
                character: modelData
                onClicked: selectTarget(character)
            }
        }
        
        // 特效层
        EffectLayer {
            id: effectLayer
            anchors.fill: parent
        }
    }
    
    // 底部UI
    Rectangle {
        id: bottomUI
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 200
        color: "#1a1a1a"
        
        // 技能栏
        SkillBar {
            id: skillBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 80
        }
        
        // 角色状态
        Row {
            anchors.top: skillBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 10
            
            // 玩家状态
            CharacterStatus {
                character: playerCharacter
                width: 200
            }
            
            // 敌人状态
            Repeater {
                model: enemyList
                delegate: CharacterStatus {
                    character: modelData
                    width: 150
                }
            }
        }
    }
}