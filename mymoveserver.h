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
#ifndef MYMOVESERVER_H
#define MYMOVESERVER_H

#include <QOrientationSensor>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QRect>
#include "eventhandler.h"

QTM_USE_NAMESPACE

class MyMoveServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.sandst1.mymoves")
public:
    explicit MyMoveServer(QObject *parent = 0);

signals:

public slots:
    void touchPress(QList<QPoint> points);
    void touchRelease(QList<QPoint> points);
    void touchMove(QList<QPoint> points);

    void recordGesture(int x, int y, int w, int h);
    void saveGesture(QString command);
    void observeGestures();
    void stopObserving();

    void orientationChanged();
private:
    void rotateToPortrait(QList<QPoint>& gesture);
    void normalizeGesture(QList<QPoint>& gesture);
    QPoint getCentralPoint(const QList<QPoint>& list, int& width, int& height);
    void loadGestures();
    void recognizeGesture();
    double pearson(const QList<QPoint>& gx, const QList<QPoint>& gy);

    enum State {
        IDLE,
        OBSERVING,
        RECOGNIZING,
        RECORDING,
        SAVING
    };

    State m_state;
    EventHandler m_eh;
    QList<QPoint> m_gesture;

    struct Gesture
    {
        QList<QPoint> data;
        QString command;
    };

    QList<Gesture> m_knownGestures;
    QRect m_recBox;

    QOrientationSensor m_orientation;
    bool m_portrait;
};

#endif // MYMOVESERVER_H
