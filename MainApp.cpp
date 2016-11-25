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
#include "ClickableLabel.h"
//#include "Keyboard.h"
#include "libgeniviwrapper/GeniviWrapper.h"
#include "libgeniviwrapper/include/genivi-navicore-constants.h"
#include "traces.h"

#define DEFAULT_TEXT        "Select your destination with Yelp !"
#define URL_AUTH            "https://api.yelp.com/oauth2/token"
#define URL_AUTOCOMPLETE    "https://api.yelp.com/v3/autocomplete"
#define URL_SEARCH          "https://api.yelp.com/v3/businesses/search"

#define BIG_BUFFER_SIZE     (1024*1024)
#define FONT_SIZE_LINEDIT   20
#define FONT_SIZE_LIST      14
#define TEXT_INPUT_WIDTH    500
#define SEARCH_BTN_SIZE     105
#define SPACER              15
#define WIDGET_WIDTH        (SEARCH_BTN_SIZE + SPACER + TEXT_INPUT_WIDTH)
#define RESULT_LIST_WIDTH   TEXT_INPUT_WIDTH
#define RESULT_LIST_HEIGHT  480
#define RESULT_ITEM_HEIGHT  80
#define INFO_WIDTH          TEXT_INPUT_WIDTH
#define INFO_HEIGHT         480
#define MARGINS             25

using namespace std;

MainApp::MainApp():
    navicoreSession(0),mutex(QMutex::Recursive),wrapper(),
    window(Q_NULLPTR, Qt::FramelessWindowHint),
    token(""),networkManager(this),pSearchReply(NULL),
    searchBtn(QIcon(tr(":/images/loupe-90.png")), tr(""), &window),
    isInfoScreen(false),
    isInputDisplayed(false),
    lineEdit(&window),resultList(&window),pInfoPanel(NULL),
    currentLatitude(0.0),currentLongitude(0.0),currentSearchText(""),
    currentIndex(0)
{
    window.setAttribute(Qt::WA_TranslucentBackground);
    window.setStyleSheet("border: none;");

    //keyboard.hide();

    searchBtn.setStyleSheet("border: none;");
    searchBtn.setMinimumSize(QSize(SEARCH_BTN_SIZE, SEARCH_BTN_SIZE));
    searchBtn.setIconSize(searchBtn.size());
    searchBtn.setGeometry(QRect(0, 0, searchBtn.width(), searchBtn.height()));

    lineEdit.setStyleSheet("border: none;");
    lineEdit.setMinimumSize(QSize(TEXT_INPUT_WIDTH, SEARCH_BTN_SIZE));

    lineEdit.setPlaceholderText(QString(DEFAULT_TEXT));
    QFont font = lineEdit.font();
    font.setPointSize(FONT_SIZE_LINEDIT);
    lineEdit.setFont(font);
    lineEdit.setTextMargins(MARGINS/2, 0, 0, 0);
    lineEdit.installEventFilter(this);
    lineEdit.setVisible(false);

    resultList.setStyleSheet("border: none;");
    resultList.setRootIsDecorated(false);
    resultList.setEditTriggers(QTreeWidget::NoEditTriggers);
    resultList.setSelectionBehavior(QTreeWidget::SelectRows);
    resultList.setFrameStyle(QFrame::Box | QFrame::Plain);
    resultList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resultList.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resultList.header()->hide();
    font.setPointSize(FONT_SIZE_LIST);
    resultList.setFont(font);
    resultList.installEventFilter(this);
    resultList.setVisible(false);

    /* We might need a Japanese font: */
    QFile fontFile(":/fonts/DroidSansJapanese.ttf");
    if (!fontFile.open(QIODevice::ReadOnly))
    {
        TRACE_ERROR("failed to open font file");
    }
    else
    {
        QByteArray fontData = fontFile.readAll();
        if (QFontDatabase::addApplicationFontFromData(fontData) == -1)
        {
            TRACE_ERROR("QFontDatabase::addApplicationFontFromData failed");
        }
    }

    window.setGeometry(QRect(window.pos().x(), window.pos().y(), searchBtn.width(), searchBtn.height()));
    window.show();
}

