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
        void ShowKeyboard(bool show = true);
        void Expand(bool expand, bool repaint = true);
        void DisplayInformation();
        void SetDestination(double latitude = 0.0, double longitude = 0.0);
        bool IsCoordinatesConsistent(Business & business);
        int DisplayResultList(std::vector<Business> & list, int focusIndex = 0);

        uint32_t navicoreSession;
        QMutex mutex; // to protect 'pSearchReply' from concurrent access
        GeniviWrapper wrapper;
        QWidget window;
        QVBoxLayout layout;
        QLineEdit lineEdit;
        QString token;
        QNetworkAccessManager networkManager;
        QNetworkReply *pSearchReply;
        QTreeWidget *pResultList;
        std::vector<Business> Businesses;
        Keyboard keyboard;
        InfoPanel *infoPanel;
        bool isInfoScreen;
        bool isKeyboard;

        double currentLatitude;
        double currentLongitude;
        QString currentSearchText;
        int currentIndex;

    private slots:
        void textChanged(const QString & text);
        void textAdded(const QString & text);
        void keyPressed(int key);
        void itemClicked(QTreeWidgetItem *item, int column);
        void networkReplySearch(QNetworkReply* reply);
        void UpdateAglSurfaces();
        //void goClicked();
};

#endif // __MAINAPP_H__
