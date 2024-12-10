#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <cfloat>

enum EntityCategory
{
    NONE,
    DIAGNOSTIC,
    CONFIG
};

enum NumberMode
{
    AUTO,
    SLIDER,
    BOX
};

class MqttDevice
{
public:
    MqttDevice()
        : MqttDevice("", "", "", "")
    {
    }

    MqttDevice(const char *identifier, const char *name, const char *model, const char *manufacturer)
        : m_swVersion(""),
          m_configurationUrl("")
    {
        strncpy(m_identifier, identifier, sizeof(m_identifier));
        strncpy(m_name, name, sizeof(m_name));
        strncpy(m_model, model, sizeof(m_model));
        strncpy(m_manufacturer, manufacturer, sizeof(m_manufacturer));
    }

public:
    const char *getIdentifier() const
    {
        return m_identifier;
    }

    const char *getName() const
    {
        return m_name;
    }

    const char *getModel() const
    {
        return m_model;
    }

    const char *getManufacturer() const
    {
        return m_manufacturer;
    }

    const char *getSWVersion() const
    {
        return m_swVersion;
    }

    void setConfigurationUrl(const char *configurationUrl)
    {
        strncpy(m_configurationUrl, configurationUrl, sizeof(m_configurationUrl));
    }

    void setSWVersion(const char *swVersion)
    {
        strncpy(m_swVersion, swVersion, sizeof(m_swVersion));
    }

    void addConfig(JsonDocument &doc) const
    {
        JsonObject device = doc["device"].to<JsonObject>();
        
        device["identifiers"][0] = getIdentifier();
        device["name"] = getName();
        device["model"] = getModel();
        device["manufacturer"] = getManufacturer();

        if (strlen(m_swVersion) > 0)
        {
            device["sw_version"] = getSWVersion();
        }
        if (strlen(m_configurationUrl) > 0)
        {
            device["configuration_url"] = m_configurationUrl;
        }
    }

private:
    char m_identifier[64];
    char m_name[64];
    char m_model[64];
    char m_manufacturer[64];
    char m_swVersion[16];
    char m_configurationUrl[256];
};

class MqttEntity
{
public:
    MqttEntity(const MqttDevice *mqttDevice, const char *objectId, const char *type, const char *humanName)
    {
        m_device = mqttDevice;
        strncpy(m_objectId, objectId, sizeof(m_objectId));
        strncpy(m_type, type, sizeof(m_type));
        strncpy(m_humanName, humanName, sizeof(m_humanName));

        snprintf(m_cmdTopic, sizeof(m_cmdTopic), "%s/%s/%s", m_device->getIdentifier(), m_objectId, m_cmdSubTopic);
        snprintf(m_stateTopic, sizeof(m_stateTopic), "%s/%s/%s", m_device->getIdentifier(), m_objectId, m_stateSubTopic);
        snprintf(m_uniqueId, sizeof(m_uniqueId), "%s-%s", m_device->getIdentifier(), m_objectId);
    }

    void setHasCommandTopic(bool hasCommand)
    {
        m_hasCommandTopic = hasCommand;
    }

    void getBaseTopic(char *baseTopic_, size_t bufferSize) const
    {
        snprintf(baseTopic_, bufferSize, "%s/%s", m_device->getIdentifier(), m_objectId);
    }

    const char *getCommandTopic() const
    {
        return m_cmdTopic;
    }

    void getCommandTopic(char *commandTopic_, size_t bufferSize)
    {
        snprintf(commandTopic_, bufferSize, "%s/%s/%s", m_device->getIdentifier(), m_objectId, m_cmdSubTopic);
    }

    const char *getStateTopic() const
    {
        return m_stateTopic;
    }

    void setCustomStateTopic(const char *customStateTopic)
    {
        strncpy(m_stateTopic, customStateTopic, sizeof(m_stateTopic));
    }

    void setValueTemplate(const char *valueTemplate)
    {
        strncpy(m_valueTemplate, valueTemplate, sizeof(m_valueTemplate));
    }

