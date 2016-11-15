#include <dbus-c++/dbus.h>
#include "GeniviWrapper.h"
#include "include/navicore.h"
#include "include/mapviewer.h"
#include "include/genivi-navicore-constants.h"

#include "../traces.h"
//#define TRACE_DEBUG(fmt, args...) do { fprintf(stderr, "[%s:%d] DEBUG: " fmt "\n", __func__, __LINE__, ##args); } while(0)
//#define TRACE_DEBUG(fmt, args...)

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

GeniviWrapper::GeniviWrapper()
{
    static DBus::BusDispatcher dispatcher;
    DBus::default_dispatcher = &dispatcher;
    
    DBus::Connection conn = DBus::Connection::SessionBus();
    navicore = new Navicore(conn, "/org/genivi/navicore", "org.agl.gpsnavi");
    mapviewer = new MapViewer(conn, "/org/genivi/mapviewer", "org.agl.gpsnavi");
}

GeniviWrapper::~GeniviWrapper()
{
    delete (MapViewer*)mapviewer;
    delete (Navicore*)navicore;
}

uint32_t GeniviWrapper::NavicoreCreateSession(const std::string& client)
{
    TRACE_DEBUG("client: %s", client.c_str());
    return ((Navicore*)navicore)->CreateSession(client);
}

void GeniviWrapper::NavicoreDeleteSession(const uint32_t& sessionHandle)
{
    TRACE_DEBUG("session: %" PRIu32, sessionHandle);
    return ((Navicore*)navicore)->DeleteSession(sessionHandle);
}

std::map<uint32_t, std::string> GeniviWrapper::NavicoreGetAllSessions()
{
    std::map<uint32_t, std::string> ret;
    std::vector< ::DBus::Struct< uint32_t, std::string > > ncAllSessions;
    std::vector< ::DBus::Struct< uint32_t, std::string > >::iterator it;

    TRACE_DEBUG(" ");
    ncAllSessions = ((Navicore*)navicore)->GetAllSessions();
    for (it = ncAllSessions.begin(); it != ncAllSessions.end(); it++)
    {
        ret[it->_1] = it->_2;
    }
    return ret;
}

Version GeniviWrapper::NavicoreSessionGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((Navicore*)navicore)->SessionGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

Version GeniviWrapper::NavicoreRoutingGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((Navicore*)navicore)->RoutingGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

Version GeniviWrapper::NavicoreGuidanceGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((Navicore*)navicore)->GuidanceGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

uint32_t GeniviWrapper::NavicoreCreateRoute(const uint32_t& sessionHandle)
{
    TRACE_DEBUG("session: %" PRIu32, sessionHandle);
    return ((Navicore*)navicore)->CreateRoute(sessionHandle);
}

void GeniviWrapper::NavicoreDeleteRoute(const uint32_t& sessionHandle, const uint32_t& routeHandle)
{
    TRACE_DEBUG("session: %" PRIu32 ", route: %" PRIu32, sessionHandle, routeHandle);
    ((Navicore*)navicore)->DeleteRoute(sessionHandle, routeHandle);
}

std::vector< uint32_t > GeniviWrapper::NavicoreGetAllRoutes()
{
    TRACE_DEBUG(" ");
    return ((Navicore*)navicore)->GetAllRoutes();
}

void GeniviWrapper::NavicoreSetWaypoints(const uint32_t& sessionHandle, const uint32_t& routeHandle,
    const bool& startFromCurrentPosition, const std::vector<Waypoint>& waypointsList)
{
    std::vector<Waypoint>::const_iterator it;
    std::vector< std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > > > wpl;

    TRACE_DEBUG("session: %" PRIu32 ", route: %" PRIu32 ", startFromCurrentPosition: %d",
        sessionHandle, routeHandle, startFromCurrentPosition);

    for (it = waypointsList.begin(); it != waypointsList.end(); it++)
    {
        std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > > Point;
        ::DBus::Struct< uint8_t, ::DBus::Variant > VarLat, VarLon;

        VarLat._1 = NAVICORE_LATITUDE;
        VarLat._2.writer().append_double(std::get<0>(*it));
        
        VarLon._1 = NAVICORE_LONGITUDE;
        VarLon._2.writer().append_double(std::get<1>(*it));

        Point[NAVICORE_LATITUDE] = VarLat;
        Point[NAVICORE_LONGITUDE] = VarLon;

        wpl.push_back(Point);
    }
    
    ((Navicore*)navicore)->SetWaypoints(sessionHandle, routeHandle, startFromCurrentPosition, wpl);
}

