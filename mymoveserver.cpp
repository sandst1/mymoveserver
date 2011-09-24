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
#include <qmath.h>

#define PEARSON_THRESHOLD 0.75
#define GESTURES_PATH "/home/user/MyDocs/moves"
#define NAME_FILTER "mymove*"
#define MAX_GESTURE_LENGTH_DIFF 0.4

#define GESTURE_RECOGNITION_THRESHOLD 0.90
#define FALSE_RECOGNITION_THRESHOLD 0.20
#define PINCH_DETECTION_THRESHOLD 0.50
#define FINGERS_TOGETHER_DISTANCE 200

#define GESTURES_CONF_FILE "/home/user/MyDocs/.moves/mymoves.conf"

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
    m_gesturesSingle(),
    m_gesturesDouble(),
    m_gesturesTriple(),
    m_orientation(),
    m_portrait(true),
    //m_gestureNN1(NULL),
    m_gestureNN2(NULL),
    m_gestureNN3(NULL),
    m_gestArray(),
    m_f11(),
    m_f12(),
    m_f21(),
    m_f22()
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

    //m_gestureNN1 = fann_create_from_file("/opt/mymoveserver/mymoves_nn1.net");
    m_gestureNN2 = fann_create_from_file("/opt/mymoveserver/mymoves_nn2.net");
    m_gestureNN3 = fann_create_from_file("/opt/mymoveserver/mymoves_nn3.net");

    system("mkdir -p /home/user/MyDocs/.moves");

#ifndef ANN_TRAINING
    observeGestures();
#endif
}

MyMoveServer::~MyMoveServer()
{
    //fann_destroy(m_gestureNN1);
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

        case COLLECTING_DATA:
            clearGesture();
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
        break;

        case IDLE:
        case RECOGNIZING:
            qDebug("idle, ignoring event");
            // Ignore events
        break;

        default:
        break;
    }
}

double MyMoveServer::dist(const QPoint& p1, const QPoint& p2)
{
    return qSqrt(qPow(p1.x()-p2.x(),2)+qPow(p1.y()-p2.y(),2));
}

bool MyMoveServer::isPinch()
{
    qDebug("isPinch points f11 %d %d, f21 %d %d, f12 %d %d, f22 %d %d", m_f11.x(),m_f11.y(),m_f21.x(),m_f21.y(),m_f12.x(),m_f12.y(),m_f22.x(),m_f22.y());
    double d1 = dist(m_f11, m_f21);
    double d2 = dist(m_f12, m_f22);
    qDebug("isPinch, lengths d1 %.2f and d2 %.2f", d1, d2);

    double frac = 0.0;
    double maxdist = 0.0;
    if (d1 < d2)
    {
        frac = d1/d2;
        //qDebug("startdist/enddist: %.2f", frac);
        maxdist = d2;
    }
    else
    {
        frac = d2/d1;
        maxdist = d1;
        //qDebug("startdist/enddist: %.2f", frac);
    }

    qDebug("isPinch, frac %.2f", frac);
    return (frac < PINCH_DETECTION_THRESHOLD && maxdist > FINGERS_TOGETHER_DISTANCE);

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

            qDebug("points length %d", points.length());
            // Check if releasing a dual-touch gesture
            if (m_fingerAmount == 2)
            {
                m_f12 = m_gesture[0].at(m_gesture[0].length()-1);
                m_f22 = m_gesture[1].at(m_gesture[1].length()-1);
                if (isPinch())
                {
                    qDebug("Pinch/unpinch gesture detected!");
                    return;
                }
            }

            // Recognition currently supported in portrait mode only
            if (m_portrait)
            {
                qDebug("trying to recognize the gesture");
                m_state = RECOGNIZING;
                //recognizeGesture();
                recognizeWithNN();
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
        case RECOGNIZING:
            qDebug("idle, ignoring event");
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
                // Check if this is a dual-touch gesture
                if (m_fingerAmount == 2)
                {
                    m_f11 = points[0];
                    m_f21 = points[1];
                }
            }
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
        break;

        case COLLECTING_DATA:
            for (int i = 0; i < points.length(); i++)
            {
                m_gesture[i].append(points[i]);
            }
        break;

        case IDLE:
        case RECOGNIZING:
            qDebug("idle, ignoring event");
            // Ignore events
        break;

        default:
        break;
    }
}