    void setUnit(const char *unit)
    {
        strncpy(m_unit, unit, sizeof(m_unit));
    }

    void setDeviceClass(const char *deviceClass)
    {
        strncpy(m_deviceClass, deviceClass, sizeof(m_deviceClass));
    }

    void setIcon(const char *icon)
    {
        strncpy(m_icon, icon, sizeof(m_icon));
    }

    void setEntityType(EntityCategory type)
    {
        m_entityType = type;
    }

    const char *getHumanName() const
    {
        return m_humanName;
    }

    void getHomeAssistantConfigTopic(char *configTopic_, size_t bufferSize)
    {
        // TODO:  homeassistant/[type]/[device identifier]/[object id]/config
        snprintf(configTopic_, bufferSize, "homeassistant/%s/%s/%s/config",
                 m_type,
                 m_device->getIdentifier(),
                 m_objectId);
    }

    void getHomeAssistantConfigTopicAlt(char *configTopic_, size_t bufferSize)
    {
        snprintf(configTopic_, bufferSize, "ha/%s/%s/%s/config",
                 m_type,
                 m_device->getIdentifier(),
                 m_objectId);
    }

    String getHomeAssistantConfigPayload() const;

    String getOnlineState() const;

protected:
    virtual void addConfig(JsonDocument &doc) const = 0;

    const MqttDevice *getDevice() const
    {
        return m_device;
    }

    const char *getObjectId() const
    {
        return m_objectId;
    }

private:
    const MqttDevice *m_device; // the device this entity belongs to
    char m_objectId[64];        // our actual device identifier, e.g. doorbell, must be unique within the nodeid
    char m_type[16];            // mqtt device type, e.g. switch
    char m_uniqueId[128];       // the unique identifier, e.g. doorman2323-doorbell
    char m_humanName[64];       // human readbable name, e.g. Door Bell

    bool m_hasCommandTopic = false;
    char m_cmdTopic[255] = "";
    char m_stateTopic[255] = "";
    char m_valueTemplate[255] = "";
    char m_unit[10] = "";
    char m_deviceClass[32] = "";
    char m_icon[128] = "";

    EntityCategory m_entityType = EntityCategory::NONE;

    const char *m_cmdSubTopic = "cmd";
    const char *m_stateSubTopic = "state";

    const char *m_stateOnline = "online";
};

class MqttBinarySensor : public MqttEntity
{
public:
    MqttBinarySensor()
        : MqttBinarySensor(nullptr, "", "")
    {
    }

    MqttBinarySensor(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "binary_sensor", humanName)
    {
    }

    const char *getOnState() const
    {
        return m_stateOn;
    }

    const char *getOffState() const
    {
        return m_stateOff;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["payload_on"] = m_stateOn;
        doc["payload_off"] = m_stateOff;
    }

private:
    static constexpr const char *m_stateOn = "on";
    static constexpr const char *m_stateOff = "off";
};

class MqttSensor : public MqttEntity
{
public:
    enum StateClass
    {
        NONE,
        MEASUREMENT,
        TOTAL,
        TOTAL_INCREASING
    };

    MqttSensor()
        : MqttSensor(nullptr, "", "")
    {
    }

    MqttSensor(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "sensor", humanName)
    {
    }

    void setStateClass(StateClass stateClass)
    {
        m_stateClass = stateClass;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        switch (m_stateClass)
        {
        case MEASUREMENT:
            doc["state_class"] = "measurement";
            break;
        case TOTAL:
            doc["state_class"] = "total";
            break;
        case TOTAL_INCREASING:
            doc["state_class"] = "total_increasing";
            break;
        case NONE:
            // nothing needed
            break;
        }
    }

private:
    StateClass m_stateClass = StateClass::NONE;
};

class MqttSwitch : public MqttEntity
{
public:
    MqttSwitch()
        : MqttSwitch(nullptr, "", "")
    {
    }

    MqttSwitch(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "switch", humanName)
    {
        setHasCommandTopic(true);
    }

