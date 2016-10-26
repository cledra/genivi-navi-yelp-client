#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QTreeWidget>
#include <iostream>
#include <error.h>
#include <json-c/json.h>
#include <stdlib.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "MainApp.h"
#include "Business.h"
#include "InfoPanel.h"
#include "libgeniviwrapper/GeniviWrapper.h"
#include "libgeniviwrapper/include/genivi-navicore-constants.h"
#include "traces.h"

#define DEFAULT_TEXT        "Select your destination with Yelp !"
#define URL_AUTH            "https://api.yelp.com/oauth2/token"
#define URL_AUTOCOMPLETE    "https://api.yelp.com/v3/autocomplete"
#define URL_SEARCH          "https://api.yelp.com/v3/businesses/search"

#define BIG_BUFFER_SIZE     (1024*1024)
#define FONT_SIZE_LINEDIT   20
#define FONT_SIZE_LIST      16
#define WIDGET_WIDTH        800

using namespace std;

MainApp::MainApp(int argc, char* argv[]):
    navicoreSession(0),mutex(),wrapper(),
    window(Q_NULLPTR, Qt::FramelessWindowHint),layout(&window),
    lineEdit(&window),token(""),networkManager(this),pSearchReply(NULL),
    pResultList(new QTreeWidget(&window)),infoPanel(NULL)
{
    window.setStyleSheet("background-color: rgba(235, 235, 235);");
    //window.setAttribute(Qt::WA_TranslucentBackground);

    lineEdit.setPlaceholderText(QString(DEFAULT_TEXT));
    QFont font = lineEdit.font();
    font.setPointSize(FONT_SIZE_LINEDIT);
    lineEdit.setFont(font);

    window.setLayout(&layout);
    layout.addWidget(&lineEdit, 0, Qt::AlignTop);

    pResultList->setColumnCount(2);
    pResultList->setUniformRowHeights(true);
    pResultList->setRootIsDecorated(false);
    pResultList->setEditTriggers(QTreeWidget::NoEditTriggers);
    pResultList->setSelectionBehavior(QTreeWidget::SelectRows);
    pResultList->setFrameStyle(QFrame::Box | QFrame::Plain);
    pResultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pResultList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pResultList->header()->hide();
    pResultList->setFont(font);
    pResultList->installEventFilter(this);
    
    Expand(false);

    window.show();
}

MainApp::~MainApp()
{
    mutex.lock();
    delete pSearchReply;
    delete pResultList;
    mutex.unlock();
}

void MainApp::UpdateAglSurfaces()
{
    char cmd[1024];

    TRACE_DEBUG("handle AGL demo surfaces...");
    snprintf(cmd, 1023, "/usr/bin/LayerManagerControl set surface $SURFACE_ID_CLIENT source region 0 0 %d %d",
        window.width(), window.height());
    TRACE_DEBUG("%s", cmd);
    system(cmd);
    snprintf(cmd, 1023, "/usr/bin/LayerManagerControl set surface $SURFACE_ID_CLIENT destination region $CLIENT_X $CLIENT_Y %d %d",
        window.width(), window.height());
    TRACE_DEBUG("%s", cmd);
    system(cmd);
}

void MainApp::Expand(bool expand)
{
    if (expand)
    {
        /* Make space to display the QLineEdit + the result list : */

        int h = pResultList->sizeHintForRow(0) * (Businesses.size() + 1);
        window.setGeometry(window.pos().x(), window.pos().y(), WIDGET_WIDTH, lineEdit.height() + h + 10);

        layout.addWidget(pResultList, 0, Qt::AlignTop);
        pResultList->show();

        pResultList->setCurrentItem(pResultList->topLevelItem(0));
        pResultList->resize(lineEdit.width(), h);
        
        pResultList->move(QPoint(lineEdit.pos().x(), lineEdit.pos().y() + lineEdit.height()));
        pResultList->setColumnWidth(0, pResultList->width() / 2);
        pResultList->setColumnWidth(1, pResultList->width() / 2);
        pResultList->setFocus();
    }
    else
    {
        /* Shrink space to only display the QLineEdit field : */

        pResultList->clear();
        pResultList->hide();
        layout.removeWidget(pResultList);
        window.setGeometry(window.pos().x(), window.pos().y(), WIDGET_WIDTH, -1);
        lineEdit.setFocus();
    }

    if (getenv("AGL_NAVI"))
    {
        QTimer timer(this);
        timer.singleShot(0, this, SLOT(UpdateAglSurfaces()));
    }
}

