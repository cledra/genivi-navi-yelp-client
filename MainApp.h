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
        explicit MainApp(int argc, char* argv[]);
        ~MainApp();
        int CheckGeniviApi();
        int AuthenticatePOI(const QString & CredentialsFile);
        int StartMonitoringUserInput();
        void setInfoScreen(bool val) { isInfoScreen = val; }

    private:
        void ParseJsonBusinessList(const char* buf, std::vector<Business> & Output);
        bool eventFilter(QObject *obj, QEvent *ev);
        void ShowKeyboard(bool show = true);
        void Expand(bool expand);
        void DisplayInformation();
        void SetDestination(double latitude = 0.0, double longitude = 0.0);

        uint32_t navicoreSession;
        QMutex mutex;
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

    private slots:
        void textChanged(const QString & text);
        void textAdded(const QString & text);
        void keyPressed(int key);
        void networkReplySearch(QNetworkReply* reply);
        void UpdateAglSurfaces();
        void goClicked();
};

#endif // __MAINAPP_H__
