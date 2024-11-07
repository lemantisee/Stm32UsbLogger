#include "LogDump.h"

#include "UsbDevice.h"
#include "JsonObject.h"
#include "MonitorCommand.h"

void LogDump::dump(UsbDevice &usbDevice)
{
    while (true) {
        const bool end = Logger::empty();

        SString<64> msg = createLogUnit(end);
        usbDevice.sendData(msg);

        if (end) {
            break;
        }

        const SString<64> report = usbDevice.popData();
        if (report.empty()) {
            break;
        }

        JsonObject inMessage(report);

        if (inMessage.getInt("id", UnknownCommand) != GetLog) {
            break;
        }
    }
}

SString<64> LogDump::createLogUnit(bool end) const
{
    JsonObject j;
    if (end) {
        j.add("id", LogEnd);
        return j.dump();
    }

    SString<48> str = Logger::pop();
    if (str.back() == '\n') {
        str.pop();
        j.add("id", LogUnitEnd);
    } else {
        j.add("id", LogUnit);
    }

    j.add("d", str.c_str());
    return j.dump();
}
