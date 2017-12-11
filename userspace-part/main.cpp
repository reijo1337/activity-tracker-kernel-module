#include <QCoreApplication>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <iostream>
#include <iostream>
#include <fstream>
#include <QTime>

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


int main(int argc, char *argv[])
{
    std::ofstream myfile;
    myfile.open ("/proc/time-tracker");
    setlocale( LC_ALL, "" );

    while (true) {
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
                QString toEcho = "echo ";
                toEcho += "\""+ out + "\"" + " >> /proc/time-tracker";
                system(toEcho.toStdString().c_str());
            }

            if( wins ) {
                XFree( wins );
            }

            XCloseDisplay( display );
        }
        delay();
    }
    return 0;
}