void GeniviWrapper::NavicoreGetWaypoints(const uint32_t& routeHandle, bool& startFromCurrentPosition, std::vector<Waypoint>& waypointsList)
{
    std::vector< std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > > >::iterator map;
    std::vector< std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > > > wpl;
    
    TRACE_DEBUG("route: %" PRIu32, routeHandle);
    
    ((Navicore*)navicore)->GetWaypoints(routeHandle, startFromCurrentPosition, wpl);

    for (map = wpl.begin(); map != wpl.end(); map++)
    {
        double lat = 0.0, lon = 0.0;
        std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > >::iterator it;

        for (it = map->begin(); it != map->end(); it++)
        {
            if (it->first == NAVICORE_LATITUDE || it->second._1 == NAVICORE_LATITUDE)
            {
                lat = it->second._2.reader().get_double();
            }
            else if (it->first == NAVICORE_LONGITUDE || it->second._1 == NAVICORE_LONGITUDE)
            {
                lon = it->second._2.reader().get_double();
            }
        }

        waypointsList.push_back(Waypoint(lat, lon));
        TRACE_DEBUG("\tfound point (%f, %f)", lat, lon);
    }
}

void GeniviWrapper::NavicoreCalculateRoute(const uint32_t& sessionHandle, const uint32_t& routeHandle)
{
    TRACE_DEBUG("session: %" PRIu32 ", route: %" PRIu32, sessionHandle, routeHandle);
    ((Navicore*)navicore)->CalculateRoute(sessionHandle, routeHandle);
}

void GeniviWrapper::NavicoreCancelRouteCalculation(const uint32_t& sessionHandle, const uint32_t& routeHandle)
{
    TRACE_DEBUG("session: %" PRIu32 ", route: %" PRIu32, sessionHandle, routeHandle);
    ((Navicore*)navicore)->CancelRouteCalculation(sessionHandle, routeHandle);
}

void GeniviWrapper::NavicoreSetSimulationMode(const uint32_t& sessionHandle, const bool& activate)
{
    TRACE_DEBUG("session: %" PRIu32 ", activation: %d", sessionHandle, activate);
    ((Navicore*)navicore)->SetSimulationMode(sessionHandle, activate);
}

void GeniviWrapper::NavicoreStartSimulation(const uint32_t& sessionHandle)
{
    TRACE_DEBUG("session: %" PRIu32, sessionHandle);
    ((Navicore*)navicore)->StartSimulation(sessionHandle);
}

void GeniviWrapper::NavicorePauseSimulation(const uint32_t& sessionHandle)
{
    TRACE_DEBUG("session: %" PRIu32, sessionHandle);
    ((Navicore*)navicore)->PauseSimulation(sessionHandle);
}

std::map< int32_t, variant > GeniviWrapper::NavicoreGetPosition(const std::vector< int32_t >& valuesToReturn)
{
    TRACE_DEBUG(" ");
    std::map< int32_t, variant > ret;
    std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > >::iterator it;
    std::map< int32_t, ::DBus::Struct< uint8_t, ::DBus::Variant > > PosList =
        ((Navicore*)navicore)->GetPosition(valuesToReturn);
    for (it = PosList.begin(); it != PosList.end(); it++)
    {
        if (it->first == NAVICORE_LATITUDE || it->second._1 == NAVICORE_LATITUDE)
            ret[it->first]._double = it->second._2.reader().get_double();
        else if (it->first == NAVICORE_LONGITUDE || it->second._1 == NAVICORE_LONGITUDE)
            ret[it->first]._double = it->second._2.reader().get_double();
        else if (it->first == NAVICORE_TIMESTAMP || it->second._1 == NAVICORE_TIMESTAMP)
            ret[it->first]._uint32_t = it->second._2.reader().get_uint32();
        else if (it->first == NAVICORE_HEADING || it->second._1 == NAVICORE_HEADING)
            ret[it->first]._uint32_t = it->second._2.reader().get_uint32();
        else if (it->first == NAVICORE_SPEED || it->second._1 == NAVICORE_SPEED)
            ret[it->first]._int32_t = it->second._2.reader().get_int32();
        else if (it->first == NAVICORE_SIMULATION_MODE || it->second._1 == NAVICORE_SIMULATION_MODE)
            ret[it->first]._bool = it->second._2.reader().get_bool();
    }

    return ret;
}

