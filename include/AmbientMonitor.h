#ifndef AMBIENT_MONITOR_H
#define AMBIENT_MONITOR_H

#include <Arduino.h>
#include "../include/FlameTriangulation.h"

class AmbientMonitor {
public:
    AmbientMonitor(unsigned long checkInterval);
    void update(FlameTriangulation& flameSensor);
private:
    unsigned long checkInterval;
    unsigned long lastAmbientCheck;
};

#endif // AMBIENT_MONITOR_H
