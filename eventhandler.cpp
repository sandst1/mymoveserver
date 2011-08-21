/*
 * MyMoves - Configurable gestures for Harmattan
 * Copyright (C) 2011 Topi Santakivi <topi.santakivi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// The XRecord interaction of this class is based on XMacro
// by Qball Cow: http://download.sarine.nl/xmacro/Description.html

#include <QtDebug>
#include "mymoveserver.h"
#include "eventhandler.h"

typedef struct
{
        int x, y;
        Display *LocalDpy, *RecDpy;
        XRecordContext rc;
        XRecordRange* rr;
        XRecordClientSpec rcs;
} Priv;
Priv priv;

EventHandler::EventHandler(MyMoveServer* srv, QObject *parent) :
    QThread(parent)
{
    EventHandler::m_server = srv;
}

MyMoveServer* EventHandler::m_server = NULL;

void EventHandler::run()
{
    XInitThreads();
    int Major, Minor;

     // open the local display twice
     Display * LocalDpy = localDisplay ();
     Display * RecDpy = localDisplay ();

     // get the screens too
     int LocalScreen  = DefaultScreen ( LocalDpy );

     qDebug() << "Server VendorRelease: " << VendorRelease(RecDpy) << endl;
     // does the remote display have the Xrecord-extension?
     if ( ! XRecordQueryVersion (RecDpy, &Major, &Minor ) ) {
           // nope, extension not supported
           qDebug() << ": XRecord extension not supported on server \""
                    << DisplayString(RecDpy) << "\"" << endl;

           // close the display and go away
           XCloseDisplay ( RecDpy );
           exit ( EXIT_FAILURE );
     }

     // print some information
     qDebug() << "XRecord for server \"" << DisplayString(RecDpy) << "\" is version "
              << Major << "." << Minor << "." << endl << endl;;

     // start the main event loop
     this->eventLoop ( LocalDpy, LocalScreen, RecDpy);

     // we're done with the display
     XCloseDisplay ( RecDpy );
     XCloseDisplay ( LocalDpy );

     qDebug() << "GestureHandler Thread exiting." << endl;
}

/****************************************************************************/
/*! Connects to the local display. Returns the \c Display or \c 0 if
    no display could be obtained.
*/
/****************************************************************************/
Display * EventHandler::localDisplay () {

  // open the display
  Display * D = XOpenDisplay ( 0 );

  // did we get it?
  if ( ! D ) {
        // nope, so show error and abort
        qDebug() << "could not open display \"" << XDisplayName ( 0 )
                 << "\", aborting." << endl;
        exit ( EXIT_FAILURE );
  }

  // return the display
  return D;
}


void EventHandler::eventCallback(XPointer priv, XRecordInterceptData *d)
{
    Priv *p=(Priv *) priv;
    unsigned int *ud4, type, detail;
    unsigned char *ud1, type1;
    short *d2, rootx, rooty;

    if (d->category!=XRecordFromServer)
    {
        XRecordFreeData(d);
        return;
    }
    ud1=(unsigned char *)d->data;
    d2=(short *)d->data;
    ud4=(unsigned int *)d->data;

    type1=ud1[0]&0x7F; type=type1;
    rootx=d2[10];
    rooty=d2[11];

    switch (type) {
        case ButtonPress:

            m_server->touchPress(rootx, rooty);
        break;

        case ButtonRelease:
            m_server->touchRelease(rootx, rooty);
        break;

        case MotionNotify:

            m_server->touchMove(rootx, rooty);
        break;
  }

  XRecordFreeData(d);
}

void EventHandler::eventLoop(Display * LocalDpy, int LocalScreen,
                             Display * RecDpy) {

    Window       Root, rRoot, rChild;
    int rootx, rooty, winx, winy;
    unsigned int mmask;
    Bool ret;
    Status sret;

    // get the root window and set default target
    Root = RootWindow ( LocalDpy, LocalScreen );

    ret=XQueryPointer(LocalDpy, Root, &rRoot, &rChild, &rootx, &rooty, &winx, &winy, &mmask);
    qDebug() << "XQueryPointer returned: " << ret << endl;
    priv.rr=XRecordAllocRange();
    if (!priv.rr)
    {
        qDebug() << "Could not alloc record range, aborting." << endl;
        exit(EXIT_FAILURE);
    }
    priv.rr->device_events.first=KeyPress;
    priv.rr->device_events.last=MotionNotify;
    priv.rcs=XRecordAllClients;
    priv.rc=XRecordCreateContext(RecDpy, 0, &priv.rcs, 1, &priv.rr, 1);
    if (!priv.rc)
    {
        qDebug() << "Could not create a record context, aborting." << endl;
        exit(EXIT_FAILURE);
    }
    priv.x=rootx;
    priv.y=rooty;
    priv.LocalDpy=LocalDpy;
    priv.RecDpy=RecDpy;

    qDebug("Entering XRecord loop...");

    //if (!XRecordEnableContextAsync(RecDpy, priv.rc, GestureHandler::eventCallback, (XPointer) &priv))
    if (!XRecordEnableContext(RecDpy, priv.rc, EventHandler::eventCallback, (XPointer) &priv))
    {
        qDebug() << "Could not enable the record context, aborting." << endl;
        exit(EXIT_FAILURE);
    }
}

