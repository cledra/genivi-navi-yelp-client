#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include <QMainWindow>
#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutex>
#include <QTreeWidget>
#include <vector>
#include "libgeniviwrapper/GeniviWrapper.h"
#include "Business.h"
#include "InfoPanel.h"
//#include "Keyboard.h"

class MainApp: public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainApp();
        ~MainApp();
        int CheckGeniviApi();
        int AuthenticatePOI(const QString & CredentialsFile);
        int StartMonitoringUserInput();
        void setInfoScreen(bool val) { isInfoScreen = val; }
        //void setKeyboard(bool val)   { isKeyboard = val; }

    private:
        void ParseJsonBusinessList(const char* buf, std::vector<Business> & Output);
        bool eventFilter(QObject *obj, QEvent *ev);
        void ShowKeyboard(bool show = true);
        void DisplayInformation(bool display);
        void SetDestination();
        bool IsCoordinatesConsistent(Business & business);
        void DisplayLineEdit(bool display = true);
        void DisplayResultList(bool display);
        int FillResultList(std::vector<Business> & list, int focusIndex = 0);

        uint32_t navicoreSession;
        QMutex mutex; // to protect 'pSearchReply' from concurrent access
        GeniviWrapper wrapper;
        QWidget window;
        QString token;
        QNetworkAccessManager networkManager;
        QNetworkReply *pSearchReply;
        QPushButton searchBtn;
        //Keyboard keyboard;
        bool isInfoScreen;
        bool isInputDisplayed;
        //bool isKeyboard;
        QLineEdit lineEdit;
        QTreeWidget resultList;
        InfoPanel* pInfoPanel;

        double currentLatitude;
        double currentLongitude;
        QString currentSearchText;
        int currentIndex;
        std::vector<Business> Businesses;

    private slots:
        void searchBtnClicked();
        void textChanged(const QString & text);
        void textAdded(const QString & text);
        void keyPressed(int key);
        void itemClicked();
        void networkReplySearch(QNetworkReply* reply);
        void UpdateAglSurfaces();
        void goClicked();
        void cancelClicked();
};

#endif // __MAINAPP_H__
