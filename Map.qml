import QtQuick 2.4
import QtLocation 5.7
import QtPositioning 5.7

Item {
    Plugin {
        id: myPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.host"; value: "https://maps.wikimedia.org/osm-intl/" }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: myPlugin
        zoomLevel: 13
        activeMapType: supportedMapTypes[7]

        Connections {
            target: controller
            onCurrentCoordinateChanged: {
                marker.center = controller.currentCoordinate
            }

            onCenterMapOnCurrentCoordinate: {
                map.center = controller.currentCoordinate
            }
        }

        MapCircle {
            id: marker
            center: controller.currentCoordinate
            radius: 20.0
            color: 'green'
            border.width: 3
            smooth: true
            opacity: 0.8
        }

        MapPolyline {
            id: pathLine
            line.width: 3
            line.color: 'blue'
            Connections {
                target: controller

                onPathChanged: {
                    pathLine.path = controller.path
                }
            }

        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                if (mouse.modifiers & Qt.ShiftModifier) {
                    controller.appendCalculatedRouteToPoint(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                } else {
                    controller.calculateRouteToPoint(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                }
            }
        }
    }
}
