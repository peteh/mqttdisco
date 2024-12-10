#include "MqttDevice.h"

String MqttEntity::getHomeAssistantConfigPayload() const
{
    JsonDocument doc;

    // if we give no human name to the entity
    // it will get the name of the device instead
    if (strlen(m_humanName) == 0)
    {
        doc["name"] = nullptr;
    }
    else
    {
        doc["name"] = m_humanName;
    }

    doc["unique_id"] = m_uniqueId;

    char baseTopic[255];
    getBaseTopic(baseTopic, sizeof(baseTopic));
    doc["~"] = baseTopic;

    if (m_hasCommandTopic)
    {
        doc["command_topic"] = "~/cmd";
    }
    // doc["state_topic"] = "~/state";
    doc["state_topic"] = m_stateTopic;

    // set new naming scheme for homeassistant
    doc["has_entity_name"] = true;

    // add the other configurations
    addConfig(doc);

    if (m_valueTemplate[0] != 0)
    {
        doc["value_template"] = m_valueTemplate;
    }

    if (strlen(m_unit) > 0)
    {
        doc["unit_of_measurement"] = m_unit;
    }

    if (strlen(m_deviceClass) > 0)
    {
        doc["device_class"] = m_deviceClass;
    }

    if (m_entityType == EntityCategory::CONFIG)
    {
        doc["entity_category"] = "config";
    }
    else if (m_entityType == EntityCategory::DIAGNOSTIC)
    {
        doc["entity_category"] = "diagnostic";
    }

    if (strlen(m_icon) > 0)
    {
        doc["icon"] = m_icon;
    }

    // add device config
    m_device->addConfig(doc);

    String configData;
    serializeJson(doc, configData);
    return configData;
}