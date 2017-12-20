#include <QCoreApplication>
#include <QTextStream>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <iostream>
#include <iostream>
#include <fstream>

#include <QTime>
#include <QFile>

#include "apue.h"
#include <syslog.h>
#include <errno.h>
#include "all.h"

Window* findWindows( Display* display, ulong* winCount ) {
    Atom actualType;
    int format;
    ulong bytesAfter;
    uchar* list = NULL;
    Status status = XGetWindowProperty(
                        display,
                        DefaultRootWindow( display ),
                        XInternAtom( display, "_NET_ACTIVE_WINDOW", False ),
                        0L,
                        ~0L,
                        False,
                        XA_WINDOW,
                        &actualType,
                        &format,
                        winCount,
                        &bytesAfter,
                        &list
                    );

    if( status != Success ) {
        *winCount = 0;
        return NULL;
    }

    return reinterpret_cast< Window* >( list );
}

char* getWindowName( Display* display, Window win ) {
    Atom actualType;
    int format;
    ulong count, bytesAfter;
    uchar* name = NULL;
    Status status = XGetWindowProperty(
                        display,
                        win,
                        XInternAtom( display, "_NET_WM_NAME", False ),
                        0L,
                        ~0L,
                        False,
                        XInternAtom( display, "UTF8_STRING", False ),
                        &actualType,
                        &format,
                        &count,
                        &bytesAfter,
                        &name
                    );

    if( status != Success ) {
        return NULL;
    }

    if( name == NULL ) {
        Status status = XGetWindowProperty(
                            display,
                            win,
                            XInternAtom( display, "WM_NAME", False ),
                            0L,
                            ~0L,
                            False,
                            AnyPropertyType,
                            &actualType,
                            &format,
                            &count,
                            &bytesAfter,
                            &name
                        );

        if( status != Success ) {
            return NULL;
        }
    }

    return reinterpret_cast< char* >( name );
}

char* getWindowClass( Display* display, Window win ) {
    Atom actualType;
    int format;
    ulong count, bytesAfter;
    uchar* name = NULL;
    Status status = XGetWindowProperty(
                        display,
                        win,
                        XInternAtom( display, "_NET_WM_CLASS", False ),
                        0L,
                        ~0L,
                        False,
                        XInternAtom( display, "UTF8_STRING", False ),
                        &actualType,
                        &format,
                        &count,
                        &bytesAfter,
                        &name
                    );

    if( status != Success ) {
        return NULL;
    }

    if( name == NULL ) {
        Status status = XGetWindowProperty(
                            display,
                            win,
                            XInternAtom( display, "WM_CLASS", False ),
                            0L,
                            ~0L,
                            False,
                            AnyPropertyType,
                            &actualType,
                            &format,
                            &count,
                            &bytesAfter,
                            &name
                        );

        if( status != Success ) {
            return NULL;
        }
    }

    return reinterpret_cast< char* >( name );
}

void delay()
{
    QTime dieTime= QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

extern int lockfile(int);
extern int already_running(void);

int main(int argc, char *argv[])
{
    char *cmd;

    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    /*
     * Become a daemon.
     */
    daemonize(cmd);

    /*
     * Make sure only one copy of the daemon is running.
     */
    if (already_running()) {
        syslog(LOG_ERR, "daemon already running");
        exit(1);
    }

    while (true) {
        QFile file("/proc/time-tracker");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            syslog(LOG_ERR, "daemon can't open proc file");
            exit(1);
        }

        if( Display* display = XOpenDisplay( NULL ) ) {
            QString out;
            ulong count = 0;
            Window* wins = findWindows( display, &count );
            for( ulong i = 0; i < count; ++i ) {
                Window w = wins[ i ];
                if( char* name = getWindowClass( display, w ) ) {
                    out = QString::fromUtf8( name );
                    XFree( name );
                }
                QTextStream outStream(&file);
                outStream << out;
                syslog(LOG_ERR, "window class writed");
            }

            if( wins ) {
                XFree( wins );
            }

            XCloseDisplay( display );
        }
        file.close();
        delay();
    }

    return 0;
}
