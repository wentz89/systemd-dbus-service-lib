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

#ifndef _SYSTEMDDBUSSERVICELIB_
#define _SYSTEMDDBUSSERVICELIB_

#include <systemd/sd-bus.h>
#include <cstdlib>
#include <cstdio>
#include <string>

namespace sysddbus{

enum class ServiceAction {
    DISABLE = 0,
    ENABLE,
    MASK,
    UNMASK,
    START,
    STOP
};

class SystemdDbusServiceLib {
   public:
    SystemdDbusServiceLib() {
        (void)reinit();
    };
    ~SystemdDbusServiceLib() {
        cleanUp();
    };

    int reinit(){
        int ret = init();
        if (ret != 0) {
            cleanUp();
            m_bus = nullptr;
        }
        return ret;
    };

    // 0 when all good
    bool isGood(){
        return (m_bus != nullptr);
    }

    int doServiceAction(ServiceAction action, const std::string& service);

   private:
    sd_bus *m_bus = nullptr;

    void cleanUp(){
        if (m_bus) {
            (void)sd_bus_flush_close_unref(m_bus);
        }
    };
    int init(){
        int ret = sd_bus_open_system(&m_bus);
        if (ret != 0) {
            fprintf(stderr, "Failed to connect to the system bus: %s\n", strerror(-ret));
        }
        return ret;
    };
};

}

#endif

