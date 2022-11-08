#include <ESP8266WiFi.h>
#include <WString.h>

#include "common.h"

const struct {
    String name = "Greenhouse Controller";
    String model = "greenhouse_controller_prototype";
    String identifier = "greenhouse_controller_" + WiFi.macAddress();
    String swVersion = "1.0"; // TODO: version will be generated.
    String manufacturer = "Studio TJ";
} DeviceInfo;

void constructDeviceInfo(JsonDocument *config) {
    JsonObject deviceInfo = config->createNestedObject("device");
    deviceInfo["name"] = DeviceInfo.name;
    deviceInfo["model"] = DeviceInfo.model;
    JsonArray identifiers = deviceInfo.createNestedArray("identifiers");
    identifiers.add(DeviceInfo.identifier);
    deviceInfo["sw_version"] = DeviceInfo.swVersion;
    deviceInfo["manufacturer"] = DeviceInfo.manufacturer;
}