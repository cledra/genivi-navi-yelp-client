#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <QWidget>
#include <QRect>
#include <QLabel>

class Keyboard : public QWidget
{
    Q_OBJECT
    public:
        explicit Keyboard(QRect r, QWidget *parent = Q_NULLPTR);

    signals:
        void specialKeyClicked(int key);
        void keyClicked(const QString &text);

    private slots:
        void buttonClicked(int key);

    private:
        QRect rect;
        QLabel background;
};

#endif // __KEYBOARD_H__
