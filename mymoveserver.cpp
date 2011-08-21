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
#include <QDBusConnection>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QOrientationReading>

#include "mymoveserver.h"
#include "eventhandler.h"
#include <math.h>

#define PEARSON_THRESHOLD 0.75
#define GESTURES_PATH "/home/user/MyDocs/moves"
#define NAME_FILTER "mymove*"

/*
qDebug("MyMoveServer::stopObserving");
//    XRecordGetContext (Display *display, XRecordContext context, XRecordState **state_return)
XRecordContext ctx;
XRecordState* state;
qDebug("opening display 0 and disabling context");
Display* d = XOpenDisplay(0);
XRecordGetContext(d, ctx, &state);
XRecordDisableContext(d, ctx);
qDebug("MyMoveServer::stopObserving OUT");
*/

MyMoveServer::MyMoveServer(QObject *parent) :
    QObject(parent),
    m_state(IDLE),
    m_eh(this),
    m_gesture(),
    m_recBox(),
    m_knownGestures(),
    m_orientation(),
    m_portrait(true)
{
    m_orientation.start();

    QOrientationReading* reading = m_orientation.reading();
    if (reading->orientation() == QOrientationReading::TopUp)
    {
        m_portrait = true;
    }
    else if (reading->orientation() == QOrientationReading::RightUp)
    {
        m_portrait = false;
    }
    qDebug("MyMoveServer, portrait: %d", m_portrait);

    connect(&m_orientation, SIGNAL(readingChanged()), this, SLOT(orientationChanged()));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject("/sandst1/mymoves", this, QDBusConnection::ExportAllSlots);
    bus.registerService("org.sandst1.mymoves");
    m_eh.start();

    system("mkdir -p /home/user/MyDocs/moves");
}

void MyMoveServer::touchPress(int x, int y)
{
    qDebug("touchPress: x %d y %d", x, y);
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            m_gesture.clear();
            m_gesture.append(QPoint(x,y));
        break;

        case RECORDING:
            qDebug("Recording");
            if (m_recBox.contains(x,y,true))
            {
                qDebug("Press inside the record box");
                m_gesture.clear();
                m_gesture.append(QPoint(x,y));
            }
        break;

        case IDLE:
        case SAVING:
        case RECOGNIZING:
            qDebug("idle or saving, ignoring event");
            // Ignore events
        break;

        default:
        break;
    }
}

void MyMoveServer::touchRelease(int x, int y)
{
    qDebug("MyMoveServer::touchRelease");
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            m_gesture.append(QPoint(x,y));
            qDebug("trying to recognize the gesture");
            m_state = RECOGNIZING;
            recognizeGesture();
        break;

        case RECORDING:
            qDebug("Recording");
            if (m_recBox.contains(x,y,true))
            {
                qDebug("Release inside the record box");
                m_gesture.append(QPoint(x,y));
            }
        break;

        case IDLE:
        case SAVING:
        case RECOGNIZING:
            qDebug("idle or saving, ignoring event");
            // Ignore events
        break;

        default:
        break;
    }

}

void MyMoveServer::touchMove(int x, int y)
{
    qDebug("touchMove: x %d y %d", x, y);
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            m_gesture.append(QPoint(x,y));
        break;

        case RECORDING:
            qDebug("Recording");
            if (m_recBox.contains(x,y,true))
            {
                qDebug("Moving inside the record box");
                m_gesture.append(QPoint(x,y));
            }
        break;

        case IDLE:
        case SAVING:
        case RECOGNIZING:
            qDebug("idle or saving, ignoring event");
            // Ignore events
        break;

        default:
        break;
    }
}

void MyMoveServer::recordGesture(int x, int y, int w, int h)
{
    qDebug("MyMoveServer::recordGesture x: %d y: %d w: %d h: %d", x, y, w, h);
    if (m_state == IDLE || m_state == OBSERVING)
    {
        m_recBox = QRect(x, y, w, h);
        m_state = RECORDING;
    }
}

void MyMoveServer::saveGesture(QString command)
{
    qDebug("MyMoveServer::saveGesture");
    QDir gdir(GESTURES_PATH);
    QStringList namefilter(NAME_FILTER);
    QStringList files = gdir.entryList(namefilter);
    int newgesturenum = files.count();

    if (m_state == RECORDING)
    {
        m_state = SAVING;
        if (m_gesture.length() > 0)
        {
            QFile gestFile(QString(GESTURES_PATH)+"/mymove"+QString::number(newgesturenum));
            gestFile.open(QIODevice::WriteOnly);
            QTextStream gstream(&gestFile);

            gstream << command << endl;
            normalizeGesture(m_gesture);
            for (int i = 0; i < m_gesture.length(); i++)
            {
                gstream << m_gesture[i].x() << " " << m_gesture[i].y() << endl;
            }
            gestFile.close();
        }
        m_state = IDLE;
    }

}

void MyMoveServer::observeGestures()
{    
    qDebug("MyMoveServer::observeGestures");    
    if (m_state == IDLE)
    {
        loadGestures();
        m_state = OBSERVING;
    }
}

void MyMoveServer::stopObserving()
{
    if (m_state == OBSERVING)
    {
        m_state = IDLE;
    }
}

void MyMoveServer::orientationChanged()
{
    QOrientationReading* reading = m_orientation.reading();
    if (reading->orientation() == QOrientationReading::TopUp)
    {
        m_portrait = true;
    }
    else if (reading->orientation() == QOrientationReading::RightUp)
    {
        m_portrait = false;
    }
    qDebug("MyMoveServer::orientationChanged, portrait %d", m_portrait);
}

