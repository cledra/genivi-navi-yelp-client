#ifndef __INFO_PANEL_H__
#define __INFO_PANEL_H__

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QBoxLayout>
#include <QPushButton>
#include <QObject>
#include "Business.h"

class InfoPanel: public QObject
{
    public:
        InfoPanel(QWidget *window, QBoxLayout *layout, Business & business);
        ~InfoPanel();

    private:
        QBoxLayout *pLayout;
        QLabel nameLabel;
        QLabel imageLabel;
        QLabel addressLabel;
        QLabel zipCodeCityLabel;
        QLabel phoneLabel;
        QLabel imgRatingLabel;
        QLabel nbReviewsLabel;
        QHBoxLayout hLayout;
        QPushButton BackButton;
        QPushButton GoButton;

        QNetworkAccessManager networkManager;
};

#endif // __INFO_PANEL_H__
