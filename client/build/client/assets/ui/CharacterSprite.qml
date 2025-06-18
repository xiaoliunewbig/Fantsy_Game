Rectangle {
    id: characterSprite
    width: 64
    height: 64
    radius: 32
    color: character.isAlive ? "#4CAF50" : "#F44336"
    
    property var character
    
    Text {
        anchors.centerIn: parent
        text: character.name
        color: "white"
        font.pixelSize: 12
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: characterSprite.clicked()
    }
    
    signal clicked()
}