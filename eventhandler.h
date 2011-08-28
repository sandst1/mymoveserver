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

#include <QPoint>
#include <QThread>
#include <QTime>
#include <X11/extensions/XInput2.h>

class MyMoveServer;
class EventHandler : public QThread
{
    Q_OBJECT
public:
    explicit EventHandler(MyMoveServer* srv, QObject *parent = 0);

    void run();

signals:
    void touchPress(QList<QPoint> points);
    void touchRelease(QList<QPoint> points);
    void touchMove(QList<QPoint> points);

public slots:

private:
    void parseTouchPoints(XIValuatorState valuators, QList<QPoint>& points);
    static MyMoveServer* m_server;

    QTime m_time;
    QPoint m_curPoint;
    QPoint m_prevPoint;
    double m_targetDist;
    double m_distance;

};

#endif // EVENTHANDLER_H