MainApp::~MainApp()
{
    mutex.lock();
    delete pSearchReply;
    mutex.unlock();
}

void MainApp::searchBtnClicked()
{
    isInputDisplayed = !isInputDisplayed;
    TRACE_DEBUG("isInputDisplayed = %d", isInputDisplayed);
    DisplayLineEdit(isInputDisplayed);
}

void MainApp::DisplayLineEdit(bool display)
{
    mutex.lock();
    if (display)
    {
        lineEdit.setGeometry(QRect(searchBtn.width()+SPACER, 0, lineEdit.width(), lineEdit.height()));
        lineEdit.setVisible(true);
        window.setGeometry(QRect(window.pos().x(), window.pos().y(), WIDGET_WIDTH, searchBtn.height()));
        lineEdit.setFocus();
    }
    else
    {
        DisplayResultList(false);
        DisplayInformation(false);
        lineEdit.setText(tr(""));
        lineEdit.setVisible(false);
        window.setGeometry(QRect(window.pos().x(), window.pos().y(), searchBtn.width(), searchBtn.height()));
    }
    isInputDisplayed = display;
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

void MainApp::DisplayResultList(bool display)
{
    mutex.lock();
    if (display)
    {
        resultList.setGeometry(QRect(   searchBtn.width()+SPACER, searchBtn.height()+SPACER,
                                        RESULT_LIST_WIDTH, RESULT_LIST_HEIGHT));
        window.setGeometry(QRect(   window.pos().x(), window.pos().y(),
                                    WIDGET_WIDTH,
                                    searchBtn.height()+SPACER+resultList.height()));
        resultList.setVisible(true);
        resultList.setFocus();
    }
    else
    {
        resultList.setVisible(false);
        lineEdit.setFocus();
        window.setGeometry(QRect(window.pos().x(), window.pos().y(), WIDGET_WIDTH, searchBtn.height()));
    }
    mutex.unlock();
}

void MainApp::textChanged(const QString & text)
{
    TRACE_INFO("New text is: %s", qPrintable(text));

    /* do not handle text input if info panel is displayed: */
    if (pInfoPanel) return;

    mutex.lock();

    delete pSearchReply;    /* cancel current search */
    pSearchReply = NULL;

    if (text.length() == 0) /* if empty text -> no search */
    {
        DisplayResultList(false);
        mutex.unlock();
        return;
    }

    /* if text is the same as previous search -> no need to search again */
    if (text == currentSearchText)
    {
        DisplayResultList(true);
        FillResultList(Businesses, currentIndex);
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
    QString myUrlStr = URL_SEARCH + tr("?") + tr("term=") + text +
        tr("&latitude=") + QString::number(currentLatitude) +
        tr("&longitude=") + QString::number(currentLongitude);

    TRACE_DEBUG("URL: %s", qPrintable(myUrlStr));

    QUrl myUrl = QUrl(myUrlStr);
    QNetworkRequest req(myUrl);
    req.setRawHeader("Authorization", (tr("bearer ") + token).toLocal8Bit());
    pSearchReply = networkManager.get(req);
    TRACE_DEBUG("after post, reply is %p", pSearchReply);
    
    mutex.unlock();
}

void MainApp::textAdded(const QString & text)
{
    mutex.lock();
    lineEdit.setText(lineEdit.text() + text);
    mutex.unlock();
}

void MainApp::keyPressed(int key)
{
    mutex.lock();
    if (key == Qt::Key_Backspace)
    {
        int len = lineEdit.text().length();
        if (len > 0)
            lineEdit.setText(lineEdit.text().remove(len-1, 1));
        /*else if (isKeyboard)
            ShowKeyboard(false);*/
    }
    mutex.unlock();
}

void MainApp::itemClicked()
{
    mutex.lock();
    if (isInfoScreen)
    {
        DisplayInformation(true);
    }
    else
    {
        SetDestination();
        DisplayLineEdit(false);
    }
    mutex.unlock();
}

void MainApp::ParseJsonBusinessList(const char* buf, std::vector<Business> & Output)
{
    json_object *jobj = json_tokener_parse(buf);
    if (!jobj)
    {
        TRACE_ERROR("json_tokener_parse failed");
        cerr << "json_tokener_parse failed: " << buf << endl;
        return;
    }

    json_object_object_foreach(jobj, key, val)
    {
        (void)key;
        json_object *value;
        
        if (json_object_get_type(val) == json_type_array)
        {
            TRACE_DEBUG_JSON("an array was found");

            if(json_object_object_get_ex(jobj, "businesses", &value))
            {
                TRACE_DEBUG_JSON("an business was found");

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
                            TRACE_DEBUG_JSON("got Rating : %f", NewBusiness.Rating);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "distance", &medi_array_obj_elem))
                        {
                            NewBusiness.Distance = json_object_get_double(medi_array_obj_elem);
                            TRACE_DEBUG_JSON("got Distance : %f", NewBusiness.Distance);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "review_count", &medi_array_obj_elem))
                        {
                            NewBusiness.ReviewCount = json_object_get_int(medi_array_obj_elem);
                            TRACE_DEBUG_JSON("got ReviewCount : %u", NewBusiness.ReviewCount);
                        }

                        if (json_object_object_get_ex(medi_array_obj, "name", &medi_array_obj_elem))
                        {
                            NewBusiness.Name = QString(json_object_get_string(medi_array_obj_elem));
                            TRACE_DEBUG_JSON("got Name : %s", qPrintable(NewBusiness.Name));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "image_url", &medi_array_obj_elem))
                        {
                            NewBusiness.ImageUrl = QString(json_object_get_string(medi_array_obj_elem));
                            TRACE_DEBUG_JSON("got ImageUrl : %s", qPrintable(NewBusiness.ImageUrl));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "phone", &medi_array_obj_elem))
                        {
                            NewBusiness.Phone = QString(json_object_get_string(medi_array_obj_elem));
                            TRACE_DEBUG_JSON("got Phone : %s", qPrintable(NewBusiness.Phone));
                        }

                        if (json_object_object_get_ex(medi_array_obj, "coordinates", &medi_array_obj_elem))
                        {
                            json_object *value2;
                            
                            TRACE_DEBUG_JSON("coordinates were found");

                            if(json_object_object_get_ex(medi_array_obj_elem, "latitude", &value2))
                            {
                                NewBusiness.Latitude = json_object_get_double(value2);
                                TRACE_DEBUG_JSON("got Latitude : %f", NewBusiness.Latitude);
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "longitude", &value2))
                            {
                                NewBusiness.Longitude = json_object_get_double(value2);
                                TRACE_DEBUG_JSON("got Longitude : %f", NewBusiness.Longitude);
                            }
                        }

                        if (json_object_object_get_ex(medi_array_obj, "location", &medi_array_obj_elem))
                        {
                            json_object *value2;
                            
                            TRACE_DEBUG_JSON("a location was found");

                            /* TODO: how do we deal with address2 and address3 ? */
                            if(json_object_object_get_ex(medi_array_obj_elem, "address1", &value2))
                            {
                                NewBusiness.Address = QString(json_object_get_string(value2));
                                TRACE_DEBUG_JSON("got Address : %s", qPrintable(NewBusiness.Address));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "city", &value2))
                            {
                                NewBusiness.City = QString(json_object_get_string(value2));
                                TRACE_DEBUG_JSON("got City : %s", qPrintable(NewBusiness.City));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "state", &value2))
                            {
                                NewBusiness.State = QString(json_object_get_string(value2));
                                TRACE_DEBUG_JSON("got State : %s", qPrintable(NewBusiness.State));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "zip_code", &value2))
                            {
                                NewBusiness.ZipCode = QString(json_object_get_string(value2));
                                TRACE_DEBUG_JSON("got ZipCode : %s", qPrintable(NewBusiness.ZipCode));
                            }

                            if(json_object_object_get_ex(medi_array_obj_elem, "country", &value2))
                            {
                                NewBusiness.Country = QString(json_object_get_string(value2));
                                TRACE_DEBUG_JSON("got Country : %s", qPrintable(NewBusiness.Country));
                            }
                        }

                        /* TODO: parse categories */

                        /* Add business in our list: */
                        Businesses.push_back(NewBusiness);
                    }
                }
            }
        }
    }

    json_object_put(jobj);
}

