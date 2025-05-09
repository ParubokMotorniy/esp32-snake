#ifndef INIT_H
#define INIT_H

#include <cstdint>

const uint16_t screenWidth = 320;
const uint16_t screenHeight = 240;
namespace Drivers {
void initHardware(void);

void update(void);
}
#endif