void MainApp::textChanged(const QString & text)
{
    double currentLatitude, currentLongitude;

    DisplayInformation(false);

    TRACE_INFO("New text is: %s", qPrintable(text));

    mutex.lock();

    delete pSearchReply;    /* cancel current search */
    pSearchReply = NULL;

    if (text.length() == 0) /* if empty text -> no search */
    {
        Expand(false);      /* shrink display to minimum */
        mutex.unlock();
        return;
    }

    /* we need to know our current position : */
    std::vector<int32_t> Params;
    Params.push_back(NAVICORE_LONGITUDE);
    Params.push_back(NAVICORE_LATITUDE);
    std::map< int32_t, variant > Ret = wrapper.NavicoreGetPosition(Params);
    std::map< int32_t, variant >::iterator it;
    for (it = Ret.begin(); it != Ret.end(); it++)
    {
        if (it->first == NAVICORE_LATITUDE)
            currentLatitude = it->second._double;
        else if (it->first == NAVICORE_LONGITUDE)
            currentLongitude = it->second._double;
    }

    TRACE_INFO("Current position: %f, %f", currentLatitude, currentLongitude);

    /* let's generate a search request : */
    QString myUrlStr = URL_SEARCH + QString("?") + QString("term=") + text +
        QString("&latitude=") + QString::number(currentLatitude) +
        QString("&longitude=") + QString::number(currentLongitude);

    TRACE_DEBUG("URL: %s", qPrintable(myUrlStr));

    QUrl myUrl = QUrl(myUrlStr);
    QNetworkRequest req(myUrl);
    req.setRawHeader("Authorization", (QString("bearer ") + token).toLocal8Bit());
    pSearchReply = networkManager.get(req);
    TRACE_DEBUG("after post, reply is %p", pSearchReply);
    
    mutex.unlock();
}

