#include <QApplication>
#include <iostream>
#include "MainApp.h"
#include <getopt.h>

#define DEFAULT_CREDENTIALS_FILE "/home/root/.credentials.txt"

using namespace std;

static void usage(void)
{
    cout << "Usage:" << endl;
    cout << "  -c, --credentials  <path-to-credentials-file>   set the POI provider credentials file" << endl;
    cout << "                     Credentials file must be formated this way:" << endl;
    cout << "                        AppId=dummy" << endl;
    cout << "                        AppSecret=dummy-secret" << endl;
    cout << "  -h, --help                                      this help message" << endl;
}

static struct option long_options[] = {
    {"credentials",             required_argument,  0,  'c' },
    {"help",                    no_argument,        0,  'h' },
    {0,                         0,                  0,  '\0'}
};

int main(int argc, char *argv[])
{
    int opt;
    QApplication a(argc, argv);
    MainApp mainapp(argc, argv);
    QString credentialsFile(DEFAULT_CREDENTIALS_FILE);

    /* first, parse options : */
    while ((opt = getopt_long(argc, argv, "c:h", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'c':
                credentialsFile = QString(optarg);
                break;
            case 'h':
                usage();
                return 0;
            default: break;
        }
    }
    
    /* then, check that Genivi API is available: */
    if (mainapp.CheckGeniviApi() < 0)
        return -1;

    /* then, authenticate connexion to POI service: */
    if (mainapp.AuthenticatePOI(credentialsFile) < 0)
        return -1;

    cout << "authentication succes !" << endl;

    /* now, let's start monitor user inut (register callbacks): */
    if (mainapp.StartMonitoringUserInput() < 0)
        return -1;

    /* main loop: */
    return a.exec();
}
