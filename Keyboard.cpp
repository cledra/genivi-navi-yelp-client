/**
  Source : inspired by https://www.kdab.com/qt-input-method-virtual-keyboard/
  */

#include "Keyboard.h"

#include <QWidget>
#include <QRect>
#include <QString>
#include <QSignalMapper>
#include <QPushButton>
#include <QImageReader>

#define NEXT_ROW_MARKER '\0'

#define SIZE_FACTOR 1       /* looks nice on AGL demo */
//#define SIZE_FACTOR 0.5   /* looks nice on PC build */

#define OFFSET_H        ( 78 * SIZE_FACTOR )
#define KEY_WIDTH       ( 78 * SIZE_FACTOR )
#define KEY_HEIGHT      ( 94 * SIZE_FACTOR )
#define MARGIN_H        ( 12 * SIZE_FACTOR )
#define MARGIN_V        ( 18 * SIZE_FACTOR )
#define SPACE_BAR_WIDTH ( 502 * SIZE_FACTOR )

struct KeyboardLayoutEntry
{
    int key;
    const char *image;
};

static KeyboardLayoutEntry keyboardLayout[] = {
    { '1' , "1.png" },
    { '2' , "2.png" },
    { '3' , "3.png" },
    { '4' , "4.png" },
    { '5' , "5.png" },
    { '6' , "6.png" },
    { '7' , "7.png" },
    { '8' , "8.png" },
    { '9' , "9.png" },
    { '0' , "0.png" },
    { NEXT_ROW_MARKER, NULL },
    { 'q' , "Q.png" },
    { 'w' , "W.png" },
    { 'e' , "E.png" },
    { 'r' , "R.png" },
    { 't' , "T.png" },
    { 'y' , "Y.png" },
    { 'u' , "U.png" },
    { 'i' , "I.png" },
    { 'o' , "O.png" },
    { 'p' , "P.png" },
    { NEXT_ROW_MARKER, NULL },
    { 'a' , "A.png" },
    { 's' , "S.png" },
    { 'd' , "D.png" },
    { 'f' , "F.png" },
    { 'g' , "G.png" },
    { 'h' , "H.png" },
    { 'j' , "J.png" },
    { 'k' , "K.png" },
    { 'l' , "L.png" },
    { NEXT_ROW_MARKER, NULL },
    { 'z' , "Z.png" },
    { 'x' , "X.png" },
    { 'c' , "C.png" },
    { 'v' , "V.png" },
    { 'b' , "B.png" },
    { 'n' , "N.png" },
    { 'm' , "M.png" },
    { '!' , "exclam.png" },
    { '\b',"back.png" },
    { NEXT_ROW_MARKER, NULL },
    { '\'',"apostrophe.png" },
    { '&' , "et.png" },
    { ' ' , "space.png" },
    { '-' , "minus.png" },
    { '/' , "slash.png" }
};

const static int layoutSize = (sizeof(keyboardLayout) /
                               sizeof(KeyboardLayoutEntry));

Keyboard::Keyboard(QRect r, QWidget *parent):QWidget(parent),background(parent),
    rect(QRect(r.x() + (r.width()-(r.width()*SIZE_FACTOR))/2, r.y(), r.width()*SIZE_FACTOR, r.height()*SIZE_FACTOR)),
    mapper(new QSignalMapper(this))
{
    int nbRowMarkers = 1;
    for (int i = 0; i < layoutSize; ++i)
        if (keyboardLayout[i].key == NEXT_ROW_MARKER)
            nbRowMarkers++;

    connect(mapper, SIGNAL(mapped(int)), SLOT(buttonClicked(int)));

    int row = 0;
    int offset_h = KEY_WIDTH;
    int offset_v = (rect.height() - (KEY_HEIGHT*nbRowMarkers + MARGIN_V*(nbRowMarkers-1))) / 2;

    background.setGeometry(rect);
    QImageReader reader(QString(":/images/background.png"));
    background.setPixmap(QPixmap::fromImage(reader.read()).scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio));

    background.show();

    for (int i = 0; i < layoutSize; ++i)
    {
        int key_width = KEY_WIDTH;
        if (keyboardLayout[i].key == NEXT_ROW_MARKER)
        {
            row++;
            offset_h = OFFSET_H;
            if (row == 2)
                offset_h += (KEY_WIDTH + MARGIN_H)/2;
            else if (row == 3)
                offset_h += (KEY_WIDTH + MARGIN_H);
            offset_v += (KEY_HEIGHT+MARGIN_V);
            continue;
        }
        else if (keyboardLayout[i].key ==  ' ')
        {
            key_width = SPACE_BAR_WIDTH;
        }

        QPushButton *button = new QPushButton(QIcon(tr(":/images/")+QString(keyboardLayout[i].image)), tr(""), &background);
        button->setMinimumSize(QSize(key_width, KEY_HEIGHT));
        button->setMaximumSize(QSize(key_width, KEY_HEIGHT));
        button->setIconSize(button->size());
        /* geometry of button is relative to its parent, ie 'background' : */
        button->setGeometry(QRect(offset_h, offset_v, button->width(), button->height()));
        button->show();

        mapper->setMapping(button, keyboardLayout[i].key);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));

        offset_h += (key_width+MARGIN_H);
    }
}

void Keyboard::buttonClicked(int key)
{
    if (key == '\b') /* backspace */
        emit specialKeyClicked(key);
    else
        emit keyClicked(QString(key));
}

Keyboard::~Keyboard()
{
    disconnect();
    delete mapper;
}
