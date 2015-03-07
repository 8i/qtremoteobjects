/****************************************************************************
**
** Copyright (C) 2014 Ford Motor Company
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qremoteobjectregistry.h"
#include "qremoteobjectreplica_p.h"

#include <QSet>


QT_BEGIN_NAMESPACE

QRemoteObjectRegistry::QRemoteObjectRegistry(QObject *parent) : QRemoteObjectReplica(parent)
{
    connect(this, &QRemoteObjectRegistry::isReplicaValidChanged, this, &QRemoteObjectRegistry::pushToRegistryIfNeeded);
}

QRemoteObjectRegistry::~QRemoteObjectRegistry()
{}

void QRemoteObjectRegistry::initialize()
{
    qRegisterMetaType<QRemoteObjectSourceLocation>();
    qRegisterMetaTypeStreamOperators<QRemoteObjectSourceLocation>();
    qRegisterMetaType<QRemoteObjectSourceLocations>();
    qRegisterMetaTypeStreamOperators<QRemoteObjectSourceLocations>();
    QVariantList properties;
    properties.reserve(3);
    properties << QVariant::fromValue(QRemoteObjectSourceLocations());
    properties << QVariant::fromValue(QRemoteObjectSourceLocation());
    properties << QVariant::fromValue(QRemoteObjectSourceLocation());
    setProperties(properties);
}

QRemoteObjectSourceLocations QRemoteObjectRegistry::sourceLocations() const
{
    return propAsVariant(0).value<QRemoteObjectSourceLocations>();
}

void QRemoteObjectRegistry::addSource(const QRemoteObjectSourceLocation &entry)
{
    if (hostedSources.contains(entry.first)) {
        qCWarning(QT_REMOTEOBJECT) << "Node warning: Ignoring Source" << entry.first
                                   << "as this Node already has a Source by that name.";
        return;
    }
    hostedSources.insert(entry.first, entry.second);
    if (!isReplicaValid()) {
        return;
    }
    if (sourceLocations().contains(entry.first)) {
        qCWarning(QT_REMOTEOBJECT) << "Node warning: Ignoring Source" << entry.first
                                   << "as another source (" << sourceLocations()[entry.first]
                                   << ") has already registered that name.";
        return;
    }
    qCDebug(QT_REMOTEOBJECT) << "An entry was added to the registry - Sending to Source" << entry.first << entry.second;
    // This does not set any data to avoid a coherency problem between client and server
    static int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("addSource(QRemoteObjectSourceLocation)");
    QVariantList args;
    args << QVariant::fromValue(entry);
    send(QMetaObject::InvokeMetaMethod, index, args);
}

void QRemoteObjectRegistry::removeSource(const QRemoteObjectSourceLocation &entry)
{
    if (!hostedSources.contains(entry.first))
        return;
    hostedSources.remove(entry.first);
    if (!isReplicaValid()) {
        return;
    }
    qCDebug(QT_REMOTEOBJECT) << "An entry was removed from the registry - Sending to Source" << entry.first << entry.second;
    // This does not set any data to avoid a coherency problem between client and server
    static int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("removeSource(QRemoteObjectSourceLocation)");
    QVariantList args;
    args << QVariant::fromValue(entry);
    send(QMetaObject::InvokeMetaMethod, index, args);
}

void QRemoteObjectRegistry::pushToRegistryIfNeeded()
{
    if (!isReplicaValid())
        return;
    const QSet<QString> myLocs = QSet<QString>::fromList(hostedSources.keys());
    if (myLocs.empty())
        return;
    const QSet<QString> registryLocs = QSet<QString>::fromList(sourceLocations().keys());
    foreach (const QString &loc, myLocs & registryLocs) {
        qCWarning(QT_REMOTEOBJECT) << "Node warning: Ignoring Source" << loc << "as another source ("
                                   << sourceLocations()[loc] << ") has already registered that name.";
        hostedSources.remove(loc);
        return;
    }
    //Sources that need to be pushed to the registry...
    foreach (const QString &loc, myLocs - registryLocs) {
        static int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("addSource(QRemoteObjectSourceLocation)");
        QVariantList args;
        args << QVariant::fromValue(QRemoteObjectSourceLocation(loc, hostedSources[loc]));
        send(QMetaObject::InvokeMetaMethod, index, args);
    }
}

QT_END_NAMESPACE
