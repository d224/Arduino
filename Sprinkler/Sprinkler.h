#ifndef Sprinkler_h
#define Sprinkler_h

#define CH_NUM 4


#define MODE_INIT         0
#define MODE_OPERATIONAL  1
#define MODE_TEST         2

extern uint8_t Buttons;
extern uint8_t Mode;

#define IS_PRESSED(x) (( Buttons & (x) ) != 0)
#define BTN_MODE_TEST (0x1 << 3)
#define BTN_M0        (0x1 << 4)
#define BTN_M1        (0x1 << 5)
#define BTN_M2        (0x1 << 6)
#define BTN_M3        (0x1 << 7)

#endif //Sprinkler_h