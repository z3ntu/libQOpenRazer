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

#ifndef LIBRAZER_H
#define LIBRAZER_H

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QException>

#include <razer_test.h>

#undef RAZER_TEST_DBUS_BUS
#define RAZER_TEST_DBUS_BUS QDBusConnection::sessionBus()

#define OPENRAZER_SERVICE_NAME "org.razer"

// NOTE: DBus types -> Qt/C++ types: http://doc.qt.io/qt-5/qdbustypesystem.html#primitive-types

namespace libopenrazer {

enum DaemonStatus { Enabled,
                    Disabled,
                    NotInstalled,
                    NoSystemd,
                    Unknown };

void printDBusError(QDBusError error, const char *functionname);
void handleVoidReply(QDBusReply<void> reply, const char *functionname);
bool handleBoolReply(QDBusReply<bool> reply, const char *functionname);
QString handleStringReply(QDBusReply<QString> reply, const char *functionname);
QStringList handleStringListReply(QDBusReply<QStringList> reply, const char *functionname);

class Manager : public QObject
{
    Q_OBJECT
private:
    QDBusInterface *ifaceDaemon = nullptr;
    QDBusInterface *ifaceDevices = nullptr;
    QDBusInterface *managerDaemonIface();
    QDBusInterface *managerDevicesIface();

public:
    Manager();

    QList<QDBusObjectPath> getDevices();
    QString getDaemonVersion();
    bool isDaemonRunning();

    QVariantHash getSupportedDevices();

    // Sync
    bool syncEffects(bool yes);
    bool getSyncEffects();

    // Screensaver
    bool setTurnOffOnScreensaver(bool turnOffOnScreensaver);
    bool getTurnOffOnScreensaver();

    // Misc
    DaemonStatus getDaemonStatus();
    QString getDaemonStatusOutput();
    bool enableDaemon();
    // bool disableDaemon();

    // - Signal Connect Mehtods -
    bool connectDevicesChanged(QObject *receiver, const char *slot);
};

class Led : public QObject
{
    Q_OBJECT
private:
    QDBusInterface *iface = nullptr;
    QDBusInterface *ifaceBrightness = nullptr;
    QDBusInterface *ledIface();
    QDBusInterface *ledBrightnessIface();

    QDBusObjectPath mObjectPath;

    razer_test::RazerLedId ledId;
    QString someStr; // FIXME Called "loc" in setupCapabilities()

public:
    Led(QDBusObjectPath objectPath, razer_test::RazerLedId ledId, QString someStr);
    ~Led() override;

    QDBusObjectPath getObjectPath();
    razer_test::RazerEffect getCurrentEffect();
    QList<razer_test::RGB> getCurrentColors();
    razer_test::RazerLedId getLedId();

    bool setOff();
    bool setStatic(QColor color);
    bool setBreathing(QColor color);
    bool setBreathingDual(QColor color, QColor color2);
    bool setBreathingRandom();
    bool setBlinking(QColor color);
    bool setSpectrum();
    bool setWave(razer_test::WaveDirection direction);
    bool setReactive(QColor color, razer_test::ReactiveSpeed speed);

    bool setBrightness(uchar brightness);
    uchar getBrightness();
};

class Device : public QObject
{
    Q_OBJECT
private:
    QDBusInterface *ifaceMisc = nullptr;
    QDBusInterface *ifaceDpi = nullptr;
    QDBusInterface *deviceMiscIface();
    QDBusInterface *deviceDpiIface();

    QDBusObjectPath mObjectPath;

    QStringList supportedFx;
    QStringList supportedFeatures;

    QList<Led *> leds;

    void introspect();
    void setupCapabilities();
    bool hasCapabilityInternal(const QString &interface, const QString &method = QString());
    QStringList introspection;

    // Maps RazerLedId to "Chroma" or "Scroll" (the string put e.g. into setScrollSpectrum)
    QMap<razer_test::RazerLedId, QString> supportedLeds;

public:
    Device(QDBusObjectPath objectPath);
    ~Device() override;

