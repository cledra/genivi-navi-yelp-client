#ifndef GENIVI_WRAPPER_H
#define GENIVI_WRAPPER_H

#include <map>
#include <tuple>
#include <vector>

typedef std::tuple<uint16_t, uint16_t, uint16_t, std::string> Version;
typedef std::tuple<double, double> Waypoint;

typedef union
{
    bool _bool;
    int32_t _int32_t;
    uint32_t _uint32_t;
    double _double;
} variant;

class GeniviWrapper
{
public:
    GeniviWrapper();
    ~GeniviWrapper();
    
    Version NavicoreSessionGetVersion();
    uint32_t NavicoreCreateSession(const std::string& client);
    void NavicoreDeleteSession(const uint32_t& sessionHandle);
    //int32_t NavicoreGetSessionStatus(const uint32_t& sessionHandle);
    std::map<uint32_t, std::string> NavicoreGetAllSessions();

    Version NavicoreRoutingGetVersion();
    uint32_t NavicoreCreateRoute(const uint32_t& sessionHandle);
    void NavicoreDeleteRoute(const uint32_t& sessionHandle, const uint32_t& routeHandle);
    std::vector< uint32_t > NavicoreGetAllRoutes();
    void NavicoreSetWaypoints(const uint32_t& sessionHandle, const uint32_t& routeHandle, const bool& startFromCurrentPosition, const std::vector<Waypoint>& waypointsList);
    void NavicoreGetWaypoints(const uint32_t& routeHandle, bool& startFromCurrentPosition, std::vector<Waypoint>& waypointsList);
    void NavicoreCalculateRoute(const uint32_t& sessionHandle, const uint32_t& routeHandle);
    void NavicoreCancelRouteCalculation(const uint32_t& sessionHandle, const uint32_t& routeHandle);

    //Version NavicorePositionGetVersion();
    void NavicoreSetSimulationMode(const uint32_t& sessionHandle, const bool& activate);
    int32_t NavicoreGetSimulationStatus();
    void NavicoreStartSimulation(const uint32_t& sessionHandle);
    void NavicorePauseSimulation(const uint32_t& sessionHandle);
    std::map< int32_t, variant > NavicoreGetPosition(const std::vector< int32_t >& valuesToReturn);
    
    Version NavicoreGuidanceGetVersion();
    
    Version MapviewerSessionGetVersion();
    uint32_t MapviewerCreateSession(const std::string& client);
    void MapviewerDeleteSession(const uint32_t& sessionHandle);
    //int32_t MapviewerGetSessionStatus(const uint32_t& sessionHandle);
    std::map<uint32_t, std::string> MapviewerGetAllSessions();

    Version MapviewerConfigurationGetVersion();

    Version MapviewerControlGetVersion();
    uint32_t MapviewerCreateMapViewInstance(const uint32_t& sessionHandle, const std::tuple< uint16_t, uint16_t >& mapViewSize, const int32_t& mapViewType);
    void MapviewerReleaseMapViewInstance(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle);
    void MapviewerSetFollowCarMode(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const bool& followCarMode);
    bool MapviewerGetFollowCarMode(const uint32_t& mapViewInstanceHandle);
    void MapviewerSetCameraHeadingAngle(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const int32_t& heading);
    void MapviewerSetCameraHeadingTrackUp(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle);
    std::vector<uint16_t> MapviewerGetScaleList(const uint32_t& mapViewInstanceHandle);
    void MapviewerSetMapViewScale(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const uint16_t& scaleID);
    void MapviewerGetMapViewScale(const uint32_t& mapViewInstanceHandle, uint8_t& scaleID, int32_t& isMinMax);
    std::vector< uint32_t > MapviewerDisplayCustomElements(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const std::vector< std::tuple< std::string, std::string, std::tuple< double, double >, std::tuple< int16_t, int16_t > > >& customElements);

private:
    void * navicore;
    void * mapviewer;
};

#endif
