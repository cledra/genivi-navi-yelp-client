#ifndef __INFO_PANEL_H__
#define __INFO_PANEL_H__

#include <QWidget>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QBoxLayout>
#include <QMessageBox>
#include "Business.h"

class InfoPanel: public QMessageBox
{
    Q_OBJECT
    
    public:
        InfoPanel(QWidget *parent, QBoxLayout *layout, Business & business);
        virtual ~InfoPanel();
        bool getCoords(double &lat, double &lon);

    private:
        QBoxLayout *pLayout;
        QLabel nameLabel;
        QLabel imageLabel;
        QLabel addressLabel;
        QLabel zipCodeCityLabel;
        QLabel phoneLabel;
        QLabel imgRatingLabel;
        QLabel nbReviewsLabel;
        QNetworkAccessManager networkManager;

        double latitude, longitude;
};

#endif // __INFO_PANEL_H__
