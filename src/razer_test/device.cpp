/*
 * Copyright (C) 2016-2018  Luca Weiss <luca (at) z3ntu (dot) xyz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "device_p.h"
#include "libopenrazer.h"
#include "libopenrazer_private.h"

#include <QVector>

namespace libopenrazer {

namespace razer_test {

Device::Device(QDBusObjectPath objectPath)
{
    d = new DevicePrivate();
    d->mParent = this;
    d->mObjectPath = objectPath;
    d->supportedFx = d->getSupportedFx();
    d->supportedFeatures = d->getSupportedFeatures();

    for (const QDBusObjectPath &ledPath : d->getLedObjectPaths()) {
        Led *led = new Led(this, ledPath);
        d->leds.append(led);
    }
}

Device::~Device()
{
    for (libopenrazer::Led *led : d->leds) {
        delete led;
    }
}

QDBusObjectPath Device::objectPath()
{
    return d->mObjectPath;
}

bool Device::hasFeature(const QString &featureStr)
{
    return d->supportedFeatures.contains(featureStr);
}

QString Device::getDeviceImageUrl()
{
    return ""; // TODO Needs implementation
}

QList<QDBusObjectPath> DevicePrivate::getLedObjectPaths()
{
    QVariant reply = deviceIface()->property("Leds");
    return handleDBusVariant<QList<QDBusObjectPath>>(reply, deviceIface()->lastError(), Q_FUNC_INFO);
}

QList<::libopenrazer::Led *> Device::getLeds()
{
    return d->leds;
}

QStringList DevicePrivate::getSupportedFx()
{
    QVariant reply = deviceIface()->property("SupportedFx");
    return handleDBusVariant<QStringList>(reply, deviceIface()->lastError(), Q_FUNC_INFO);
}

QStringList DevicePrivate::getSupportedFeatures()
{
    QVariant reply = deviceIface()->property("SupportedFeatures");
    return handleDBusVariant<QStringList>(reply, deviceIface()->lastError(), Q_FUNC_INFO);
}

QString Device::getDeviceMode()
{
    return "error"; // TODO Needs implementation
}

void Device::setDeviceMode(uchar mode_id, uchar param)
{
    // TODO Needs implementation
}

QString Device::getSerial()
{
    QDBusReply<QString> reply = d->deviceIface()->call("getSerial");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

QString Device::getDeviceName()
{
    QVariant reply = d->deviceIface()->property("Name");
    return handleDBusVariant<QString>(reply, d->deviceIface()->lastError(), Q_FUNC_INFO);
}

QString Device::getDeviceType()
{
    QVariant reply = d->deviceIface()->property("Type");
    return handleDBusVariant<QString>(reply, d->deviceIface()->lastError(), Q_FUNC_INFO);
}

QString Device::getFirmwareVersion()
{
    QDBusReply<QString> reply = d->deviceIface()->call("getFirmwareVersion");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

QString Device::getKeyboardLayout()
{
    QDBusReply<QString> reply = d->deviceIface()->call("getKeyboardLayout");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

ushort Device::getPollRate()
{
    QDBusReply<ushort> reply = d->deviceIface()->call("getPollRate");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

void Device::setPollRate(ushort pollrate)
{
    QDBusReply<bool> reply = d->deviceIface()->call("setPollRate", QVariant::fromValue(pollrate));
    handleVoidDBusReply(reply, Q_FUNC_INFO);
}

QVector<ushort> Device::getSupportedPollRates()
{
    // TODO Needs implementation
    return { 125, 500, 1000 };
}

void Device::setDPI(::openrazer::RazerDPI dpi)
{
    QDBusReply<bool> reply = d->deviceIface()->call("setDPI", QVariant::fromValue(dpi));
    handleVoidDBusReply(reply, Q_FUNC_INFO);
}

::openrazer::RazerDPI Device::getDPI()
{
    QDBusReply<::openrazer::RazerDPI> reply = d->deviceIface()->call("getDPI");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

ushort Device::maxDPI()
{
    QDBusReply<ushort> reply = d->deviceIface()->call("getMaxDPI");
    return handleDBusReply(reply, Q_FUNC_INFO);
}

QVector<ushort> Device::getAllowedDPI()
{
    // TODO Needs implementation
    return {};
}

void Device::displayCustomFrame()
{
    QDBusReply<bool> reply = d->deviceIface()->call("displayCustomFrame");
    handleVoidDBusReply(reply, Q_FUNC_INFO);
}

void Device::defineCustomFrame(uchar row, uchar startColumn, uchar endColumn, QVector<::openrazer::RGB> colorData)
{
    QDBusReply<bool> reply = d->deviceIface()->call("defineCustomFrame", QVariant::fromValue(row), QVariant::fromValue(startColumn), QVariant::fromValue(endColumn), QVariant::fromValue(colorData));
    handleVoidDBusReply(reply, Q_FUNC_INFO);
}

::openrazer::MatrixDimensions Device::getMatrixDimensions()
{
    QVariant reply = d->deviceIface()->property("MatrixDimensions");
    return handleDBusVariant<::openrazer::MatrixDimensions>(reply, d->deviceIface()->lastError(), Q_FUNC_INFO);
}

QDBusInterface *DevicePrivate::deviceIface()
{
    if (iface == nullptr) {
        iface = new QDBusInterface(OPENRAZER_SERVICE_NAME, mObjectPath.path(), "io.github.openrazer1.Device",
                                   OPENRAZER_DBUS_BUS, mParent);
    }
    if (!iface->isValid()) {
        fprintf(stderr, "%s\n",
                qPrintable(OPENRAZER_DBUS_BUS.lastError().message()));
    }
    return iface;
}

}

}
