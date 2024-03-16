/*
MIT License

Copyright (c) 2024 Alexander Wentz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "systemdDbusServiceLib.h"
#include <map>
#include <cstdarg>

namespace sysddbus{

struct DbusData{
    std::string dbusFunction;
    std::string signature;
};

std::map<ServiceAction, DbusData> actionMapping{
    {ServiceAction::DISABLE, {"DisableUnitFiles", "asb"}},
    {ServiceAction::ENABLE, {"EnableUnitFiles", "asbb"}},
    {ServiceAction::MASK, {"MaskUnitFiles", "asbb"}},
    {ServiceAction::UNMASK, {"UnmaskUnitFiles", "asb"}},
    {ServiceAction::START, {"StartUnit", "ss"}},
    {ServiceAction::STOP, {"StopUnit", "ss"}}
};

class SD_BUS_MSG {
   public: 
    SD_BUS_MSG() = default;
    ~SD_BUS_MSG() {
        if(m_msg) {
            sd_bus_message_unref(m_msg);
        }
    };
    inline sd_bus_message** get(){
        return &m_msg;
    }
    void setData(const DbusData& data){
        m_data = data;
    };
    std::string getSignature(){
        return m_data.signature;
    };
   private:
    sd_bus_message *m_msg = NULL;
    DbusData m_data;
};


int createNewMethodCall(sd_bus* bus, SD_BUS_MSG& msg, ServiceAction action){
    auto iter = actionMapping.find(action);
    if (iter == actionMapping.end()){
        fprintf(stderr, "Action not specified in map\n");
        return -1;
    }
    msg.setData(iter->second);
    
    // see also: https://man7.org/linux/man-pages/man3/sd_bus_message_new_method_call.3.html
    return sd_bus_message_new_method_call(
                bus,
                msg.get(),
                "org.freedesktop.systemd1",          // Service name
                "/org/freedesktop/systemd1",         // Object path
                "org.freedesktop.systemd1.Manager",  // Interface
                iter->second.dbusFunction.c_str());  // Method name
}

int appendUserData2Msg(ServiceAction action, SD_BUS_MSG& msg, const std::string& service){
    // see also: https://www.freedesktop.org/software/systemd/man/latest/sd_bus_message_append.html#
    // and: https://www.freedesktop.org/wiki/Software/systemd/dbus/
    int ret = 0;
    switch (action)
    {
    case ServiceAction::DISABLE:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), 1, service.c_str(), false);
        break;
    case ServiceAction::ENABLE:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), 1, service.c_str(), false, true /*force*/);
        break;
    case ServiceAction::MASK:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), 1, service.c_str(), false, true /*force*/);
        break;
    case ServiceAction::UNMASK:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), 1, service.c_str(), false);
        break;
    case ServiceAction::START:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), service.c_str(), "fail"/*mode*/);
        break;
    case ServiceAction::STOP:
        ret = sd_bus_message_append(*msg.get(), msg.getSignature().c_str(), service.c_str(), "fail"/*mode*/);
        break;
    default:
        ret = -1;
        break;
    }

    if (ret < 0) {
        fprintf(stderr, "Failed to append arguments to method call message: %s\n", strerror(-ret));
        return ret;
    }
    return ret;
}

int SystemdDbusServiceLib::doServiceAction(ServiceAction action, const std::string& service){
    if( !m_bus ){
        fprintf(stderr, "Not initialized. Abort.\n");
        return -1;
    }

    SD_BUS_MSG msg{}, reply{};
    int ret = createNewMethodCall(m_bus, msg, action);
    if( ret != 0 ){
        fprintf(stderr, "Could not create New Method Call. %s. Abort.\n", strerror(-ret));
        return ret;
    }

    ret = appendUserData2Msg(action, msg, service);
    if (ret < 0) {
        fprintf(stderr, "Failed to append arguments to method call message: %s\n", strerror(-ret));
        return ret;
    }

    sd_bus_error error = SD_BUS_ERROR_NULL;
    ret = sd_bus_call(m_bus, *msg.get(), 0, &error, reply.get());
    if (ret < 0) {
        fprintf(stderr, "Failed to call method: %s\n", error.message);
        return ret;
    }
    return 0;
}

}

