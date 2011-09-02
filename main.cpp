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
#include <QtCore/QCoreApplication>

#include "mymoveserver.h"
#include <QStringList>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef ANN_TRAINING
    QStringList args = a.arguments();
    if (args.count() != 4)
    {
        qDebug("defaulting to 0. please use ./prog <gestnum> <gest amount> <samples_per_gesture>");
        MyMoveServer::setGestureNumber(0);
        MyMoveServer::setGestureAmount(0);
        MyMoveServer::setGestureSamples(0);
    }
    else
    {
        MyMoveServer::setGestureNumber(args.at(1).toInt());
        MyMoveServer::setGestureAmount(args.at(2).toInt());
        MyMoveServer::setGestureSamples(args.at(3).toInt());
    }

#endif

    MyMoveServer* mms = new MyMoveServer(&a);
    return a.exec();
}