    const char *getOnState() const
    {
        return m_stateOn;
    }

    const char *getOffState() const
    {
        return m_stateOff;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["state_on"] = m_stateOn;
        doc["state_off"] = m_stateOff;
        doc["payload_on"] = m_stateOn;
        doc["payload_off"] = m_stateOff;
    }

private:
    static constexpr const char *m_stateOn = "on";
    static constexpr const char *m_stateOff = "off";
};

class MqttSiren : public MqttEntity
{
public:
    MqttSiren()
        : MqttSiren(nullptr, "", "")
    {
    }

    MqttSiren(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "siren", humanName)
    {
        setHasCommandTopic(true);
    }

    const char *getOnState() const
    {
        return m_stateOn;
    }

    const char *getOffState() const
    {
        return m_stateOff;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["state_on"] = m_stateOn;
        doc["state_off"] = m_stateOff;
        doc["payload_on"] = m_stateOn;
        doc["payload_off"] = m_stateOff;

        // TODO: make this configurable
        doc["support_duration"] = m_supportDuration;
        doc["support_volume_set"] = m_supportVolumeSet;
    }

private:
    static constexpr const char *m_stateOn = "on";
    static constexpr const char *m_stateOff = "off";

    bool m_supportDuration = false;
    bool m_supportVolumeSet = false;
};

class MqttButton : public MqttEntity
{
public:
    MqttButton(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "button", humanName)
    {
        setHasCommandTopic(true);
    }

    const char *getPressState() const
    {
        return m_statePress;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["payload_press"] = m_statePress;
    }

private:
    static constexpr const char *m_statePress = "PRESS";
};

class MqttSelect : public MqttEntity
{
public:
    MqttSelect(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "select", humanName), m_options()
    {
        setHasCommandTopic(true);
    }

    const char *getOption(uint8_t index) const
    {
        return m_options[index].c_str(); // you can access the elements just like if it was a regular array
    }

    void addOption(const char *option)
    {
        m_options.push_back(String(option));
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        for (uint i = 0; i < m_options.size(); i++)
        {
            doc["options"][i].add(m_options[i]);
        }
    }

private:
    std::vector<String> m_options;
};


class MqttNumber : public MqttEntity
{
public:
    MqttNumber(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "number", humanName)
    {
        setHasCommandTopic(true);
    }

    void setMin(float minValue)
    {
        m_min = minValue;
    }

    void setMax(float maxValue)
    {
        m_max = maxValue;
    }

    void setMode(NumberMode mode)
    {
        m_mode = mode;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        if (m_min != FLT_MIN)
        {
            doc["min"] = m_min;
        }
        if (m_max != FLT_MAX)
        {
            doc["max"] = m_max;
        }
        doc["step"] = m_step;

        if(m_mode == NumberMode::SLIDER)
        {
            doc["mode"] = "slider";
        }
        else if(m_mode == NumberMode::BOX)
        {
            doc["mode"] = "box";
        }
    }

private:
    char m_pattern[255] = "";
    float m_min = FLT_MIN;
    float m_max = FLT_MAX;
    float m_step = 1.0f;
    NumberMode m_mode = NumberMode::AUTO;
};

class MqttText : public MqttEntity
{
public:
    MqttText(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "text", humanName)
    {
        setHasCommandTopic(true);
    }

    void setPattern(const char *pattern)
    {
        strncpy(m_pattern, pattern, sizeof(m_pattern));
    }

    void setMinLetters(int16_t minLetters)
    {
        m_min = minLetters;
    }

    void setMaxLetters(int16_t maxLetters)
    {
        m_max = maxLetters;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        if (strlen(m_pattern) > 0)
        {
            doc["pattern"] = m_pattern;
        }
        if (m_min >= 0)
        {
            doc["min"] = m_min;
        }
        if (m_max >= 0)
        {
            doc["max"] = m_max;
        }
    }

private:
    char m_pattern[255] = "";
    int16_t m_min = -1;
    int16_t m_max = -1;
};

