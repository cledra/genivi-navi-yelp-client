#include "InfoPanelLabel.h"

InfoPanelLabel::InfoPanelLabel(QWidget *parent, QRect &r):QLabel(parent),rect(r)
{
}

void InfoPanelLabel::Init(int pos, int height, const QString &text, QFont *font)
{
    if (text.length() > 0)
    {
        setText(text);
        if (font)
            setFont(*font);
    }
    setStyleSheet("QLabel { background-color : white; color : #333333; }");
    setGeometry(QRect(rect.x(), rect.y()+pos, rect.width(), height));

    /* if text is too big, align left so that we can at least read the beginning : */
    if (this->text().length() > 0 && this->fontMetrics().width(this->text()) >= rect.width())
        setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    else
        setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        
    setVisible(true);
}