void MyMoveServer::rotateToPortrait(QList<QPoint>& gesture)
{
    int tmp = 0;
    for (int i=0; i < gesture.length(); i++)
    {
        tmp = gesture[i].x();
        gesture[i].rx() = -gesture[i].ry();
        gesture[i].ry() = tmp;
    }
}

void MyMoveServer::normalizeGesture(QList<QPoint>& gesture)
{
    // Normalize the gesture
    int gw = 0;
    int gh = 0;
    QPoint cp = getCentralPoint(gesture, gw, gh);
    int tmp = 0;
    qDebug("gw: %d, gh: %d", gw, gh);

    for (int i=0; i < gesture.length(); i++)
    {       
        gesture[i].rx() -= cp.x();
        gesture[i].ry() -= cp.y();

        gesture[i].rx() = (int)(gesture[i].x()*(1000.0/gw));
        gesture[i].ry() = (int)(gesture[i].y()*(1000.0/gh));

        if (!m_portrait)
        {
            tmp = gesture[i].x();
            gesture[i].rx() = gesture[i].y();
            gesture[i].ry() = tmp;
        }

        qDebug("%d %d", gesture[i].x(), gesture[i].y());
    }
}

QPoint MyMoveServer::getCentralPoint(const QList<QPoint>& list, int& width, int& height)
{
    int minx = 0;
    int maxx = 0;
    int miny = 0;
    int maxy = 0;
    int x = 0;
    int y = 0;
    for (int i=0; i < list.length(); i++)
    {
        QPoint p = list.at(i);
        x = p.x(); y = p.y();
        if (x < minx ) minx = x;
        else if (x > maxx) maxx = x;
        if (y < miny ) miny = y;
        else if (y > maxy) maxy = y;
    }

    width = maxx - minx;
    height = maxy - miny;

    return QPointF((minx+maxx)*0.5, (maxy+miny)*0.5).toPoint();
}

void MyMoveServer::loadGestures()
{
    m_knownGestures.clear();
    qDebug("MyMoveServer::loadGestures");
    QDir gdir(GESTURES_PATH);
    QStringList namefilter(NAME_FILTER);
    QStringList files = gdir.entryList(namefilter);
    qDebug("Loading %d gestures", files.count());

    for (int i = 0; i < files.count(); i++)
    {
        QFile gestFile(QString(GESTURES_PATH) + "/" + files.at(i));
        QString line;
        int x = 0;
        int y = 0;
        gestFile.open(QIODevice::ReadOnly);
        QTextStream gstream(&gestFile);
        Gesture gesture;
        // Read the command attached to this gesture
        line = gstream.readLine();
        gesture.command = line;
        qDebug() << "Command for this gesture: " << gesture.command;
        line = gstream.readLine();
        do
        {
            QStringList slist = line.split(" ");
            x = slist.at(0).toInt();
            y = slist.at(1).toInt();
            gesture.data.append(QPoint(x,y));
            qDebug("Read x %d y %d", x, y);
            line = gstream.readLine();
        } while (!line.isEmpty());
        gestFile.close();

        m_knownGestures.append(gesture);
    }
}

void MyMoveServer::recognizeGesture()
{
    normalizeGesture(m_gesture);
    QString cmdToRun;
    double maxP = 0.0;

    for (int i = 0; i < m_knownGestures.length(); i++)
    {
        QList<QPoint> gesture = m_knownGestures[i].data;
        qDebug("Gesture length: %d, known gesture length: %d", m_gesture.length(), gesture.length());
        // Eliminate too short or too long gesture
        if (abs(gesture.length()-m_gesture.length()) > 0.3*gesture.length())
        {
            qDebug("Gesture length invalid, continuing with another gesture");
            continue;
        }

        double p = pearson(m_gesture, gesture);

        if ( p >= PEARSON_THRESHOLD && p > maxP )
        {
            maxP = p;
            cmdToRun = m_knownGestures[i].command.toLatin1();
        }
    }
    system(cmdToRun.toLatin1());

    m_state = OBSERVING;
}

double MyMoveServer::pearson(const QList<QPoint>& gx, const QList<QPoint>& gy)
{
    qDebug("MyMoveServer::pearson");
    int n = gx.length() < gy.length() ? gx.length() : gy.length();

    int sumx = 0;
    int sumy = 0;
    int ssumx = 0;
    int ssumy = 0;
    int sumxy = 0;

    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    for (int i = 0; i < n; i++)
    {
        x1 = gx[i].x();

        if (m_portrait)
        {
            x2 = gx[i].y();
        }
        else
        {
            x2 = gx[n-1-i].y();
        }

        y1 = gy[i].x();
        y2 = gy[i].y();

        sumx += x1 + x2;
        sumy += y1 + y2;

        ssumx += x1*x1 + x2*x2;
        ssumy += y1*y1 + y2*y2;

        sumxy += x1*y1 + x2*y2;
    }

    // There are 2*n points to compare (both x&y coords)
    n *= 2;

    double nom = sumxy - ((1.0/n)*sumx*sumy);
    double den = sqrt(ssumx - ((1.0/n)*sumx*sumx))*sqrt(ssumy - ((1.0/n)*sumy*sumy));

    double p = nom / den;

    qDebug("pearson: %.2f", p);
    return p;
}