    QDBusObjectPath objectPath();
    bool hasFx(const QString &fxStr);
    bool hasFx(razer_test::RazerEffect fx);
    bool hasFeature(const QString &featureStr);
    QString getPngFilename();
    QString getPngUrl();

    QList<Led *> getLeds();

    // --- MISC METHODS ---
    QString getDeviceMode();
    bool setDeviceMode(uchar mode_id, uchar param);
    QString getSerial();
    QString getDeviceName();
    QString getDeviceType();
    QString getFirmwareVersion();
    QString getKeyboardLayout();
    QVariantHash getRazerUrls();

    // --- POLL RATE ---
    ushort getPollRate();
    bool setPollRate(ushort pollrate);

    // --- DPI ---
    bool setDPI(razer_test::RazerDPI dpi);
    razer_test::RazerDPI getDPI();
    ushort maxDPI();

    // - Custom frame -
    bool displayCustomFrame();
    bool defineCustomFrame(uchar row, uchar startColumn, uchar endColumn, QVector<QColor> colorData);
    razer_test::MatrixDimensions getMatrixDimensions();
};

class DBusException : public QException
{
public:
    DBusException(const QDBusError &error);
    DBusException(const QString &name, const QString &message);

    void raise() const override;
    DBusException *clone() const override;

    QString getName();
    QString getMessage();

private:
    QString name;
    QString message;
};

class RazerCapability
{
public:
    RazerCapability();
    RazerCapability(razer_test::RazerEffect identifier, QString displayString, int numColors);
    RazerCapability(razer_test::RazerEffect, QString displayString, bool wave);
    RazerCapability(const RazerCapability &other);
    ~RazerCapability();

    int getNumColors() const;
    razer_test::RazerEffect getIdentifier() const;
    QString getDisplayString() const;
    bool isWave() const;

private:
    int numColors;
    razer_test::RazerEffect identifier;
    QString displayString;
    bool wave;
};

const QList<RazerCapability> ledFxList {
    RazerCapability(razer_test::RazerEffect::Off, Led::tr("Off"), 0),
    RazerCapability(razer_test::RazerEffect::Static, Led::tr("Static"), 1),
    RazerCapability(razer_test::RazerEffect::Breathing, Led::tr("Breathing"), 1),
    RazerCapability(razer_test::RazerEffect::BreathingDual, Led::tr("Breathing Dual"), 2),
    RazerCapability(razer_test::RazerEffect::BreathingRandom, Led::tr("Breathing Random"), 0),
    RazerCapability(razer_test::RazerEffect::Blinking, Led::tr("Blinking"), 1),
    RazerCapability(razer_test::RazerEffect::Spectrum, Led::tr("Spectrum"), 0),
    RazerCapability(razer_test::RazerEffect::Wave, Led::tr("Wave"), true),
    RazerCapability(razer_test::RazerEffect::Reactive, Led::tr("Reactive"), 1),
};

const QHash<razer_test::RazerLedId, QString> ledIdToStringTable {
    { razer_test::RazerLedId::Unspecified, Led::tr("Unspecified") },
    { razer_test::RazerLedId::ScrollWheelLED, Led::tr("Scroll Wheel") },
    { razer_test::RazerLedId::BatteryLED, Led::tr("Battery") },
    { razer_test::RazerLedId::LogoLED, Led::tr("Logo") },
    { razer_test::RazerLedId::BacklightLED, Led::tr("Backlight") },
    { razer_test::RazerLedId::MacroRecordingLED, Led::tr("Macro Recording") },
    { razer_test::RazerLedId::GameModeLED, Led::tr("Game Mode") },
    { razer_test::RazerLedId::KeymapRedLED, Led::tr("Keymap Red") },
    { razer_test::RazerLedId::KeymapGreenLED, Led::tr("Keymap Green") },
    { razer_test::RazerLedId::KeymapBlueLED, Led::tr("Keymap Blue") },
    { razer_test::RazerLedId::RightSideLED, Led::tr("Right Side") },
    { razer_test::RazerLedId::LeftSideLED, Led::tr("Left Side") }
};

}

Q_DECLARE_METATYPE(libopenrazer::RazerCapability)

#endif // LIBRAZER_H
