#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QFont>
#include <QEventLoop>
#include <QPixmap>
#include <QImageReader>
#include <QBoxLayout>
#include <QPushButton>
#include "InfoPanel.h"
#include "traces.h"

#define FONT_SIZE_INFO      24
#define STARS_IMG_OFFSET    1520
#define STARS_IMG_HEIGHT    69
#define STARS_IMG_WIDTH     324

InfoPanel::InfoPanel(QWidget *window, QBoxLayout *layout, Business & business):
            QObject(),
            pLayout(layout),
            nameLabel(window),
            imageLabel(window),
            addressLabel(window),
            zipCodeCityLabel(window),
            phoneLabel(window),
            imgRatingLabel(window),
            nbReviewsLabel(window),
            networkManager(window),
            hLayout(window),
            BackButton("Cancel", NULL),
            GoButton("Go !", window)
{
    QFont font = nameLabel.font();
    font.setPointSize(FONT_SIZE_INFO-4);

    QFont fontBold = nameLabel.font();
    fontBold.setPointSize(FONT_SIZE_INFO+2);
    fontBold.setBold(true);

    /* Display Name: */
    nameLabel.setText(business.Name);
    nameLabel.setFont(fontBold);
    layout->addWidget(&nameLabel);
    layout->setAlignment(&nameLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Display Address: */
    addressLabel.setText(business.Address);
    addressLabel.setFont(font);
    layout->addWidget(&addressLabel);
    layout->setAlignment(&addressLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Display Zip code + city: */
    zipCodeCityLabel.setText(business.ZipCode + QString(", ") + business.City);
    zipCodeCityLabel.setFont(font);
    layout->addWidget(&zipCodeCityLabel);
    layout->setAlignment(&zipCodeCityLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Display phone number: */
    phoneLabel.setText(business.Phone);
    phoneLabel.setFont(font);
    layout->addWidget(&phoneLabel);
    layout->setAlignment(&phoneLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Image Url: */
    TRACE_INFO("Image URL: %s", qPrintable(business.ImageUrl));
    QEventLoop eventLoop;
    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req(QUrl(business.ImageUrl));
    QNetworkReply* reply = networkManager.get(req);

    TRACE_DEBUG("start waiting...");
    eventLoop.exec(); // wait for answer
    if (reply->error() == QNetworkReply::NoError)
    {
        TRACE_DEBUG("got an answer");
        QByteArray jpegData = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(jpegData);
        imageLabel.setPixmap(pixmap.scaled(QSize(window->width() / 2, window->width() / 2), Qt::KeepAspectRatio));
        layout->addWidget(&imageLabel);
        layout->setAlignment(&imageLabel, Qt::AlignTop | Qt::AlignHCenter);
    }
    delete reply;

    /* Display number of reviews: */
    nbReviewsLabel.setText(QString("Number of reviews : %1").arg(business.ReviewCount));
    nbReviewsLabel.setFont(font);
    layout->addWidget(&nbReviewsLabel);
    layout->setAlignment(&nbReviewsLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Rating image: */
    QImageReader reader(QString(":/images/stars_map_www.png"));
    reader.setClipRect(QRect(0,
        STARS_IMG_OFFSET +((int)((double)business.Rating*2)-1)*STARS_IMG_HEIGHT, STARS_IMG_WIDTH, STARS_IMG_HEIGHT));
    const QImage image = reader.read();
    imgRatingLabel.setPixmap(QPixmap::fromImage(image).scaled(QSize(window->width() / 4, 69), Qt::KeepAspectRatio));
    layout->addWidget(&imgRatingLabel);
    layout->setAlignment(&imgRatingLabel, Qt::AlignTop | Qt::AlignHCenter);

    /* Buttons : */
    hLayout.addWidget(&BackButton);
    hLayout.addWidget(&GoButton);
    layout->addLayout(&hLayout);
    BackButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    GoButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    BackButton.setMinimumSize(QSize(window->width()/4, 75));
    GoButton.setMinimumSize(QSize(window->width()/4, 75));
    BackButton.setFont(font);
    GoButton.setFont(font);

    connect(&BackButton, SIGNAL(released()), window, SLOT(cancelClicked()));
    connect(&GoButton,   SIGNAL(released()), window, SLOT(goClicked()));
}

InfoPanel::~InfoPanel()
{
    pLayout->removeWidget(&nameLabel);
    pLayout->removeWidget(&imageLabel);
    pLayout->removeWidget(&addressLabel);
    pLayout->removeWidget(&zipCodeCityLabel);
    pLayout->removeWidget(&phoneLabel);
    pLayout->removeWidget(&imgRatingLabel);
    pLayout->removeWidget(&nbReviewsLabel);
}
