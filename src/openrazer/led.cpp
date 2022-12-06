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
#include "led_p.h"
#include "libopenrazer.h"
#include "libopenrazer_private.h"

#include <QColor>
#include <QDBusReply>

#define QCOLOR_TO_QVARIANT(c)               \
    QVariant::fromValue(c.red()),           \
            QVariant::fromValue(c.green()), \
            QVariant::fromValue(c.blue())

namespace libopenrazer {

namespace openrazer {

Led::Led(Device *device, QDBusObjectPath objectPath, ::openrazer::RazerLedId ledId, QString lightingLocation)
{
    d = new LedPrivate();
    d->mParent = this;
    d->device = device;
    d->mObjectPath = objectPath;
    d->ledId = ledId;
    d->lightingLocation = lightingLocation;

    // Leave lightingLocationMethod empty in case it's Chroma
    if (lightingLocation == "Chroma") {
        d->interface = "razer.device.lighting.chroma";
    } else {
        d->lightingLocationMethod = lightingLocation;
        d->interface = "razer.device.lighting." + lightingLocation.toLower();
    }

    d->setupCapabilities();
}

/*
 * Destructor
 */
Led::~Led() = default;

void LedPrivate::setupCapabilities()
{
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "None"))
        supportedFx.append("off");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Static"))
        supportedFx.append("static");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Blinking"))
        supportedFx.append("blinking");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "BreathSingle"))
        supportedFx.append("breathing");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "BreathDual"))
        supportedFx.append("breathing_dual");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "BreathRandom"))
        supportedFx.append("breathing_random");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Spectrum"))
        supportedFx.append("spectrum");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Wave"))
        supportedFx.append("wave");
    if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Reactive"))
        supportedFx.append("reactive");

    if (supportedFx.isEmpty() && device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Active")) {
        supportedFx.append("off");
        supportedFx.append("on");
    }

    // No-color static/breathing variants
    if (device->d->hasCapabilityInternal("razer.device.lighting.bw2013", "setStatic"))
        supportedFx.append("static");
    if (device->d->hasCapabilityInternal("razer.device.lighting.bw2013", "setPulsate"))
        supportedFx.append("breathing");

    if (lightingLocation == "Chroma") {
        if (device->d->hasCapabilityInternal("razer.device.lighting.brightness", "setBrightness"))
            supportedFx.append("brightness");
    } else {
        if (device->d->hasCapabilityInternal(interface, "set" + lightingLocationMethod + "Brightness"))
            supportedFx.append("brightness");
    }
}

QDBusObjectPath Led::getObjectPath()
{
    return d->mObjectPath;
}

bool Led::hasFx(const QString &fxStr)
{
    return d->supportedFx.contains(fxStr);
}

bool Led::hasFx(::openrazer::RazerEffect fx)
{
    QString fxStr;
    switch (fx) {
    case ::openrazer::RazerEffect::Off:
        fxStr = "off";
        break;
    case ::openrazer::RazerEffect::On:
        fxStr = "on";
        break;
    case ::openrazer::RazerEffect::Static:
        fxStr = "static";
        break;
    case ::openrazer::RazerEffect::Breathing:
        fxStr = "breathing";
        break;
    case ::openrazer::RazerEffect::BreathingDual:
        fxStr = "breathing_dual";
        break;
    case ::openrazer::RazerEffect::BreathingRandom:
        fxStr = "breathing_random";
        break;
    case ::openrazer::RazerEffect::Blinking:
        fxStr = "blinking";
        break;
    case ::openrazer::RazerEffect::Spectrum:
        fxStr = "spectrum";
        break;
    case ::openrazer::RazerEffect::Wave:
        fxStr = "wave";
        break;
    case ::openrazer::RazerEffect::Reactive:
        fxStr = "reactive";
        break;
    }
    return hasFx(fxStr);
}

::openrazer::RazerEffect Led::getCurrentEffect()
{
    QDBusReply<QString> reply = d->ledIface()->call("get" + d->lightingLocationMethod + "Effect");
    if (!reply.isValid()) {
        printDBusError(reply.error(), Q_FUNC_INFO);
        throw DBusException(reply.error());
    }

    QString effect = reply.value();
    // TODO:
    // * breathTriple
    // * ripple
    // * rippleRandomColour
    // * starlightSingle
    // * starlightDual
    // * starlightRandom
    if (effect == "none") {
        return ::openrazer::RazerEffect::Off;
    } else if (effect == "static") {
        return ::openrazer::RazerEffect::Static;
    } else if (effect == "breathSingle" || effect == "pulsate") {
        return ::openrazer::RazerEffect::Breathing;
    } else if (effect == "breathDual") {
        return ::openrazer::RazerEffect::BreathingDual;
    } else if (effect == "breathingRandom") {
        return ::openrazer::RazerEffect::BreathingRandom;
    } else if (effect == "blinking") {
        return ::openrazer::RazerEffect::Blinking;
    } else if (effect == "spectrum") {
        return ::openrazer::RazerEffect::Spectrum;
    } else if (effect == "wave") {
        return ::openrazer::RazerEffect::Wave;
    } else if (effect == "reactive") {
        return ::openrazer::RazerEffect::Reactive;
    } else {
        qWarning("libopenrazer: Unhandled effect in getCurrentEffect: %s, defaulting to Spectrum", qUtf8Printable(effect));
        return ::openrazer::RazerEffect::Spectrum;
    }
}