void MainApp::ParseJsonBusinessList(const char* buf, std::vector<Business> & Output)
{
    json_object *jobj = json_tokener_parse(buf);
    if (!jobj)
    {
        TRACE_ERROR("json_tokener_parse failed");
        return;
    }

    json_object_object_foreach(jobj, key, val)
    {
        (void)key;
        json_object *value;
        
        if (json_object_get_type(val) == json_type_array)
        {
            TRACE_DEBUG("an array was found");

            if(json_object_object_get_ex(jobj, "businesses", &value))
            {
                TRACE_DEBUG("an business was found");

                int arraylen = json_object_array_length(value);

                for (int i = 0; i < arraylen; i++)
                {
                    Business NewBusiness;

                    json_object* medi_array_obj, *medi_array_obj_elem;
                    medi_array_obj = json_object_array_get_idx(value, i);
                    if (medi_array_obj)
                    {
                        if (json_object_object_get_ex(medi_array_obj, "rating", &medi_array_obj_elem))
                        {
                            NewBusiness.Rating = json_object_get_double(medi_array_obj_elem);
                            free(medi_array_obj_elem);
                            TRACE_DEBUG("got Rating : %f", NewBusiness.Rating);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "review_count", &medi_array_obj_elem))
                        {
                            NewBusiness.ReviewCount = json_object_get_int(medi_array_obj_elem);
                            free(medi_array_obj_elem);
                            TRACE_DEBUG("got ReviewCount : %u", NewBusiness.ReviewCount);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "name", &medi_array_obj_elem))
                        {
                            NewBusiness.Name = QString(json_object_get_string(medi_array_obj_elem));
                            free(medi_array_obj_elem);
                            TRACE_DEBUG("got Name : %s", qPrintable(NewBusiness.Name));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "image_url", &medi_array_obj_elem))
                        {
                            NewBusiness.ImageUrl = QString(json_object_get_string(medi_array_obj_elem));
                            free(medi_array_obj_elem);
                            TRACE_DEBUG("got ImageUrl : %s", qPrintable(NewBusiness.ImageUrl));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "phone", &medi_array_obj_elem))
                        {
                            NewBusiness.Phone = QString(json_object_get_string(medi_array_obj_elem));
                            free(medi_array_obj_elem);
                            TRACE_DEBUG("got Phone : %s", qPrintable(NewBusiness.Phone));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "coordinates", &medi_array_obj_elem))
                        {
                            json_object *value2;
                            
                            TRACE_DEBUG("coordinates were found");

                            if(json_object_object_get_ex(medi_array_obj_elem, "latitude", &value2))
                            {
                                NewBusiness.Latitude = json_object_get_double(value2);
                                free(value2);
                                TRACE_DEBUG("got Latitude : %f", NewBusiness.Latitude);
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "longitude", &value2))
                            {
                                NewBusiness.Longitude = json_object_get_double(value2);
                                free(value2);
                                TRACE_DEBUG("got Longitude : %f", NewBusiness.Longitude);
                            }
                                    
                            free(medi_array_obj_elem);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "location", &medi_array_obj_elem))
                        {
                            json_object *value2;
                            
                            TRACE_DEBUG("a location was found");

                            /* TODO: how do we deal with address2 and address3 ? */
                            if(json_object_object_get_ex(medi_array_obj_elem, "address1", &value2))
                            {
                                NewBusiness.Address = QString(json_object_get_string(value2));
                                free(value2);
                                TRACE_DEBUG("got Address : %s", qPrintable(NewBusiness.Address));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "city", &value2))
                            {
                                NewBusiness.City = QString(json_object_get_string(value2));
                                free(value2);
                                TRACE_DEBUG("got City : %s", qPrintable(NewBusiness.City));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "state", &value2))
                            {
                                NewBusiness.State = QString(json_object_get_string(value2));
                                free(value2);
                                TRACE_DEBUG("got State : %s", qPrintable(NewBusiness.State));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "zip_code", &value2))
                            {
                                NewBusiness.ZipCode = QString(json_object_get_string(value2));
                                free(value2);
                                TRACE_DEBUG("got ZipCode : %s", qPrintable(NewBusiness.ZipCode));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "country", &value2))
                            {
                                NewBusiness.Country = QString(json_object_get_string(value2));
                                free(value2);
                                TRACE_DEBUG("got Country : %s", qPrintable(NewBusiness.Country));
                            }

                            free(medi_array_obj_elem);
                        }

                        /* TODO: parse categories */

                        /* Add business in our list: */
                        Businesses.push_back(NewBusiness);

                        free(medi_array_obj);
                    }
                }

                free(value);
            }
        }
    }

    free(jobj);
}

bool MainApp::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != pResultList)
    {
        TRACE_WARN("return false");
        return false;
    }

    TRACE_DEBUG("ev->type() = %d", (int)ev->type());

    if (ev->type() == QEvent::HideToParent)
    {
        /* that's no reason to select the text, is it ? */
        lineEdit.deselect();
    }

    if (ev->type() == QEvent::MouseButtonPress) {
        Expand(false);
        TRACE_DEBUG("mouse button");
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        TRACE_DEBUG("key pressed (%d)", key);
        switch (key) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                TRACE_DEBUG("enter or return");
                //SetDestination();
                consumed = true;

                /**/
                DisplayInformation();
                break;
                /**/

            case Qt::Key_Escape:
                TRACE_DEBUG("escape");
                Expand(false);
                consumed = true;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
                TRACE_DEBUG("arrows");
                break;

            default:
                TRACE_DEBUG("default");
                lineEdit.event(ev);
                Expand(false);
                break;
        }

        return consumed;
    }

    return false;
}