bool MainApp::eventFilter(QObject *obj, QEvent *ev)
{
    mutex.lock();

    if (obj == &resultList)
    {
        //TRACE_DEBUG("ev->type() = %d", (int)ev->type());

        /*if (lineEdit.hasSelectedText())
        {
            // I never want the text to be selected, but sometimes it is (why ??)
            //lineEdit.deselect();
            TRACE_WARN(" ");
        }*/

        /*if (ev->type() == QEvent::MouseButtonRelease)
        {
            if (isInfoScreen)
            {
                TRACE_WARN(" ");
                DisplayResultList(false);
                DisplayInformation(true);
            }
            else
            {
                TRACE_WARN(" ");
                SetDestination();
                DisplayLineEdit(false);
            }
        }
        else*/ if (ev->type() == QEvent::KeyPress)
        {
            bool consumed = false;
            int key = static_cast<QKeyEvent*>(ev)->key();
            TRACE_DEBUG("key pressed (%d)", key);
            switch (key) {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    TRACE_DEBUG("enter or return");
                    if (isInfoScreen)
                    {
                        DisplayInformation(true);
                        break;
                    }
                    else
                    {
                        SetDestination();
                        DisplayLineEdit(false);
                    }
                    consumed = true;

                case Qt::Key_Escape:
                    TRACE_DEBUG("escape");
                    DisplayResultList(false);
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
                    break;
            }

            mutex.unlock();
            return consumed;
        }
    }
    else if (obj == &lineEdit)
    {
        if (pInfoPanel && ev->type() == QEvent::KeyPress)
        {
            switch(static_cast<QKeyEvent*>(ev)->key())
            {
                case Qt::Key_Escape:
                    TRACE_DEBUG("Escape !");
                    DisplayInformation(false);
                    DisplayResultList(true);
                    break;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    TRACE_DEBUG("Go !");
                    SetDestination();
                    DisplayLineEdit(false);
                    break;
                default: break;
            }
        }
        /*if (ev->type() == QEvent::MouseButtonRelease)
        {
            TRACE_DEBUG("lineEdit widget clicked !");
            if (isKeyboard && !infoPanel && !keyboard.isVisible())
                ShowKeyboard();
        }*/
    }
    mutex.unlock();
    return false;
}

