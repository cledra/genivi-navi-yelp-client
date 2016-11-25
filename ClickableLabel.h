#ifndef __CLICKABLE_LABEL_H__
#define __CLICKABLE_LABEL_H__

#include <QLabel>
#include <QWidget>
#include <QString>

class ClickableLabel : public QLabel
{
    Q_OBJECT
    public:
        ClickableLabel(const QString& text="", QWidget* parent=0 ):QLabel(parent)
        {
            setText(text);
        }
        ~ClickableLabel(){}
    signals:
        void clicked();
    protected:
        void mouseReleaseEvent(QMouseEvent* event) { emit clicked(); }
};

#endif // __CLICKABLE_LABEL_H__