void MainApp::SetDestination()
{
    QList<QTreeWidgetItem *> SelectedItems = pResultList->selectedItems();
    if (SelectedItems.size() <= 0)
    {
        TRACE_ERROR("no item is selected");
        return;
    }

    /* select the first selected item : */
    int index = pResultList->indexOfTopLevelItem(*SelectedItems.begin());
    TRACE_DEBUG("index is: %d", index);

    /* retrieve the coordinates of this item : */
    double targetLatitude  = Businesses[index].Latitude;
    double targetLongitude = Businesses[index].Longitude;

    /* check if a route already exists, if not create it : */
    uint32_t myRoute;
    std::vector< uint32_t > allRoutes = wrapper.NavicoreGetAllRoutes();
    if (allRoutes.size() == 0)
    {
        myRoute = wrapper.NavicoreCreateRoute(navicoreSession);
        TRACE_INFO("Created route %" PRIu32, myRoute);
    }
    else
    {
        myRoute = allRoutes[0];
        wrapper.NavicorePauseSimulation(navicoreSession);
        wrapper.NavicoreSetSimulationMode(navicoreSession, false);
        wrapper.NavicoreCancelRouteCalculation(navicoreSession, myRoute);
        TRACE_INFO("Re-use route %" PRIu32, myRoute);
    }

    /* set the destination : */
    Waypoint destWp(targetLatitude, targetLongitude);
    std::vector<Waypoint> myWayPoints;
    myWayPoints.push_back(destWp);
    wrapper.NavicoreSetWaypoints(navicoreSession, myRoute, true, myWayPoints);

    wrapper.NavicoreCalculateRoute(navicoreSession, myRoute);
}

void MainApp::DisplayInformation(bool display)
{
    if (display)
    {
        QList<QTreeWidgetItem *> SelectedItems = pResultList->selectedItems();
        if (SelectedItems.size() <= 0)
        {
            TRACE_ERROR("no item is selected");
            return;
        }

        /* select the first selected item : */
        int index = pResultList->indexOfTopLevelItem(*SelectedItems.begin());
        TRACE_DEBUG("index is: %d", index);

        /* Resize window: */
        Expand(false);
        window.setGeometry(window.pos().x(), window.pos().y(), WIDGET_WIDTH, WIDGET_WIDTH);

        /* Display info for the selected item: */
        infoPanel = new InfoPanel(this, &layout, Businesses[index]);
    }
    else
    {
        delete infoPanel;
        infoPanel = NULL;
    }

    if (getenv("AGL_NAVI"))
    {
        QTimer timer(this);
        timer.singleShot(0, this, SLOT(UpdateAglSurfaces()));
    }
}

void MainApp::networkReplySearch(QNetworkReply* reply)
{
    char buf[BIG_BUFFER_SIZE];
    int buflen;
    
    mutex.lock();

    // we only handle this callback if it matches the last search request:
    if (reply != pSearchReply)
    {
        TRACE_INFO("this reply is already too late (or about a different network request)");
        mutex.unlock();
        return;
    }
    
    buflen = reply->read(buf, BIG_BUFFER_SIZE-1);

    /* empty our business list, and replace its content with the reply's content: */
    Businesses.clear();
    ParseJsonBusinessList(buf, Businesses);

    pResultList->clear();

    const QPalette &pal = lineEdit.palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    /* filling the dropdown menu: */
    pResultList->setUpdatesEnabled(false);
    for (vector<Business>::iterator it = Businesses.begin(); it != Businesses.end(); it++)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(pResultList);
        item->setText(0, (*it).Name);
        item->setText(1, (*it).Address + QString(", ") + (*it).City);
        item->setTextAlignment(1, Qt::AlignRight);
        item->setTextColor(1, color);
    }

    //pResultList->setCurrentItem(pResultList->topLevelItem(0));
    pResultList->setUpdatesEnabled(true);

    Expand(true);

    mutex.unlock();
}