class MqttLock : public MqttEntity
{
public:
    MqttLock(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "lock", humanName)
    {
        setHasCommandTopic(true);
    }

    const char *getLockCommand() const
    {
        return m_cmdLock;
    }

    const char *getUnlockCommand() const
    {
        return m_cmdUnlock;
    }

    const char *getOpenCommand() const
    {
        return m_cmdOpen;
    }

    const char *getLockedState() const
    {
        return m_stateLocked;
    }

    const char *getLockingState() const
    {
        return m_stateLocking;
    }

    const char *getUnlockedState() const
    {
        return m_stateUnlocked;
    }

    const char *getUnlockingState() const
    {
        return m_stateUnlocking;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["payload_lock"] = m_cmdLock;
        doc["payload_unlock"] = m_cmdUnlock;
        doc["payload_open"] = m_cmdOpen;
        doc["state_locked"] = m_stateLocked;
        doc["state_locking"] = m_stateLocking;
        doc["state_unlocked"] = m_stateUnlocked;
        doc["state_unlocking"] = m_stateUnlocking;
    }

private:
    static constexpr const char *m_cmdLock = "lock";
    static constexpr const char *m_cmdUnlock = "unlock";
    static constexpr const char *m_cmdOpen = "open";
    static constexpr const char *m_stateLocked = "locked";
    static constexpr const char *m_stateLocking = "locking";
    static constexpr const char *m_stateUnlocked = "unlocked";
    static constexpr const char *m_stateUnlocking = "unlocking";
};

class MqttCover : public MqttEntity
{
public:
    MqttCover(MqttDevice *device, const char *objectId, const char *humanName)
        : MqttEntity(device, objectId, "cover", humanName)
    {
        setHasCommandTopic(true);
        snprintf(m_cmdPositionTopic, sizeof(m_cmdPositionTopic), "%s/%s/%s", getDevice()->getIdentifier(), getObjectId(), m_cmdPositionSubTopic);
        snprintf(m_positionTopic, sizeof(m_positionTopic), "%s/%s/%s", getDevice()->getIdentifier(), getObjectId(), m_positionSubTopic);
    }

    const char *getCommandPositionTopic() const
    {
        return m_cmdPositionTopic;
    }

    const char *getPositionTopic() const
    {
        return m_positionTopic;
    }

    const char *getOpenCommand() const
    {
        return m_cmdOpen;
    }

    const char *getCloseCommand() const
    {
        return m_cmdClose;
    }

    const char *getStopCommand() const
    {
        return m_cmdStop;
    }

    const char *getOpeningState() const
    {
        return m_stateOpening;
    }

    const char *getClosingState() const
    {
        return m_stateClosing;
    }

    const char *getStoppedState() const
    {
        return m_stateStopped;
    }

protected:
    virtual void addConfig(JsonDocument &doc) const override
    {
        doc["payload_open"] = m_cmdOpen;
        doc["payload_close"] = m_cmdClose;
        doc["payload_stop"] = m_cmdStop;

        doc["state_opening"] = m_stateOpening;
        doc["state_stopped"] = m_stateStopped;
        doc["state_closing"] = m_stateClosing;

        doc["set_position_topic"] = m_cmdPositionTopic;
        doc["position_topic"] = m_positionTopic;
        doc["position_open"] = m_positionOpen;
        doc["position_closed"] = m_positionClosed;
    }

private:
    static constexpr const char *m_cmdOpen = "open";
    static constexpr const char *m_cmdClose = "close";
    static constexpr const char *m_cmdStop = "stop";

    static constexpr const char *m_stateOpening = "opening";
    static constexpr const char *m_stateClosing = "closing";
    static constexpr const char *m_stateStopped = "stopped";

    static constexpr const uint8_t m_positionOpen = 100;
    static constexpr const uint8_t m_positionClosed = 0;

    static constexpr const char *m_cmdPositionSubTopic = "cmd_position";
    static constexpr const char *m_positionSubTopic = "position";

    char m_cmdPositionTopic[255];
    char m_positionTopic[255];
};
