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
#include <QCoreApplication>
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
#define MAX_GESTURE_LENGTH_DIFF 0.4

#ifdef ANN_TRAINING
int MyMoveServer::m_gestureNum = -1;
int MyMoveServer::m_gestureAmount = -1;
int MyMoveServer::m_gestureSamples = -1;

void MyMoveServer::setGestureNumber(int number)
{
    qDebug("Setting gesture number to %d", number);
    m_gestureNum = number;
}

void MyMoveServer::setGestureAmount(int number)
{
    qDebug("Setting gesture amount to %d", number);
    m_gestureAmount = number;
}

void MyMoveServer::setGestureSamples(int number)
{
    qDebug("Setting gesture samples to %d", number);
    m_gestureSamples = number;

}

#endif

MyMoveServer::MyMoveServer(QObject *parent) :
    QObject(parent),
#ifndef ANN_TRAINING
    m_state(IDLE),
#else
    m_state(COLLECTING_DATA),
    m_gestureFile(),
#endif
    m_eh(this),
    m_gesture(),
    m_gestVect(),
    m_recBox(),
    m_knownGestures(),
    m_orientation(),
    m_portrait(true),
    m_gestureNN1(NULL),
    m_gestureNN2(NULL),
    m_gestureNN3(NULL),
    m_gestArray()
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
#ifndef ANN_TRAINING
    qDebug("MyMoveServer, portrait: %d", m_portrait);
#else
    qDebug("Starting in the data collecting mode");
    QString fname("/home/user/MyDocs/gesturedata");
    fname.append(QVariant(m_gestureNum).toString());
    m_gestureFile.setFileName(fname);
    m_gestureCount = 0;
#endif
    connect(&m_orientation, SIGNAL(readingChanged()), this, SLOT(orientationChanged()));

    qRegisterMetaType<QPoint>("QPoint");
    qRegisterMetaType<QList<QPoint> >("QList<QPoint>");

    connect(&m_eh, SIGNAL(touchPress(QList<QPoint>)), this, SLOT(touchPress(QList<QPoint>)));
    connect(&m_eh, SIGNAL(touchMove(QList<QPoint>)), this, SLOT(touchMove(QList<QPoint>)));
    connect(&m_eh, SIGNAL(touchRelease(QList<QPoint>)), this, SLOT(touchRelease(QList<QPoint>)));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject("/sandst1/mymoves", this, QDBusConnection::ExportAllSlots);
    bus.registerService("org.sandst1.mymoves");
    m_eh.start();

    m_gestVect.reserve(MAX_GESTURE_LENGTH);

    QPoint zeroPoint(0,0);
    for (int i = 0; i < MAX_GESTURE_LENGTH; i++)
    {
        m_padVect.push_back(zeroPoint);
    }

    m_gestureNN1 = fann_create_from_file("/opt/mymoveserver/mymoves_nn1.net");
    m_gestureNN2 = fann_create_from_file("/opt/mymoveserver/mymoves_nn2.net");
    m_gestureNN3 = fann_create_from_file("/opt/mymoveserver/mymoves_nn3.net");

    system("mkdir -p /home/user/MyDocs/.moves");
}

MyMoveServer::~MyMoveServer()
{
    fann_destroy(m_gestureNN1);
    fann_destroy(m_gestureNN2);
    fann_destroy(m_gestureNN3);
}

void MyMoveServer::clearGesture()
{
    for (int i = 0; i < MAX_FINGERS; i++)
    {
        m_gesture[i].clear();
    }
    m_gestVect.clear();
    m_fingerAmount = 0;
}

