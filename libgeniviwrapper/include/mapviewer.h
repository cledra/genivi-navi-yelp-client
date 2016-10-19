#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <dbus-c++/dbus.h>
#include "genivi-mapviewer-proxy.h"
#include <stdio.h>

class MapViewer :
  public org::genivi::mapviewer::Session_proxy,
  public org::genivi::mapviewer::Configuration_proxy,
  public org::genivi::mapviewer::MapViewerControl_proxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{
public:
    MapViewer(DBus::Connection &connection, const char *path, const char *name)
        : DBus::ObjectProxy(connection, path, name)
    {
    };

    // Session interface
    void SessionDeleted(const uint32_t& sessionHandle)
    {
        // TODO
    };

    // Configuration
    void ConfigurationChanged(const std::vector< int32_t >& changedSettings)
    {
        // TODO
    };

    // MapViewerControl
    void FollowCarModeChanged(const uint32_t& mapViewInstanceHandle, const bool& followCarMode)
    {
        // TODO
    };

    void CameraPositionChanged(const uint32_t& mapViewInstanceHandle, const ::DBus::Struct< double, double, double >& targetPoint)
    {
        // TODO
    };

    void CameraHeadingChanged(const uint32_t& mapViewInstanceHandle, const uint16_t& headingType, const int32_t& headingAngle)
    {
        // TODO
    };

    void CameraTiltAngleChanged(const uint32_t& mapViewInstanceHandle, const int32_t& tilt)
    {
        // TODO
    };

    void CameraDistanceFromTargetPointChanged(const uint32_t& mapViewInstanceHandle, const uint32_t& distance)
    {
        // TODO
    };

    void MapViewScaleChanged(const uint32_t& mapViewInstanceHandle, const uint8_t& scaleID, const int32_t& isMinMax)
    {
        // TODO
    };

    void MapViewPerspectiveChanged(const uint32_t& mapViewInstanceHandle, const uint16_t& perspective)
    {
        // TODO
    };

    void MapViewObjectVisibilityChanged(const uint32_t& mapViewInstanceHandle, const std::map< int32_t, bool >& objectVisibilityList)
    {
        // TODO
    };

    void MapViewBoundingBoxChanged(const uint32_t& mapViewInstanceHandle, const ::DBus::Struct< ::DBus::Struct< double, double >, ::DBus::Struct< double, double > >& boundingBox)
    {
        // TODO
    };

    void MapViewSaveAreaChanged(const uint32_t& mapViewInstanceHandle, const ::DBus::Struct< double, double, double, double >& saveArea)
    {
        // TODO
    };

    void MapViewVisibilityChanged(const uint32_t& mapViewInstanceHandle, const int32_t& visibilityMode)
    {
        // TODO
    };

    void MapViewPerformanceLevelChanged(const uint32_t& mapViewInstanceHandle, const int32_t& performanceLevel)
    {
        // TODO
    };

    void DisplayedRoutes(const uint32_t& mapViewInstanceHandle, const std::vector< ::DBus::Struct< uint32_t, bool > >& displayedRoutes)
    {
        // TODO
    };

    void PoiCategoriesVisibilityChanged(const uint32_t& mapViewInstanceHandle, const std::vector< uint32_t >& poiCategoryIds, const bool& visible, const uint8_t& minScaleID, const uint8_t& maxScaleID)
    {
        // TODO
    };

    void MapViewThemeChanged(const uint32_t& mapViewInstanceHandle, const int32_t& mapViewTheme)
    {
        // TODO
    };

};

#endif