void MainApp::SetDestination()
{
    mutex.lock();
    /* params are only used if no item is selected in pResultList. */

    QList<QTreeWidgetItem *> SelectedItems = resultList.selectedItems();
    if (SelectedItems.size() <= 0)
    {
        TRACE_INFO("no item is selected");
        mutex.unlock();
        return;
    }

    /* select the first selected item : */
    int index = resultList.indexOfTopLevelItem(*SelectedItems.begin());
    TRACE_DEBUG("index is: %d", index);

    /* retrieve the coordinates of this item : */
    double latitude = Businesses[index].Latitude;
    double longitude = Businesses[index].Longitude;

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
    Waypoint destWp(latitude, longitude);
    std::vector<Waypoint> myWayPoints;
    myWayPoints.push_back(destWp);
    wrapper.NavicoreSetWaypoints(navicoreSession, myRoute, true, myWayPoints);

    wrapper.NavicoreCalculateRoute(navicoreSession, myRoute);

    /* reset search: */
    currentSearchText = tr("");
    currentIndex = 0;
    Businesses.clear();

    mutex.unlock();
}

void MainApp::DisplayInformation(bool display)
{
    if (display)
    {
        //DisplayResultList(false);
        QList<QTreeWidgetItem *> SelectedItems = resultList.selectedItems();
        if (SelectedItems.size() <= 0)
        {
            TRACE_ERROR("no item is selected");
            return;
        }

        /* select the first selected item : */
        currentIndex = resultList.indexOfTopLevelItem(*SelectedItems.begin());

        /* Resize window: */
        DisplayResultList(false);

        /* Display info for the selected item: */
        QRect rect( searchBtn.width()+SPACER, searchBtn.height()+SPACER,
                    INFO_WIDTH, INFO_HEIGHT);
        pInfoPanel = new InfoPanel(&window, Businesses[currentIndex], rect);

        window.setGeometry(QRect(   window.pos().x(), window.pos().y(),
                                    WIDGET_WIDTH,
                                    searchBtn.height()+SPACER+INFO_HEIGHT));

        connect(pInfoPanel->getGoButton(),      SIGNAL(clicked(bool)), this, SLOT(goClicked()));
        connect(pInfoPanel->getCancelButton(),  SIGNAL(clicked(bool)), this, SLOT(cancelClicked()));
    }
    else
    {
        if (pInfoPanel)
        {
            delete pInfoPanel;
            pInfoPanel = NULL;
        }
        lineEdit.setFocus();
        window.setGeometry(QRect(window.pos().x(), window.pos().y(), WIDGET_WIDTH, searchBtn.height()));
    }

    /*if (getenv("AGL_NAVI"))
    {
        QTimer timer(this);
        timer.singleShot(100, this, SLOT(UpdateAglSurfaces()));
    }*/

    //int res = msgBox.exec(); // wait for user to click

    /*mutex.lock();

    if (res == QDialog::Accepted)
    {
        double latitude, longitude;
        if (pInfoPanel->getCoords(latitude, longitude))
        {
            TRACE_ERROR("warning: make sure that no index is used"); // TODO: remove this
            SetDestination(latitude, longitude);
        }
    }

    //vLayout.removeWidget(infoPanel);
    delete pInfoPanel;
    pInfoPanel = NULL;

    if (res == QDialog::Rejected)
    {
        //Expand(false, false);
        DisplayResultList(true);
        FillResultList(Businesses, currentIndex);
    }

    mutex.unlock();

    if (res == QDialog::Accepted)
    {
        lineEdit.setText(tr(""));
    }*/
}

