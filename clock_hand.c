#include <stdint.h>
#include <stdio.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "gpio.h"

//hand 1: P0_5
#define HAND_ONE_PORT   MXC_GPIO0
#define HAND_ONE_MASK   MXC_GPIO_PIN_5

//hand 2: P0_6
#define HAND_TWO_PORT   MXC_GPIO0
#define HAND_TWO_MASK   MXC_GPIO_PIN_6

//hand 3: P2_6
#define HAND_THREE_PORT MXC_GPIO2
#define HAND_THREE_MASK MXC_GPIO_PIN_6

//hand 4: P2_7
#define HAND_FOUR_PORT  MXC_GPIO2
#define HAND_FOUR_MASK  MXC_GPIO_PIN_7

//switch: P0_7
#define SWITCH_PORT    MXC_GPIO0
#define SWITCH_MASK    MXC_GPIO_PIN_7

//stuff for switch handling
#define STEP_DELAY 100
static int current_hand = 0;
static int direction = 1; //1 makes it clockwise, -1 makes it counterclockwise
static uint8_t last_switch_state = 0; // last debounced state (0 = not pressed, 1 = pressed)

static void init(void){
    mxc_gpio_cfg_t cfg;

    //-----init pins for hands-----
    //P0_5
    cfg.port  = HAND_ONE_PORT;
    cfg.mask  = HAND_ONE_MASK;
    cfg.func  = MXC_GPIO_FUNC_OUT;
    cfg.pad   = MXC_GPIO_PAD_NONE;
    cfg.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&cfg);

    //P0_6
    cfg.port  = HAND_TWO_PORT;
    cfg.mask  = HAND_TWO_MASK;
    cfg.func  = MXC_GPIO_FUNC_OUT;
    cfg.pad   = MXC_GPIO_PAD_NONE;
    cfg.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&cfg);

    //P2_6
    cfg.port  = HAND_THREE_PORT;
    cfg.mask  = HAND_THREE_MASK;
    cfg.func  = MXC_GPIO_FUNC_OUT;
    cfg.pad   = MXC_GPIO_PAD_NONE;
    cfg.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&cfg);

    //P2_7
    cfg.port  = HAND_FOUR_PORT;
    cfg.mask  = HAND_FOUR_MASK;
    cfg.func  = MXC_GPIO_FUNC_OUT;
    cfg.pad   = MXC_GPIO_PAD_NONE;
    cfg.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&cfg);

    //-----init pin for switch-----
    cfg.port  = SWITCH_PORT;
    cfg.mask  = SWITCH_MASK;
    cfg.func  = MXC_GPIO_FUNC_IN;
    cfg.pad   = MXC_GPIO_PAD_PULL_UP;
    cfg.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&cfg);

    //start with all hands low
    MXC_GPIO_OutClr(HAND_ONE_PORT,  HAND_ONE_MASK);
    MXC_GPIO_OutClr(HAND_TWO_PORT,  HAND_TWO_MASK);
    MXC_GPIO_OutClr(HAND_THREE_PORT, HAND_THREE_MASK);
    MXC_GPIO_OutClr(HAND_FOUR_PORT, HAND_FOUR_MASK);
}


static void set_hand_high(uint8_t hand_index){
    //clear all outputs first
    MXC_GPIO_OutClr(HAND_ONE_PORT,  HAND_ONE_MASK);
    MXC_GPIO_OutClr(HAND_TWO_PORT,  HAND_TWO_MASK);
    MXC_GPIO_OutClr(HAND_THREE_PORT, HAND_THREE_MASK);
    MXC_GPIO_OutClr(HAND_FOUR_PORT, HAND_FOUR_MASK);

    switch (hand_index) {
    case 0:
        MXC_GPIO_OutSet(HAND_ONE_PORT, HAND_ONE_MASK);
        break;
    case 1:
        MXC_GPIO_OutSet(HAND_TWO_PORT, HAND_TWO_MASK);
        break;
    case 2:
        MXC_GPIO_OutSet(HAND_THREE_PORT, HAND_THREE_MASK);
        break;
    case 3:
        MXC_GPIO_OutSet(HAND_FOUR_PORT, HAND_FOUR_MASK);
        break;
    }
}

static uint8_t is_switch_pressed(void){
    return MXC_GPIO_InGet(SWITCH_PORT, SWITCH_MASK) == 0;
}

int main(void){
    init();
    last_switch_state = is_switch_pressed(); 
    printf("init direction: %d\n", direction);

    while (1) {
        // --- handle button: toggle direction on new press ---
        uint8_t now_pressed = is_switch_pressed();

        // detect rising edge of "pressed" (not pressed -> pressed)
        if (now_pressed && !last_switch_state) {
            // simple debounce
            MXC_Delay(MXC_DELAY_MSEC(20)); // 20ms debounce
            if (is_switch_pressed()) {
                direction = -direction; // flip direction
            }
        }
        last_switch_state = now_pressed;

        // --- drive current hand ---
        set_hand_high((uint8_t) current_hand);
        MXC_Delay(MXC_DELAY_MSEC(STEP_DELAY));

        // --- advance to next hand according to direction ---
        current_hand += direction;

        if (current_hand > 3) {
            current_hand = 0;
        } 
        else if (current_hand < 0) {
            current_hand = 3;
        }
        printf("current direction: %d\n", direction);
        printf("current hand: %d\n", current_hand);
    }

    return 0;
}