bool MyMoveServer::CentralPointLessThan(const CentralPoint& a, const CentralPoint& b)
{
    return ((a.point.x() < b.point.x()) && (a.point.y() < b.point.y()));
}

int MyMoveServer::serverStatus()
{
    return (int)m_state;
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
    //qDebug("gw: %d, gh: %d", gw, gh);

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

        //qDebug("%d %d", gesture[i].x(), gesture[i].y());
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
    if (!QFile::exists(GESTURES_CONF_FILE))
    {
        return;
    }

    m_gesturesSingle.clear();
    m_gesturesDouble.clear();
    m_gesturesTriple.clear();
    QFile gfile(GESTURES_CONF_FILE);
    gfile.open(QIODevice::ReadOnly);
    QTextStream stream(&gfile);
    // Read the version
    QString version = stream.readLine();

    QString line = stream.readLine();
    do
    {
        QStringList data = line.split("###");
        QString id;
        QString app;
        QString command;
        bool reserved = false;
        qDebug("### Gesture ###");
        for (int i = 0; i < data.size(); i++)
        {
            qDebug("data %d %s", i, data.at(i).toLatin1().data());
        }

        id = data.at(0);
        app = data.at(1);
        command = data.at(2);
        if (app.size() > 0)
        {
            reserved = true;
        }
        qDebug("### Gesture END ###");

        Gesture gest;
        gest.command = command;
        if (id.at(0) == 'd')
        {
            qDebug() << "Appending to double gestures: " << id << ", " << command;
            m_gesturesDouble.append(gest);
        }
        else if (id.at(0) == 't')
        {
            qDebug() << "Appending to triple gestures: " << id << ", " << command;
            m_gesturesTriple.append(gest);
        }
        else
        {
            qDebug() << "Appending to single gestures: " << id << ", " << command;
            m_gesturesSingle.append(gest);
        }

        /*GestureItem* item = new GestureItem(id, image, this);
        item->setApp(app);
        item->setCommand(command);
        item->setReserved(reserved);
        this->appendRow(item);*/

        line = stream.readLine();
    } while (!line.isEmpty());
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
    qDebug("MyMoveServer::recognizeWithNN: finger vectors 1 %d 2 %d 3 %d", m_gesture[0].length(),m_gesture[1].length(),m_gesture[2].length());

    // Check if the user has been doing a two-finger gesture but
    // has accidentally touched the screen with a third finger
    if (m_fingerAmount == 3 && m_gesture[2].length() < 5)
    {
        m_fingerAmount = 2;
    }
    // Also check the same for 3-finger gesture
    if (m_fingerAmount == 4 && m_gesture[3].length() < 5)
    {
        m_fingerAmount = 3;
    }

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
    QList<Gesture>* gestureList = NULL;
    switch(m_fingerAmount)
    {
        /*case 1:
            network = m_gestureNN1;
            gestureList = &m_gesturesSingle;
        break;*/

        case 2:
            network = m_gestureNN2;
            gestureList = &m_gesturesDouble;
        break;

        case 3:
            network = m_gestureNN3;
            gestureList = &m_gesturesTriple;
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

    int matchingIdx = -1;
    int matches = 0;
    bool falseRecognitions = false;
    for (int i = 0; i < outputs; i++)
    {
        if (results[i] >= GESTURE_RECOGNITION_THRESHOLD)
        {
            matches++;
            matchingIdx = i;
        }
        else if (results[i] >= FALSE_RECOGNITION_THRESHOLD)
        {
            falseRecognitions = true;
        }
        qDebug("Gesture %d, result: %.2f", i, results[i]);
    }

    QString command("");
    if (matches == 1 && !falseRecognitions)
    {
        qDebug("Found a single match: %d", matchingIdx);
        if (matchingIdx < gestureList->size())
        {
            qDebug() << "Command to execute: " << gestureList->at(matchingIdx).command;
            command = gestureList->at(matchingIdx).command;
        }
    }

    m_state = OBSERVING;
    if (command.length() > 0)
    {
        command = command + " &";
        system(command.toLatin1());
    }
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