void MainApp::networkReplySearch(QNetworkReply* reply)
{
    char buf[BIG_BUFFER_SIZE];
    int buflen;
    
    mutex.lock();

    /* memorize the text which gave this result: */
    currentSearchText = lineEdit.text();

    // we only handle this callback if it matches the last search request:
    if (reply != pSearchReply)
    {
        TRACE_INFO("this reply is already too late (or about a different network request)");
        mutex.unlock();
        return;
    }
    
    buflen = reply->read(buf, BIG_BUFFER_SIZE-1);
    buf[buflen] = '\0';

    if (buflen == 0)
    {
        mutex.unlock();
        return;
    }

    currentIndex = 0;
    Businesses.clear();
    ParseJsonBusinessList(buf, Businesses);
    DisplayResultList(true);
    FillResultList(Businesses);
    
    mutex.unlock();
}

int MainApp::FillResultList(vector<Business> & list, int focusIndex)
{
    int nbElem = 0;

    mutex.lock();

    resultList.setUpdatesEnabled(false);
    resultList.clear();

    /* filling the dropdown menu: */
    for (vector<Business>::iterator it = list.begin(); it != list.end(); it++)
    {
        /*  workaround to avoid entries with wrong coordinates returned by Yelp: */
        if (IsCoordinatesConsistent(*it) == false)
        {
            list.erase(it--);
            continue;
        }

        QTreeWidgetItem * item = new QTreeWidgetItem(&resultList);
        ClickableLabel *label = new ClickableLabel("<b>"+(*it).Name+"</b><br>"+(*it).Address+", "+(*it).City+", "+
            (*it).State+" "+(*it).ZipCode+", "+(*it).Country);
        label->setTextFormat(Qt::RichText);
        label->setIndent(MARGINS);
        //resultList.addTopLevelItem(item);
        item->setSizeHint(0, QSize(TEXT_INPUT_WIDTH, RESULT_ITEM_HEIGHT));
        resultList.setItemWidget(item, 0, label);
        connect(label, SIGNAL(clicked()), this, SLOT(itemClicked()));

        if (nbElem == focusIndex)
        {
            resultList.setCurrentItem(item); 
        }
        nbElem++;
    }

    resultList.setUpdatesEnabled(true);

    mutex.unlock();
    return nbElem;
}

