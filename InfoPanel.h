#ifndef __INFO_PANEL_H__
#define __INFO_PANEL_H__

#include <QWidget>
#include <QNetworkAccessManager>
#include <QRect>
#include <QPushButton>

#include "Business.h"
#include "InfoPanelLabel.h"

class InfoPanel
{
    public:
        InfoPanel(QWidget *parent, Business & business, QRect rect);
        virtual ~InfoPanel(){}
        QPushButton * getCancelButton() { return &cancelButton; }
        QPushButton * getGoButton() { return &goButton; }

    private:
        InfoPanelLabel nameLabel;
        InfoPanelLabel imageLabel;
        InfoPanelLabel addressLabel;
        InfoPanelLabel phoneLabel;
        InfoPanelLabel imgRatingLabel;
        InfoPanelLabel nbReviewsLabel;
        InfoPanelLabel btnsBackground;
        QPushButton cancelButton;
        QPushButton goButton;
        QNetworkAccessManager networkManager;
};

#endif // __INFO_PANEL_H__
