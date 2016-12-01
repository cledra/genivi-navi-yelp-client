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
#include "Keyboard.h"

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
        void setKeyboard(bool val)   { isKeyboard = val; }

    private:
        void ParseJsonBusinessList(const char* buf, std::vector<Business> & Output);
        bool eventFilter(QObject *obj, QEvent *ev);
        void SetDestination();
        bool IsCoordinatesConsistent(Business & business);
        void DisplayLineEdit(bool display = true);
        void DisplayResultList(bool display, bool RefreshDisplay = true);
        void DisplayInformation(bool display, bool RefreshDisplay = true);
        int FillResultList(std::vector<Business> & list, int focusIndex = 0);

        GeniviWrapper wrapper;
        QWidget window;
        QNetworkAccessManager networkManager;
        QPushButton searchBtn;
        QLineEdit lineEdit;
        QTreeWidget resultList;
        QMutex mutex; // to protect 'pSearchReply' from concurrent access
        QString token;
        QString currentSearchText;
        QNetworkReply *pSearchReply;
        InfoPanel *pInfoPanel;
        Keyboard *pKeyboard;
        double currentLatitude;
        double currentLongitude;
        uint32_t navicoreSession;
        int currentIndex;
        int fontId;
        bool isInfoScreen;
        bool isInputDisplayed;
        bool isKeyboard;
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
