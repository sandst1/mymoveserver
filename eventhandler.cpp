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

#define TARGET_SPEED 3.50

EventHandler::EventHandler(MyMoveServer* srv, QObject *parent) :
    QThread(parent)
{
    EventHandler::m_server = srv;
}

MyMoveServer* EventHandler::m_server = NULL;

void EventHandler::parseTouchPoints(XIValuatorState valuators, QList<QPoint>& points)
{
    double *val = valuators.values;
    int x = 0;
    int y = 0;
    for (int i = 0; i < valuators.mask_len * 8; i++)
    {
        if (XIMaskIsSet(valuators.mask, i) && (i % 5 == 0))
        {
            x = (int)(*val++);
            y = (int)(*val);
            points.push_back(QPoint(x,y));
        }
    }
}

void EventHandler::run()
{
    qDebug("EventHandler::run");
    XInitThreads();
    Display *display;
    int xi_opcode, event, error;

    display = XOpenDisplay(NULL);

    if (!display)
    {
        qDebug("Could not open display.");
        return;
    }

    if (!XQueryExtension(display, "XInputExtension",
                         &xi_opcode, &event, &error))
    {
        qDebug("XInputExtension is not available.");
        return;
    }

    XIEventMask mask;
    XSelectInput(display, DefaultRootWindow(display), ExposureMask);
    mask.deviceid = XIAllDevices;
    mask.mask_len = sizeof(mask);
    mask.mask = (unsigned char*)calloc(mask.mask_len, sizeof(char));
    XISetMask(mask.mask, XI_Motion);
    XISetMask(mask.mask, XI_ButtonPress);
    XISetMask(mask.mask, XI_ButtonRelease);
    XISelectEvents(display, DefaultRootWindow(display), &mask, 1);

    free(mask.mask);
    XMapWindow(display, DefaultRootWindow(display));
    XSync(display, True);

    while(1)
    {
        XEvent ev;
        XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
        XNextEvent(display, (XEvent*)&ev);

        if (XGetEventData(display, cookie) &&
            cookie->type == GenericEvent &&
            cookie->extension == xi_opcode)
        {
            switch (cookie->evtype)
            {
                case XI_ButtonPress:
                {
                    XIDeviceEvent *e = (XIDeviceEvent*)(cookie->data);
#ifndef ANN_TRAINING
                    qDebug("XI_ButtonPress, x: %.2f, y: %.2f", e->root_x, e->root_y);
#endif
                    QList<QPoint> points;
                    parseTouchPoints(e->valuators, points);

                    m_targetDist = 0.0;
                    m_distance = 0.0;
                    m_time.restart();

                    emit this->touchPress(points);
                }
                break;
                case XI_ButtonRelease:
                {
                    XIDeviceEvent *e = (XIDeviceEvent*)(cookie->data);
#ifndef ANN_TRAINING
                    qDebug("Button release, x: %.2f, y: %.2f", e->root_x, e->root_y);
#endif
                    QList<QPoint> points;
                    parseTouchPoints(e->valuators, points);
                    //qDebug("%d ms since previous event", m_time.elapsed());

                    //m_server->touchRelease(points);
                    emit this->touchRelease(points);
                 }
                break;
                case XI_Motion:
                {
                    XIDeviceEvent *e = (XIDeviceEvent*)(cookie->data);
#ifndef ANN_TRAINING
                    qDebug("XI_Motion, x: %.2f, y: %.2f", e->root_x, e->root_y);
#endif
                    QList<QPoint> points;
                    parseTouchPoints(e->valuators, points);

                    m_targetDist = m_time.elapsed()*TARGET_SPEED;
                    m_curPoint = QPoint(e->root_x, e->root_y);

                    m_distance += QPointF(m_curPoint-m_prevPoint).manhattanLength();
                    m_prevPoint = m_curPoint;
                    if ( m_distance >= m_targetDist )
                    {
                        emit this->touchMove(points);
                        m_distance = 0;
                    }
                    m_time.restart();
                }
                break;

                default:
                break;
            }
        }
        XFreeEventData(display, cookie);
    }

    qDebug() << "GestureHandler Thread exiting." << endl;
}
