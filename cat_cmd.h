#ifndef CAT_CMD_H
#define CAT_CMD_H

// 命令码
constexpr uint8_t CMD_CONNECT                 = 0x01;
constexpr uint8_t CMD_MONITOR                 = 0x02;
constexpr uint8_t CMD_MOUSE_BUTTON            = 0x03;
constexpr uint8_t CMD_KEYBOARD_BUTTON         = 0x04;
constexpr uint8_t CMD_BLOCKED                 = 0x05;
constexpr uint8_t CMD_UNBLOCKED_MOUSE_ALL     = 0x06;
constexpr uint8_t CMD_UNBLOCKED_KEYBOARD_ALL  = 0x07;
constexpr uint8_t CMD_MOUSE_MOVE              = 0x08;
constexpr uint8_t CMD_MOUSE_AUTO_MOVE         = 0x09;

struct CmdData {
    uint8_t cmd;
    uint16_t options;
    int16_t value1;
    int16_t value2;
} __attribute__((packed, aligned(1)));

struct MouseEvent {
    int16_t x;
    int16_t y;
    int16_t wheel;
} __attribute__((packed, aligned(1)));

struct MouseData {
    uint16_t code;
    uint16_t value;
    MouseEvent mouseEvent;
} __attribute__((packed, aligned(1)));

struct KeyboardData {
    uint16_t code;
    uint16_t value;
    uint8_t lock;
} __attribute__((packed, aligned(1)));

struct HidData {
    MouseData mouse_data;
    KeyboardData keyboard_data;
} __attribute__((packed, aligned(1)));;

#endif //CAT_CMD_H
