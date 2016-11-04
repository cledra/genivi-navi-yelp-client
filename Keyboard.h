#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <QWidget>

class Keyboard : public QWidget
{
    Q_OBJECT

public:
    explicit Keyboard(QWidget *parent = Q_NULLPTR);

signals:
    void specialKeyClicked(int key);
    void keyClicked(const QString &text);

private slots:
    void buttonClicked(int key);
};

#endif // __KEYBOARD_H__
