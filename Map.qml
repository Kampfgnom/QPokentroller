import QtQuick 2.4
import QtLocation 5.7
import QtPositioning 5.7

Item {
    width: 400
    height: 400

    Plugin {
        id: myPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.host"; value: "https://maps.wikimedia.org/osm-intl/" }
    }
    ListView {
        id: listview
        anchors.fill: parent
        spacing: 10
        model: routeModel.status == RouteModel.Ready ? routeModel.get(0).segments : null
        visible: model ? true : false
        delegate: Row {
            width: parent.width
            spacing: 10
            property bool hasManeuver : modelData.maneuver && modelData.maneuver.valid
            visible: hasManeuver
            Text { text: (1 + index) + "." }
            Text { text: hasManeuver ? modelData.maneuver.instructionText : "" }
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: myPlugin
        zoomLevel: 13
        activeMapType: supportedMapTypes[7]
        center: controller.currentCoordinate

        RouteModel {
            id: routeModel
            plugin : map.plugin
            query:  RouteQuery {
                id: routeQuery
            }
            onStatusChanged: {
                if (status == RouteModel.Ready) {
                    switch (count) {
                    case 0:
                        // technically not an error
                        map.routeError()
                        break
                    case 1:
                        map.update()
                        break
                    }
                } else if (status == RouteModel.Error) {
                    map.routeError()
                }
            }
        }

        MapItemView {
            model: routeModel
            delegate: routeDelegate
        }

        Component {
            id: routeDelegate

            MapRoute {
                route: routeModel
                line.color: "blue"
                line.width: 5
                smooth: true
                opacity: 0.8
            }
        }

        MapCircle {
            center: controller.currentCoordinate
            radius: 20.0
            color: 'green'
            border.width: 3
            smooth: true
            opacity: 0.8
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (mouse.button != Qt.LeftButton) {
                    return
                }

                controller.destinationCoorinate = map.toCoordinate(Qt.point(mouse.x, mouse.y))
            }

            onPressAndHold: {
                if (mouse.button != Qt.LeftButton) {
                    return
                }

                var coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y))

                // clear away any old data in the query
                routeQuery.clearWaypoints();

                // add the start and end coords as waypoints on the route
                routeQuery.addWaypoint(map.center)
                routeQuery.addWaypoint(coordinate)
                routeQuery.travelModes = RouteQuery.CarTravel
                routeQuery.routeOptimizations = RouteQuery.FastestRoute

                routeModel.update();
            }
        }
    }
}
