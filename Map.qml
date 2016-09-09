import QtQuick 2.4
import QtLocation 5.7
import QtPositioning 5.7

Item {
    Plugin {
        id: myPlugin
        name: "googlemaps"
        PluginParameter { name: "googlemaps.maps.apikey"; value: <YOU KEY HERE> }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: myPlugin
        zoomLevel: 13

        Connections {
            target: movementController
            onCurrentCoordinateChanged: {
                marker.center = movementController.currentCoordinate
            }

            onCenterMapOnCurrentCoordinate: {
                map.center = movementController.currentCoordinate
            }
        }

        MapCircle {
            id: marker
            center: movementController.currentCoordinate
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
                target: movementController

                onPathChanged: {
                    pathLine.path = movementController.path
                }
            }

        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                if (mouse.modifiers & Qt.ShiftModifier) {
                    movementController.appendCalculatedRouteToPoint(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                } else {
                    movementController.calculateRouteToPoint(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                }
            }
        }
    }
}