Version GeniviWrapper::MapviewerSessionGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((MapViewer*)mapviewer)->SessionGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

uint32_t GeniviWrapper::MapviewerCreateSession(const std::string& client)
{
    TRACE_DEBUG("client: %s", client.c_str());
    return ((MapViewer*)mapviewer)->CreateSession(client);
}

void GeniviWrapper::MapviewerDeleteSession(const uint32_t& sessionHandle)
{
    TRACE_DEBUG("session: %" PRIu32, sessionHandle);
    return ((MapViewer*)mapviewer)->DeleteSession(sessionHandle);
}

std::map<uint32_t, std::string> GeniviWrapper::MapviewerGetAllSessions()
{
    std::map<uint32_t, std::string> ret;
    std::vector< ::DBus::Struct< uint32_t, std::string > > mvAllSessions;
    std::vector< ::DBus::Struct< uint32_t, std::string > >::iterator it;

    TRACE_DEBUG(" ");
    mvAllSessions = ((MapViewer*)mapviewer)->GetAllSessions();
    for (it = mvAllSessions.begin(); it != mvAllSessions.end(); it++)
    {
        ret[it->_1] = it->_2;
    }
    return ret;
}

Version GeniviWrapper::MapviewerConfigurationGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((MapViewer*)mapviewer)->ConfigurationGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

Version GeniviWrapper::MapviewerControlGetVersion()
{
    TRACE_DEBUG(" ");
    ::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > v =
        ((MapViewer*)mapviewer)->MapViewerControlGetVersion();
    return Version(v._1, v._2, v._3, v._4);
}

uint32_t GeniviWrapper::MapviewerCreateMapViewInstance(const uint32_t& sessionHandle, const std::tuple< uint16_t, uint16_t >& mapViewSize, const int32_t& mapViewType)
{
    ::DBus::Struct< uint16_t, uint16_t > dbusMapViewSize;
    dbusMapViewSize._1 = std::get<0>(mapViewSize);
    dbusMapViewSize._2 = std::get<1>(mapViewSize);

    TRACE_DEBUG("session: %" PRIu32 ", size: %" PRIu16 "x%" PRIu16 ", type: %" PRId32,
        sessionHandle, dbusMapViewSize._1, dbusMapViewSize._2, mapViewType);
    return ((MapViewer*)mapviewer)->CreateMapViewInstance(sessionHandle, dbusMapViewSize, mapViewType);
}

void GeniviWrapper::MapviewerReleaseMapViewInstance(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle)
{
    TRACE_DEBUG("session: %" PRIu32 ", instance: %" PRIu32, sessionHandle, mapViewInstanceHandle);
    return ((MapViewer*)mapviewer)->ReleaseMapViewInstance(sessionHandle, mapViewInstanceHandle);
}

void GeniviWrapper::MapviewerSetFollowCarMode(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const bool& followCarMode)
{
    TRACE_DEBUG("session: %" PRIu32 ", instance: %" PRIu32 ", mode: %d",
        sessionHandle, mapViewInstanceHandle, followCarMode);
    return ((MapViewer*)mapviewer)->SetFollowCarMode(sessionHandle, mapViewInstanceHandle, followCarMode);
}

bool GeniviWrapper::MapviewerGetFollowCarMode(const uint32_t& mapViewInstanceHandle)
{
    TRACE_DEBUG("instance: %" PRIu32, mapViewInstanceHandle);
    return ((MapViewer*)mapviewer)->GetFollowCarMode(mapViewInstanceHandle);
}

