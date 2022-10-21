/* use '#define DCSBIOS_DEFAULT_SERIAL' instead if your Arduino board
 *  does not feature an ATMega328 or ATMega2650 controller.
 */
#define DCSBIOS_IRQ_SERIAL
#include "DcsBios.h"

// Left position (red) 
DcsBios::LED extPositionLightLeft(0x11bc, 0x0800, 13);
// Left strobe
DcsBios::LED extStrobeLeft(0x11bc, 0x4000, 12);
// Right strobe
DcsBios::LED extStrobeRight(0x11bc, 0x8000, 8);
// Right position (green)
DcsBios::LED extPositionLightRight(0x11bc, 0x1000, 7);

void setup() {
  DcsBios::setup();
}

void loop() {
  DcsBios::loop();
}
