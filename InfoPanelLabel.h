#ifndef __INFO_PANEL_LABEL_H__
#define __INFO_PANEL_LABEL_H__

#include <QWidget>
#include <QLabel>
#include <QRect>
#include <QFont>
#include <QString>

class InfoPanelLabel : public QLabel
{
    Q_OBJECT
    public:
        InfoPanelLabel(QWidget *parent, QRect &r);
        void Init(int pos, int height, const QString &text, QFont *font = NULL);

    private:
        QRect rect;
};

#endif // __INFO_PANEL_LABEL_H__