void GeniviWrapper::MapviewerSetCameraHeadingAngle(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const int32_t& heading)
{
    TRACE_DEBUG("session: %" PRIu32 ", instance: %" PRIu32 ", mode: %" PRId32,
        sessionHandle, mapViewInstanceHandle, heading);
    return ((MapViewer*)mapviewer)->SetCameraHeadingAngle(sessionHandle, mapViewInstanceHandle, heading);
}

void GeniviWrapper::MapviewerSetCameraHeadingTrackUp(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle)
{
    TRACE_DEBUG("session: %" PRIu32 "instance: %" PRIu32, sessionHandle, mapViewInstanceHandle);
    return ((MapViewer*)mapviewer)->SetCameraHeadingTrackUp(sessionHandle, mapViewInstanceHandle);
}

std::vector<uint16_t> GeniviWrapper::MapviewerGetScaleList(const uint32_t& mapViewInstanceHandle)
{
    std::vector<uint16_t> Ret;
    std::vector< ::DBus::Struct< uint16_t, uint16_t, int32_t, uint32_t > > ScaleList;
    std::vector< ::DBus::Struct< uint16_t, uint16_t, int32_t, uint32_t > >::iterator it;

    TRACE_DEBUG("instance: %" PRIu32, mapViewInstanceHandle);
    ScaleList = ((MapViewer*)mapviewer)->GetScaleList(mapViewInstanceHandle);

    for (it = ScaleList.begin(); it != ScaleList.end(); it++)
    {
        Ret.push_back(it->_1);
    }
    return Ret;
}

void GeniviWrapper::MapviewerSetMapViewScale(const uint32_t& sessionHandle, const uint32_t& mapViewInstanceHandle, const uint16_t& scaleID)
{
    TRACE_DEBUG("session: %" PRIu32 ", instance: %" PRIu32 ", scaleID: %" PRIu16,
        sessionHandle, mapViewInstanceHandle, scaleID);
    ((MapViewer*)mapviewer)->SetMapViewScale(sessionHandle, mapViewInstanceHandle, scaleID);
}

void GeniviWrapper::MapviewerGetMapViewScale(const uint32_t& mapViewInstanceHandle, uint8_t& scaleID, int32_t& isMinMax)
{
    TRACE_DEBUG("session: %" PRIu32, mapViewInstanceHandle);
    ((MapViewer*)mapviewer)->GetMapViewScale(mapViewInstanceHandle, scaleID, isMinMax);
}

std::vector< uint32_t > GeniviWrapper::MapviewerDisplayCustomElements(const uint32_t& sessionHandle,
    const uint32_t& mapViewInstanceHandle,
    const std::vector< std::tuple< std::string, std::string, std::tuple< double, double >, std::tuple< int16_t, int16_t > > >& customElements)
{
    std::vector< std::tuple< std::string, std::string, std::tuple< double, double >, std::tuple< int16_t, int16_t > > >::const_iterator it;
    std::vector< ::DBus::Struct< std::string, std::string, ::DBus::Struct< double, double >, ::DBus::Struct< int16_t, int16_t > > > CE;

    TRACE_DEBUG("session: %" PRIu32 ", instance: %" PRIu32, sessionHandle, mapViewInstanceHandle);

    for (it = customElements.begin(); it != customElements.end(); it++)
    {
        ::DBus::Struct< std::string, std::string, ::DBus::Struct< double, double >, ::DBus::Struct< int16_t, int16_t > > Element;

        // name
        Element._1 = std::string(std::get<0>(*it));

        // uri
        Element._2 = std::string(std::get<1>(*it));

        // anchor
        Element._3._1 = std::get<0>( std::get<2>(*it) );
        Element._3._2 = std::get<1>( std::get<2>(*it) );

        Element._4._1 = std::get<0>( std::get<3>(*it) );
        Element._4._2 = std::get<1>( std::get<3>(*it) );

        CE.push_back(Element);
    }

    return ((MapViewer*)mapviewer)->DisplayCustomElements(sessionHandle, mapViewInstanceHandle, CE);
}
