#ifndef NAVICORE_SESSION_H
#define NAVICORE_SESSION_H

#include <dbus-c++/dbus.h>
#include "genivi-navicore-session-proxy.h"
#include <stdio.h>

class NavicoreSession :
  public org::genivi::navigationcore::Session_proxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{
public:
    NavicoreSession(DBus::Connection &connection, const char *path, const char *name)
        : DBus::ObjectProxy(connection, path, name)
    {
    };

    void SessionDeleted(const uint32_t& sessionHandle)
    {
        printf("NavicoreSession - Session %d deleted\n",sessionHandle);
    };
};

#endif
