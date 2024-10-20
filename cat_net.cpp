#include "cat_net.h"

void initialize(uint32_t, bool) {
}

void receive(const websocketpp::connection_hdl&, const websocketpp::server<websocketpp::config::asio_tls>::message_ptr& msg) {
    std::string payload = msg->get_payload();
    const auto *buf = reinterpret_cast<const unsigned char *>(payload.c_str());

    CmdData data{};
    if (payload.size() >= sizeof(CmdData)) {
        std::memcpy(&data, buf, sizeof(CmdData));
    } else {
        return;
    }

    switch (data.cmd) {
        case CMD_MONITOR:
            if (data.options & 0x01) {
                is_monitor = true;
            } else {
                is_monitor = false;
            }
            break;
        case CMD_MOUSE_BUTTON:
            mouseButtonPassThrough(data.options, data.value1);
            break;
        case CMD_KEYBOARD_BUTTON:
            keyboardPassThrough(data.options, data.value1);
            break;
        case CMD_BLOCKED:
            if (data.options & 0x01) {
                mouseBlocked(data.value1, data.value2);
            } else if (data.options & 0x02) {
                keyboardBlocked(data.value1, data.value2);
            }
            break;
        case CMD_UNBLOCKED_MOUSE_ALL:
            mouseAllUnblocked();
            break;
        case CMD_UNBLOCKED_KEYBOARD_ALL:
            keyboardAllUnblocked();
            break;
        case CMD_MOUSE_MOVE:
            mouseAxisPassThrough(REL_X, data.value1);
            mouseAxisPassThrough(REL_Y, data.value2);
            break;
        case CMD_MOUSE_AUTO_MOVE:
            mouseAutoMove(data.value1, data.value2, data.options);
            break;
        default:
            break;
    }
}

void mouseListen(struct input_event &ev) {
    if (!is_monitor) {
        return;
    }
    if (ev.type == EV_REL) {
        if (ev.code == REL_WHEEL) {
            hid_data.mouse_data.mouseEvent.wheel = static_cast<int16_t>(ev.value);
        } else {
            if (ev.code == REL_X) {
                hid_data.mouse_data.mouseEvent.x = static_cast<int16_t>(ev.value);
            } else if (ev.code == REL_Y) {
                hid_data.mouse_data.mouseEvent.y = static_cast<int16_t>(ev.value);
            }
        }
        sendHidData();
    } else if (ev.type == EV_KEY) {
        if (ev.value == 2) {
            return;
        }
        hid_data.mouse_data.code = ev.code;
        hid_data.mouse_data.value = ev.value;
        sendHidData();
    }
}

void keyboardListen(struct input_event &ev) {
    if (!is_monitor) {
        return;
    }
    if (ev.type == EV_KEY) {
        if (ev.value == 2) {
            return;
        }
        hid_data.keyboard_data.code = ev.code;
        hid_data.keyboard_data.value = ev.value;
        sendHidData();
    }
}

void lockKeyListen(uint8_t lockState) {
    if (!is_monitor) {
        return;
    }
    hid_data.keyboard_data.lock = lockState;
    sendHidData();
}

void cleanup() {
    is_monitor = false;
    std::memset(&hid_data, 0, sizeof(HidData));
}

void sendHidData() {
    unsigned char data_buf[sizeof(HidData)];
    memcpy(data_buf, &hid_data, sizeof(HidData));
    sendHid(data_buf, sizeof(HidData), websocketpp::frame::opcode::binary);
}

void mouseAutoMove(int x, int y, int ms) {
    double stepX = static_cast<double>(x) / ms;
    double stepY = static_cast<double>(y) / ms;

    double accumX = 0.0, accumY = 0.0;

    for (int i = 0; i < ms; ++i) {
        accumX += stepX;
        accumY += stepY;

        int moveX = static_cast<int>(round(accumX));
        int moveY = static_cast<int>(round(accumY));

        accumX -= moveX;
        accumY -= moveY;

        if (moveX != 0 || moveY != 0) {
            mouseAxisPassThrough(REL_X, static_cast<int16_t >(moveX));
            mouseAxisPassThrough(REL_Y, static_cast<int16_t >(moveY));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