int MainApp::CheckGeniviApi()
{
    map<uint32_t, std::string> NcSessions = wrapper.NavicoreGetAllSessions();
    if (NcSessions.empty())
    {
        TRACE_ERROR("Error: could not find an instance of Genivi Navicore");
        return -1;
    }

    navicoreSession = NcSessions.begin()->first;
    TRACE_INFO("Using Genivi Navicore session \"%s\" (%" PRIu32 ")",
        NcSessions.begin()->second.c_str(), NcSessions.begin()->first);

    return 0;
}

int MainApp::AuthenticatePOI(const QString & CredentialsFile)
{
    char buf[512];
    QString AppId;
    QString AppSecret;
    
    /* First, read AppId and AppSecret from credentials file: */
    FILE* filep = fopen(qPrintable(CredentialsFile), "r");
    if (!filep)
    {
        TRACE_ERROR("Failed to open credentials file \"%s\": %m", qPrintable(CredentialsFile));
        return -1;
    }

    if (!fgets(buf, 5125, filep))
    {
        TRACE_ERROR("Failed to read AppId from credentials file \"%s\"", qPrintable(CredentialsFile));
        fclose(filep);
        return -1;
    }
    if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = '\0';
    AppId = QString(buf);
    AppId.replace(0, 6, QString(""));
    
    if (!fgets(buf, 5125, filep))
    {
        TRACE_ERROR("Failed to read AppSecret from credentials file \"%s\"", qPrintable(CredentialsFile));
        fclose(filep);
        return -1;
    }
    if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = '\0';
    AppSecret = QString(buf);
    AppSecret.replace(0, 10, QString(""));

    fclose(filep);

    TRACE_INFO("Found credentials");

    /* Then, send a HTTP request to get the token and wait for answer (synchronously): */
    QEventLoop eventLoop;
    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req(QUrl(URL_AUTH));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QUrl params;
    QUrlQuery query;
    query.addQueryItem("grant_type", "client_credentials");
    query.addQueryItem("client_id", qPrintable(AppId));
    query.addQueryItem("client_secret", qPrintable(AppSecret));
    params.setQuery(query);
    QNetworkReply* reply = networkManager.post(req, params.toEncoded());

    eventLoop.exec(); // wait for answer

    if (reply->error() == QNetworkReply::NoError)
    {
        TRACE_DEBUG("HTTP request success");

        char buf[1024];
        int buflen;
        json_object *jobj;
        buflen = reply->read(buf, 1023);
        TRACE_DEBUG("reply is: %s", buf);

        jobj = json_tokener_parse(buf);
        if (!jobj)
        {
            TRACE_ERROR("json_tokener_parse failed");
            delete reply;
            return -1;
        }

        json_object_object_foreach(jobj, key, val)
        {
            (void)key;
            if (json_object_get_type(val) == json_type_string)
            {
                json_object *value;
                if(json_object_object_get_ex(jobj, "access_token", &value))
                {
                    TRACE_INFO("token was found");
                    token = QString(json_object_get_string(value));
                    free(value);
                    break;
                }
            }
        }

        free(jobj);
    }
    else
    {
        TRACE_ERROR("HTTP request failure: %s", qPrintable(reply->errorString()));
        delete reply;
        return -1;
    }

    delete reply;
    return 0;
}

int MainApp::StartMonitoringUserInput()
{
    connect(&lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplySearch(QNetworkReply*)));
    return 1;
}

void MainApp::cancelClicked()
{
    TRACE_WARN("cancel clicked !");
    DisplayInformation(false);
    Expand(false);
    TRACE_WARN("done");
}

void MainApp::goClicked()
{
    TRACE_WARN("go clicked !");

    TRACE_WARN("done");
}
