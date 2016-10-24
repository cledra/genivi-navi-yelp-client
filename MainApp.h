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

class MainApp: public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainApp(int argc, char* argv[]);
        ~MainApp();
        int CheckGeniviApi();
        int AuthenticatePOI(const QString & CredentialsFile);
        int StartMonitoringUserInput();

    private:
        void ParseJsonBusinessList(const char* buf, std::vector<Business> & Output);
        bool eventFilter(QObject *obj, QEvent *ev);
        void Expand(bool expand);
        void SetDestination();
    
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

    private slots:
        void textChanged(const QString & text);
        void networkReplySearch(QNetworkReply* reply);
};

#endif // __MAINAPP_H__
