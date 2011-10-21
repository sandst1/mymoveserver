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
#include <QFile>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QRect>
#include "eventhandler.h"
#include <floatfann.h>

QTM_USE_NAMESPACE

#define MAX_FINGERS 6
#define MAX_GESTURE_LENGTH_TWOFINGERS 150
#define MAX_GESTURE_LENGTH_THREEFINGERS 300

class MyMoveServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.sandst1.mymoves")
public:
    explicit MyMoveServer(QObject *parent = 0);
    ~MyMoveServer();

#ifdef ANN_TRAINING
    static void setGestureNumber(int number);
    static void setGestureAmount(int number);
    static void setGestureSamples(int number);
#endif

signals:

public slots:
    void touchPress(QList<QPoint> points);
    void touchRelease(QList<QPoint> points);
    void touchMove(QList<QPoint> points);


    int serverStatus();
    void observeGestures();
    void stopObserving();

    void orientationChanged();
private:
    struct Gesture
    {
        //QList<QPoint> data;
        QString command;
    };

    struct CentralPoint
    {
        QPoint point;
        int index;
    };

    void clearGesture();

    void rotateToPortrait(QList<QPoint>& gesture);
    QPoint normalizeGesture(QList<QPoint>& gesture);
    QPoint getCentralPoint(const QList<QPoint>& list, int& width, int& height);
    void loadGestures();
    void recognizeGesture();    
    double pearson(const QList<QPoint>& gx, const QList<QPoint>& gy);
    void formGestureVector();
    void recognizeWithNN();


    enum State {
        IDLE,
        OBSERVING,
        RECOGNIZING,
        COLLECTING_DATA
    };

    State m_state;
    EventHandler m_eh;
    QList<QPoint> m_gesture[MAX_FINGERS];
    int m_fingerAmount;

    QList<QPoint> m_gestVect;
    QList<QPoint> m_padVect;

    static bool CentralPointLessThan(const CentralPoint& a, const CentralPoint& b);

    QList<Gesture> m_gesturesSingle;
    QList<Gesture> m_gesturesDouble;
    QList<Gesture> m_gesturesTriple;

    QOrientationSensor m_orientation;
    bool m_portrait;    

    //struct fann *m_gestureNN1;
    struct fann *m_gestureNN2;
    struct fann *m_gestureNN3;
    fann_type m_gestArray2[MAX_GESTURE_LENGTH_TWOFINGERS*2];
    fann_type m_gestArray3[MAX_GESTURE_LENGTH_THREEFINGERS*2];

#ifdef ANN_TRAINING
    static int m_gestureNum;
    static int m_gestureAmount;
    static int m_gestureSamples;
    QFile m_gestureFile;
    int m_gestureCount;
    void saveData();
#endif
    double dist(const QPoint& p1, const QPoint& p2);
    bool isPinch();

    QPoint m_f11;
    QPoint m_f12;
    QPoint m_f21;
    QPoint m_f22;  

    int m_lastMoveIndex[2];

};

#endif // MYMOVESERVER_H