void MyMoveServer::touchPress(QList<QPoint> points)
{   
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            clearGesture();
            for (int i = 0; i < points.length(); i++)
            {                
                m_gesture[i].append(points[i]);
            }
        break;

        case RECORDING:
            qDebug("Recording");
            for (int i = 0; i < points.length(); i++)
            {
                if (m_recBox.contains(points[i], true))
                {
                    qDebug("Press inside the record box");
                    clearGesture();
                    m_gesture[i].append(points[i]);
                }
            }
        break;

        case COLLECTING_DATA:
            clearGesture();
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
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

void MyMoveServer::touchRelease(QList<QPoint> points)
{ 
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
            qDebug("trying to recognize the gesture");
            m_state = RECOGNIZING;
            //recognizeGesture();
            recognizeWithNN();
        break;

        case RECORDING:
            qDebug("Recording");
            for (int i = 0; i < points.length(); i++)
            {
                if (m_recBox.contains(points[i], true))
                {
                    qDebug("Release inside the record box");
                    m_gesture[i].append(points[i]);
                }
            }
        break;

#ifdef ANN_TRAINING
        case COLLECTING_DATA:
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
            qDebug("trying to recognize the gesture");
            m_state = RECOGNIZING;
            // Collect the data
            saveData();
        break;
#endif

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

void MyMoveServer::touchMove(QList<QPoint> points)
{
    switch (m_state)
    {
        case OBSERVING:
            qDebug("Observing");
            if (points.length() > m_fingerAmount)
            {
                m_fingerAmount = points.length();
            }
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
        break;

        case RECORDING:
            qDebug("Recording");

            for (int i = 0; i < points.length(); i++)
            {
                if (m_recBox.contains(points[i], true))
                {
                    qDebug("Moving inside the record box");
                    m_gesture[i].append(points[i]);
                }
            }
        break;

        case COLLECTING_DATA:
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
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

bool MyMoveServer::CentralPointLessThan(const CentralPoint& a, const CentralPoint& b)
{
    return ((a.point.x() < b.point.x()) && (a.point.y() < b.point.y()));
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
        if (m_gesture[0].length() > 0)
        {
            QFile gestFile(QString(GESTURES_PATH)+"/mymove"+QString::number(newgesturenum));
            gestFile.open(QIODevice::WriteOnly);
            QTextStream gstream(&gestFile);

            gstream << command << endl;

            formGestureVector();

            for (int i = 0; i < m_gestVect.length(); i++)
            {
                gstream << m_gestVect[i].x() << " " << m_gestVect[i].y() << endl;
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

QPoint MyMoveServer::normalizeGesture(QList<QPoint>& gesture)
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
    return cp;
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
    qDebug("MyMoveServer::recognizeGesture");
    formGestureVector();

    QString cmdToRun;
    double maxP = 0.0;

    for (int i = 0; i < m_knownGestures.length(); i++)
    {
        QList<QPoint> gesture = m_knownGestures[i].data;
        qDebug("Gesture length: %d, known gesture length: %d", m_gestVect.length(), gesture.length());
        // Eliminate too short or too long gesture
        if (abs(gesture.length()-m_gestVect.length()) > MAX_GESTURE_LENGTH_DIFF*gesture.length())
        {
            qDebug("Gesture length invalid, continuing with another gesture");
            continue;
        }

        double p = pearson(m_gestVect, gesture);

        if ( p >= PEARSON_THRESHOLD && p > maxP )
        {
            maxP = p;
            cmdToRun = m_knownGestures[i].command.toLatin1();
        }
    }
    system(cmdToRun.toLatin1());

    qDebug("Going back to OBSERVING");
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

void MyMoveServer::formGestureVector()
{
    qDebug("MyMoveServer::formGestureVector");
    m_gestVect.clear();
    QList<CentralPoint> centralPoints;
    for (int i = 0; i < MAX_FINGERS; i++)
    {
        if (m_gesture[i].length() > 0)
        {
            CentralPoint cp;
            cp.index = i;
            cp.point = normalizeGesture(m_gesture[i]);
            centralPoints.push_back(cp);
        }
    }

    // Order the gestures
    qSort(centralPoints.begin(), centralPoints.end(), CentralPointLessThan);

    // Push the finger data to m_gestVect in the order of the central points
    for (int i = 0; i < centralPoints.length(); i++)
    {
        m_gestVect.append(m_gesture[centralPoints[i].index]);
    }

    qDebug("Gest vector length: %d", m_gestVect.length());

    if (m_gestVect.length() < MAX_GESTURE_LENGTH)
    {
        qDebug("Padding gesture vector to %d", MAX_GESTURE_LENGTH);
        int padding = MAX_GESTURE_LENGTH - m_gestVect.length();
        m_gestVect.append(m_padVect.mid(0, padding));
        qDebug("Current length %d", m_gestVect.length());
    }
    else if (m_gestVect.length() > MAX_GESTURE_LENGTH)
    {
        qDebug("Capping gesture vector to %d", MAX_GESTURE_LENGTH);
        m_gestVect = m_gestVect.mid(0, MAX_GESTURE_LENGTH);
        qDebug("Current length %d", m_gestVect.length());
    }
}

void MyMoveServer::recognizeWithNN()
{
    qDebug("MyMoveServer::recognizeWithNN");
    formGestureVector();
    for (int i = 0; i < MAX_GESTURE_LENGTH*2; i++)
    {
        if (i%2)
        {
            m_gestArray[i] = m_gestVect[i/2].y();
        }
        else
        {
            m_gestArray[i] = m_gestVect[i/2].x();
        }
        //qDebug("Gest value %.2f", m_gestArray[i]);
    }


    struct fann* network = NULL;
    switch(m_fingerAmount)
    {
        case 1:
            network = m_gestureNN1;
        break;

        case 2:
            network = m_gestureNN2;
        break;

        case 3:
            network = m_gestureNN3;
        break;

        default:
            qDebug("Amount of fingers not supported, returning");
            m_state = OBSERVING;
            return;
        break;
    }

    qDebug("Using neural network %d", m_fingerAmount);
    fann_type* results = fann_run(network, m_gestArray);
    int outputs = fann_get_num_output(network);

    for (int i = 0; i < outputs; i++)
    {
        qDebug("Gesture %d, result: %.2f", i, results[i]);
    }

    m_state = OBSERVING;
    qDebug("MyMoveServer::recognizeWithNN out");
}


#ifdef ANN_TRAINING
void MyMoveServer::saveData()
{
    formGestureVector();
    m_gestureFile.open(QIODevice::Append);
    QTextStream stream(&m_gestureFile);
    for (int i = 0; i < m_gestVect.length(); i++)
    {
        stream << m_gestVect[i].x() << " " << m_gestVect[i].y() << " ";
    }
    //stream << endl << m_gestureNum << endl;
    int zerosBefore = m_gestureNum;
    int zerosAfter = m_gestureAmount - 1 - m_gestureNum;
    stream << endl;
    for (int i = 0; i < zerosBefore; i++)
    {
        stream << "0 ";
    }
    stream << 1 << " ";
    for (int i = 0; i < zerosAfter; i++)
    {
        stream << "0 ";
    }
    stream << endl;

    m_gestureFile.close();

    m_gestureCount++;
    qDebug("Gesture %d: %d gestures saved so far", m_gestureNum, m_gestureCount);
    if (m_gestureCount == m_gestureSamples)
    {
        qDebug("Quitting this round");
        m_eh.terminate();
        m_eh.wait();
        QCoreApplication::exit(0);
    }
    m_state = COLLECTING_DATA;
}
#endif
