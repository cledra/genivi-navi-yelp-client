/**
  Source : https://www.kdab.com/qt-input-method-virtual-keyboard/
  */

#include "Keyboard.h"

#include <QWidget>
#include <QGridLayout>
#include <QSignalMapper>
#include <QPushButton>

#define NEXT_ROW_MARKER 0
#define NEXT_COL_MARKER 1
#define KEY_WIDTH 60

struct KeyboardLayoutEntry{
    int key;
    const char *lowerlabel;
    const char *upperlabel;
};

/*KeyboardLayoutEntry keyboardLayout[] = {
    { Qt::Key_1, "1", "1" },
    { Qt::Key_2, "2", "2" },
    { Qt::Key_3, "3", "3" },
    { Qt::Key_4, "4", "4" },
    { Qt::Key_5, "5", "5" },
    { Qt::Key_6, "6", "6" },
    { Qt::Key_7, "7", "7" },
    { Qt::Key_8, "8", "8" },
    { Qt::Key_9, "9", "9" },
    { Qt::Key_0, "0", "0" },
    { Qt::Key_Backspace, "<-", "<-" },
    { NEXT_ROW_MARKER, 0, 0 },
    { Qt::Key_Q, "q", "Q" },
    { Qt::Key_W, "w", "W" },
    { Qt::Key_E, "e", "E" },
    { Qt::Key_R, "r", "R" },
    { Qt::Key_T, "t", "T" },
    { Qt::Key_Z, "y", "Y" },
    { Qt::Key_U, "u", "U" },
    { Qt::Key_I, "i", "I" },
    { Qt::Key_O, "o", "O" },
    { Qt::Key_P, "p", "P" },
    { NEXT_ROW_MARKER, 0, 0 },
    { NEXT_COL_MARKER, 0, 0 },
    { Qt::Key_A, "a", "A" },
    { Qt::Key_S, "s", "S" },
    { Qt::Key_D, "d", "D" },
    { Qt::Key_F, "f", "F" },
    { Qt::Key_G, "g", "G" },
    { Qt::Key_H, "h", "H" },
    { Qt::Key_J, "j", "J" },
    { Qt::Key_K, "k", "K" },
    { Qt::Key_L, "l", "L" },
    { NEXT_ROW_MARKER, 0, 0 },
    { NEXT_COL_MARKER, 0, 0 },
    { NEXT_COL_MARKER, 0, 0 },
    { Qt::Key_Y, "z", "Z" },
    { Qt::Key_X, "x", "X" },
    { Qt::Key_C, "c", "C" },
    { Qt::Key_V, "v", "V" },
    { Qt::Key_B, "b", "B" },
    { Qt::Key_N, "n", "N" },
    { Qt::Key_M, "m", "M" },
    //{ Qt::Key_Enter, "Enter", "Enter" }
};*/

KeyboardLayoutEntry keyboardLayout[] = {
    { Qt::Key_1, "1", "1" },
    { Qt::Key_2, "2", "2" },
    { Qt::Key_3, "3", "3" },
    { Qt::Key_4, "4", "4" },
    { Qt::Key_5, "5", "5" },
    { Qt::Key_6, "6", "6" },
    { Qt::Key_7, "7", "7" },
    { Qt::Key_8, "8", "8" },
    { Qt::Key_9, "9", "9" },
    { Qt::Key_0, "0", "0" },
    { Qt::Key_Backspace, "<-", "<-" },
    { NEXT_ROW_MARKER, 0, 0 },
    { Qt::Key_Q, "q", "Q" },
    { Qt::Key_W, "w", "W" },
    { Qt::Key_E, "e", "E" },
    { Qt::Key_R, "r", "R" },
    { Qt::Key_T, "t", "T" },
    { Qt::Key_Z, "y", "Y" },
    { Qt::Key_U, "u", "U" },
    { Qt::Key_I, "i", "I" },
    { Qt::Key_O, "o", "O" },
    { Qt::Key_P, "p", "P" },
    { NEXT_ROW_MARKER, 0, 0 },
    { Qt::Key_A, "a", "A" },
    { Qt::Key_S, "s", "S" },
    { Qt::Key_D, "d", "D" },
    { Qt::Key_F, "f", "F" },
    { Qt::Key_G, "g", "G" },
    { Qt::Key_H, "h", "H" },
    { Qt::Key_J, "j", "J" },
    { Qt::Key_K, "k", "K" },
    { Qt::Key_L, "l", "L" },
    { NEXT_ROW_MARKER, 0, 0 },
    { Qt::Key_Y, "z", "Z" },
    { Qt::Key_X, "x", "X" },
    { Qt::Key_C, "c", "C" },
    { Qt::Key_V, "v", "V" },
    { Qt::Key_B, "b", "B" },
    { Qt::Key_N, "n", "N" },
    { Qt::Key_M, "m", "M" },
    { Qt::Key_Comma, ",", "," },
    { NEXT_ROW_MARKER, 0, 0 },
    { Qt::Key_Apostrophe, "'", "'" },
    { Qt::Key_Minus, "-", "-" },
    { Qt::Key_Space, " ", "" },
    { Qt::Key_Slash, "/", "/" },
    { Qt::Key_Exclam, "!", "!" }
};

const static int layoutSize = (sizeof(keyboardLayout) /
                               sizeof(KeyboardLayoutEntry));

Keyboard::Keyboard(QWidget *parent)
    : QWidget(parent)
{
    int nbRowMarkers = 1;
    for (int i = 0; i < layoutSize; ++i)
        if (keyboardLayout[i].key == NEXT_ROW_MARKER)
            nbRowMarkers++;
    
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    QGridLayout *rows[nbRowMarkers];
    for (int i=0; i<nbRowMarkers; i++)
    {
        rows[i] = new QGridLayout(NULL);
        vboxLayout->insertLayout(i, rows[i]);
        rows[i]->setContentsMargins(KEY_WIDTH*i/2, 0, KEY_WIDTH*i/2, 0);
    }

    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), SLOT(buttonClicked(int)));

    int row = 0;
    int column = 0;

    for (int i = 0; i < layoutSize; ++i)
    {
        if (keyboardLayout[i].key == NEXT_ROW_MARKER)
        {
            row++;
            column = 0;
            continue;
        }

        QPushButton *button = new QPushButton;
        QFont font = button->font();
        font.setPointSize(19);
        button->setFont(font);
        button->setFixedHeight(33);
        if (keyboardLayout[i].key == Qt::Key_Backspace)
            button->setFixedWidth(KEY_WIDTH*1.5);
        else if (keyboardLayout[i].key == Qt::Key_Space)
            button->setFixedWidth(220);
        else
            button->setFixedWidth(KEY_WIDTH);

        button->setText(QString::fromLatin1(keyboardLayout[i].upperlabel));

        mapper->setMapping(button, keyboardLayout[i].key);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));

        rows[row]->addWidget(button, 0, column);
        column++;
    }
}

static QString keyToCharacter(int key)
{
    for (int i = 0; i < layoutSize; ++i)
    {
        if (keyboardLayout[i].key == key)
            return QString::fromLatin1(keyboardLayout[i].lowerlabel);
    }

    return QString();
}

void Keyboard::buttonClicked(int key)
{
    if (key == Qt::Key_Backspace)
        emit specialKeyClicked(key);
    else
        emit keyClicked(keyToCharacter(key));
}
