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

#include "qtremoteobjectglobal.h"

#include <QMetaObject>
#include <QMetaProperty>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QT_REMOTEOBJECT, "qt.remoteobjects")

namespace QtRemoteObjects {

void copyStoredProperties(const QObject *src, QObject *dst)
{
    if (!src) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy from a null QObject";
        return;
    }
    if (!dst) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy to a null QObject";
        return;
    }

    const QMetaObject * const mof = src->metaObject();
    const QMetaObject * const mot = dst->metaObject();
    if (mof->propertyCount() != mot->propertyCount()) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy from a different QObject";
        return;
    }

    for (int i = 0, end = mof->propertyCount(); i != end; ++i) {
        const QMetaProperty mpf = mof->property(i);
        if (!mpf.isStored(src))
            continue;
        const QMetaProperty mpt = mot->property(i);
        mpt.write(dst, mpf.read(src));
    }
}

void copyStoredProperties(const QObject *src, QDataStream &dst)
{
    if (!src) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy from a null QObject";
        return;
    }

    const QMetaObject * const mof = src->metaObject();

    for (int i = 0, end = mof->propertyCount(); i != end; ++i) {
        const QMetaProperty mpf = mof->property(i);
        if (!mpf.isStored(src))
            continue;
        dst << mpf.read(src);
    }
}

void copyStoredProperties(QDataStream &src, QObject *dst)
{
    if (!dst) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy to a null QObject";
        return;
    }

    const QMetaObject * const mot = dst->metaObject();

    for (int i = 0, end = mot->propertyCount(); i != end; ++i) {
        const QMetaProperty mpt = mot->property(i);
        if (!mpt.isStored(dst))
            continue;
        QVariant v;
        src >> v;
        mpt.write(dst, v);
    }
}

} // namespace QtRemoteObjects

QT_END_NAMESPACE
