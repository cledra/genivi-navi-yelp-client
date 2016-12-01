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
    cout << "  -i, --information-screen                        display info screen about selection" << endl;
    cout << "  -k, --keyboard                                  display a virtual keyboard" << endl;
    cout << "  -h, --help                                      this help message" << endl;
}

static struct option long_options[] = {
    {"credentials",             required_argument,  0,  'c' },
    {"information-screen",      no_argument,        0,  'i' },
    {"keyboard",                no_argument,        0,  'k' },
    {"help",                    no_argument,        0,  'h' },
    {0,                         0,                  0,  '\0'}
};

int main(int argc, char *argv[])
{
    int opt;
    QApplication a(argc, argv);
    MainApp mainapp;
    QString credentialsFile(DEFAULT_CREDENTIALS_FILE);

    /* first, parse options : */
    while ((opt = getopt_long(argc, argv, "c:ikh", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'c':
                credentialsFile = QString(optarg);
                break;
            case 'i':
                mainapp.setInfoScreen(true);
                break;
            case 'k':
                mainapp.setKeyboard(true);
                break;
            case 'h':
                usage();
                return 0;
            default: break;
        }
    }
    
    /* then, check that Genivi API is available: */
    if (mainapp.CheckGeniviApi() < 0)
    {
        cerr << "Error: Genivi API is  not available" << endl;
        return -1;
    }

    /* then, authenticate connexion to POI service: */
    if (mainapp.AuthenticatePOI(credentialsFile) < 0)
    {
        cerr << "Error: POI server authentication failed" << endl;
        return -1;
    }

    cerr << "authentication succes !" << endl;

    /* now, let's start monitor user inut (register callbacks): */
    if (mainapp.StartMonitoringUserInput() < 0)
        return -1;

    /* main loop: */
    return a.exec();
}
