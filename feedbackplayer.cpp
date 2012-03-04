/***************************************************************************
**
** Copyright (C) 2010, 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of libmeegotouch.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "feedbackplayer.h"

#include <QDir>
#include <QString>
#include <QTimer>

namespace {
    const QString FeedbackProtocolVersion("FeedbackProtocolVersion#1");
    const QString SocketServer("/tmp/mfeedbackd/player.sock");
    const int fastReconnectionTime = 1000;
    const int maximumFastReconnectionCount = 10;
}

FeedbackPlayer::FeedbackPlayer(QObject *parent)
    : QObject(parent),
      previousSuccessfullConnection(QElapsedTimer())
{
    socketStream.setDevice(&socket);
    connect(&socket, SIGNAL(connected()), SLOT(onConnected()));
    connect(&socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            SLOT(onSocketError(QLocalSocket::LocalSocketError)));

    reconnectionAttempts = 0;
    fastReconnectionCount = 0;
    previousSuccessfullConnection.invalidate();

    reconnectionIntervalsList << 10;
    reconnectionIntervalsList << 100;
    reconnectionIntervalsList << 300;
    reconnectionIntervalsList << 600;
    reconnectionIntervalsList << 1000;
    reconnectionIntervalsList << 1000;
    reconnectionIntervalsList << 2000;
    reconnectionIntervalsList << 2000;
    reconnectionIntervalsList << 5000;
    reconnectionIntervalsList << 5000;
    reconnectionIntervalsList << 10000;
    reconnectionIntervalsList << 10000;
    reconnectionIntervalsList << 30000;
    reconnectionIntervalsList << 30000;
    reconnectionIntervalsList << 300000;
    reconnectionIntervalsList << 300000; // 5*60*1000 (five minutes)
}

FeedbackPlayer::~FeedbackPlayer()
{
    // socket emits disconnected() in its destructor. If we don't
    // disconnect from this signal we will get called after being
    // destroyed which will cause a SIGSEGV
    socket.disconnect(this);
}

bool FeedbackPlayer::init(const QString &applicationName)
{
    this->applicationName = applicationName;

    // Initial connection to server is immediate
    connectIdle();

    return true;
}

void FeedbackPlayer::playFeedback(const QString &name)
{
    if (socket.state() == QLocalSocket::ConnectedState &&
        name.isEmpty() == false) {
        socketStream << name;
        socket.flush();
    }
}

void FeedbackPlayer::onConnected()
{
    reconnectionAttempts = 0;
    socketStream << FeedbackProtocolVersion;
    socketStream << applicationName;

    previousSuccessfullConnection.start();
}

void FeedbackPlayer::connectIdle()
{
    socket.connectToServer(SocketServer);
}

void FeedbackPlayer::onSocketError(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError);

    if (reconnectionAttempts == 0) {
        if (previousSuccessfullConnection.isValid() &&
            previousSuccessfullConnection.elapsed() < fastReconnectionTime) {
            // Increment fast reconnection count if time since previous succesfull
            // reconnection is less than fastReconnectionTime.
            fastReconnectionCount++;
        } else {
            fastReconnectionCount = 0;
        }
    }

    if (reconnectionAttempts < reconnectionIntervalsList.size() &&
        fastReconnectionCount <= maximumFastReconnectionCount) {
        // Try to reconnect to mfeedbackd
        QTimer::singleShot(reconnectionIntervalsList[reconnectionAttempts],
                           this, SLOT(connectIdle()));
        reconnectionAttempts++;
    } else {
        // Give up
        qWarning("FeedbackPlayer: Couldn't establish connection with mfeedbackd.");
    }
}