QVector<::openrazer::RGB> Led::getCurrentColors()
{
    QDBusReply<QByteArray> reply = d->ledIface()->call("get" + d->lightingLocationMethod + "EffectColors");
    if (!reply.isValid()) {
        printDBusError(reply.error(), Q_FUNC_INFO);
        throw DBusException(reply.error());
    }

    QByteArray values = reply.value();
    if (values.size() % 3 != 0) {
        throw DBusException("Invalid return array from EffectColors", "The EffectColors return array has an invalid size.");
    }
    QVector<::openrazer::RGB> colors;
    for (int i = 0; i < values.size() / 3; i++) {
        colors.append({ static_cast<uchar>(values[i * 3]),
                        static_cast<uchar>(values[i * 3 + 1]),
                        static_cast<uchar>(values[i * 3 + 2]) });
    }
    return colors;
}

::openrazer::RazerLedId Led::getLedId()
{
    return d->ledId;
}

bool Led::setOff()
{
    QDBusReply<void> reply;
    if (d->device->d->hasCapabilityInternal(d->interface, "set" + d->lightingLocationMethod + "Active"))
        reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Active", false);
    else
        reply = d->ledIface()->call("set" + d->lightingLocationMethod + "None");
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setOn()
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Active", true);
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setStatic(QColor color)
{
    QDBusReply<void> reply;
    if (d->device->d->hasCapabilityInternal("razer.device.lighting.bw2013", "setStatic"))
        reply = d->ledBw2013Iface()->call("setStatic");
    else
        reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Static", QCOLOR_TO_QVARIANT(color));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setBreathing(QColor color)
{
    QDBusReply<void> reply;
    if (d->device->d->hasCapabilityInternal("razer.device.lighting.bw2013", "setPulsate"))
        reply = d->ledBw2013Iface()->call("setPulsate");
    else
        reply = d->ledIface()->call("set" + d->lightingLocationMethod + "BreathSingle", QCOLOR_TO_QVARIANT(color));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setBreathingDual(QColor color, QColor color2)
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "BreathDual", QCOLOR_TO_QVARIANT(color), QCOLOR_TO_QVARIANT(color2));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setBreathingRandom()
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "BreathRandom");
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setBlinking(QColor color)
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Blinking", QCOLOR_TO_QVARIANT(color));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setSpectrum()
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Spectrum");
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setWave(::openrazer::WaveDirection direction)
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Wave", static_cast<int>(direction));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setReactive(QColor color, ::openrazer::ReactiveSpeed speed)
{
    QDBusReply<void> reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Reactive", QCOLOR_TO_QVARIANT(color), static_cast<uchar>(speed));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

bool Led::setBrightness(uchar brightness)
{
    double dbusBrightness = (double)brightness / 255 * 100;
    QDBusReply<void> reply;
    if (d->lightingLocation == "Chroma")
        reply = d->ledBrightnessIface()->call("setBrightness", QVariant::fromValue(dbusBrightness));
    else
        reply = d->ledIface()->call("set" + d->lightingLocationMethod + "Brightness", QVariant::fromValue(dbusBrightness));
    return handleVoidDBusReply(reply, Q_FUNC_INFO);
}

uchar Led::getBrightness()
{
    QDBusReply<double> reply;
    if (d->lightingLocation == "Chroma")
        reply = d->ledBrightnessIface()->call("getBrightness");
    else
        reply = d->ledIface()->call("get" + d->lightingLocationMethod + "Brightness");
    if (reply.isValid()) {
        return reply.value() / 100 * 255;
    } else {
        printDBusError(reply.error(), Q_FUNC_INFO);
        throw DBusException(reply.error());
    }
}

QDBusInterface *LedPrivate::ledIface()
{
    if (iface == nullptr) {
        iface = new QDBusInterface(OPENRAZER_SERVICE_NAME, mObjectPath.path(), interface,
                                   OPENRAZER_DBUS_BUS, mParent);
    }
    if (!iface->isValid()) {
        fprintf(stderr, "%s\n",
                qPrintable(OPENRAZER_DBUS_BUS.lastError().message()));
    }
    return iface;
}

QDBusInterface *LedPrivate::ledBrightnessIface()
{
    if (ifaceBrightness == nullptr) {
        ifaceBrightness = new QDBusInterface(OPENRAZER_SERVICE_NAME, mObjectPath.path(), "razer.device.lighting.brightness",
                                             OPENRAZER_DBUS_BUS, mParent);
    }
    if (!ifaceBrightness->isValid()) {
        fprintf(stderr, "%s\n",
                qPrintable(OPENRAZER_DBUS_BUS.lastError().message()));
    }
    return ifaceBrightness;
}

QDBusInterface *LedPrivate::ledBw2013Iface()
{
    if (ifaceBw2013 == nullptr) {
        ifaceBw2013 = new QDBusInterface(OPENRAZER_SERVICE_NAME, mObjectPath.path(), "razer.device.lighting.bw2013",
                                         OPENRAZER_DBUS_BUS, mParent);
    }
    if (!ifaceBw2013->isValid()) {
        fprintf(stderr, "%s\n",
                qPrintable(OPENRAZER_DBUS_BUS.lastError().message()));
    }
    return ifaceBw2013;
}

}

}