/* Well... some of the POI returned by Yelp have coordinates which are
 * completely inconsistent with the distance at which the POI is
 * supposed to be.
 * https://github.com/Yelp/yelp-fusion/issues/104
 * Let's skip them for the moment: */
#define PI 3.14159265
#define EARTH_RADIUS 6371000
static inline double toRadians(double a) { return a * PI / 180.0; }
bool MainApp::IsCoordinatesConsistent(Business & business)
{
    double lat1 = toRadians(currentLatitude);
    double lon1 = toRadians(currentLongitude);
    double lat2 = toRadians(business.Latitude);
    double lon2 = toRadians(business.Longitude);
    double x = (lon2 - lon1) * cos((lat1 + lat2)/2);
    double y = lat2 - lat1;
    double DistanceFromCoords = EARTH_RADIUS * sqrt(pow(x, 2) + pow(y, 2));

    /* if calculated distance is not between +/- 10% of the announced
     * distance -> skip this POI: */
    if (DistanceFromCoords < business.Distance * 0.9 ||
        DistanceFromCoords > business.Distance * 1.1)
    {
        TRACE_ERROR("Announced distance: %f, calculated distance: %f", business.Distance, DistanceFromCoords);
        return false;
    }

    return true;
}
/* end of workaround */

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

    if (!fgets(buf, 512, filep))
    {
        TRACE_ERROR("Failed to read AppId from credentials file \"%s\"", qPrintable(CredentialsFile));
        fclose(filep);
        return -1;
    }
    if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = '\0';
    AppId = QString(buf);
    AppId.replace(0, 6, tr(""));
    
    if (!fgets(buf, 512, filep))
    {
        TRACE_ERROR("Failed to read AppSecret from credentials file \"%s\"", qPrintable(CredentialsFile));
        fclose(filep);
        return -1;
    }
    if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = '\0';
    AppSecret = QString(buf);
    AppSecret.replace(0, 10, tr(""));

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
        buf[buflen] = '\0';

        if (buflen == 0)
        {
            delete reply;
            return -1;
        }
        
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
                    break;
                }
            }
        }

        json_object_put(jobj);
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
    connect(&searchBtn, SIGNAL(clicked(bool)), this, SLOT(searchBtnClicked()));
    connect(&lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    /*if (isKeyboard)
    {
        connect(&keyboard, SIGNAL(keyClicked(const QString &)), this, SLOT(textAdded(const QString &)));
        connect(&keyboard, SIGNAL(specialKeyClicked(int)), this, SLOT(keyPressed(int)));
    }*/
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplySearch(QNetworkReply*)));
    return 1;
}

void MainApp::goClicked()
{
    TRACE_DEBUG("Go clicked !");
    SetDestination();
    DisplayLineEdit(false);
}

void MainApp::cancelClicked()
{
    TRACE_DEBUG("Cancel clicked !");
    DisplayInformation(false);
    DisplayResultList(true);
    FillResultList(Businesses, currentIndex);
}
