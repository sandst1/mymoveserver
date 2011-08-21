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
#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QThread>

#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>

class MyMoveServer;
class EventHandler : public QThread
{
    Q_OBJECT
public:
    explicit EventHandler(MyMoveServer* srv, QObject *parent = 0);

    void run();

signals:

public slots:

private:
    Display* localDisplay ();
    static void eventCallback(XPointer priv, XRecordInterceptData *d);
    void eventLoop (Display * LocalDpy, int LocalScreen,
                    Display * RecDpy);

    static MyMoveServer* m_server;

};

#endif // EVENTHANDLER_H
