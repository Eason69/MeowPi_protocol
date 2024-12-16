#include "cat_net.h"
#include <iomanip>
#include <cmath>
#include <iostream>

void initialize(uint32_t uuid, bool) {
    m_key = expandTo16Bytes(uuid);
}

void receive(const asio::ip::udp::endpoint &received_endpoint, const std::array<char, 1024> &buffer, std::size_t buffer_len) {
    const auto *encrypt_buf = reinterpret_cast<const unsigned char *>(buffer.data());
    unsigned char decrypt_buf[1024];
    int decrypt_len = aes128CBCDecrypt(encrypt_buf, static_cast<int>(buffer_len), m_key, decrypt_buf);

    CmdData data{};
    if (decrypt_len >= sizeof(CmdData)) {
        std::memcpy(&data, decrypt_buf, sizeof(CmdData));
    } else {
        return;
    }
    switch (data.cmd) {
        case CMD_CONNECT:
            target_endpoint.address(received_endpoint.address());
            sendAck("ok", received_endpoint);
            break;
        case CMD_MONITOR:
            if (data.options > 0) {
                target_endpoint.port(data.options);
                is_monitor = true;
            } else {
                is_monitor = false;
            }
            sendAck("ok", received_endpoint);
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
            sendAck("ok", received_endpoint);
            break;
        default:
            break;
    }
//     send("ok", received_endpoint);
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
    std::memset(m_key, 0, AES_BLOCK_SIZE);
    std::memset(&hid_data, 0, sizeof(HidData));
}

void sendAck(const std::string &msg, const asio::ip::udp::endpoint &received_endpoint) {
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);
    unsigned char encrypt_buf[1024];
    int encrypt_len = aes128CBCEncrypt(reinterpret_cast<const unsigned char *>(msg.c_str()),
                                       static_cast<int>(msg.length()), m_key, iv, encrypt_buf);
    if (encrypt_len <= 0) {
        return;
    }
    sendHid(asio::buffer(encrypt_buf, encrypt_len), received_endpoint);
}

void send(const std::string &msg, const asio::ip::udp::endpoint &received_endpoint) {
    std::vector<char> buffer(msg.begin(), msg.end());
    sendHid(asio::buffer(buffer), received_endpoint);
}

void sendHidData() {
    unsigned char data_buf[sizeof(HidData)];
    memcpy(data_buf, &hid_data, sizeof(HidData));
    unsigned char encrypt_buf[1024];
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);
    int encrypt_len = aes128CBCEncrypt(data_buf, sizeof(HidData), m_key, iv, encrypt_buf);
    if (encrypt_len <= 0) {
        return;
    }
    sendHid(asio::buffer(encrypt_buf, encrypt_len), target_endpoint);
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

int aes128CBCEncrypt(const unsigned char *buf, int buf_len, const unsigned char *key, unsigned char *iv,
                     unsigned char *encrypt_buf) {
    AES_KEY encryptKey;
    int padding_len = AES_BLOCK_SIZE - (buf_len % AES_BLOCK_SIZE);
    int total_len = buf_len + padding_len;
    unsigned char temp_buf[total_len];

    memcpy(temp_buf, buf, buf_len);
    memset(temp_buf + buf_len, padding_len, padding_len);

    if (AES_set_encrypt_key(key, 128, &encryptKey) < 0) {
        return -1;
    }

    memcpy(encrypt_buf, iv, AES_BLOCK_SIZE);

    AES_cbc_encrypt(temp_buf, encrypt_buf + AES_BLOCK_SIZE, total_len, &encryptKey, iv, AES_ENCRYPT);

    return total_len + AES_BLOCK_SIZE;
}

int aes128CBCDecrypt(const unsigned char *encrypt_buf, int encrypt_buf_len, const unsigned char *key,
                     unsigned char *decrypt_buf) {
    AES_KEY decryptKey;
    unsigned char iv[AES_BLOCK_SIZE];

    memcpy(iv, encrypt_buf, AES_BLOCK_SIZE);

    if (AES_set_decrypt_key(key, 128, &decryptKey) < 0) {
        return -1;
    }

    int len = encrypt_buf_len - AES_BLOCK_SIZE;
    int padding_len = 0;
    unsigned char temp_buf[len];

    AES_cbc_encrypt(encrypt_buf + AES_BLOCK_SIZE, temp_buf, len, &decryptKey, iv, AES_DECRYPT);
    padding_len = temp_buf[len - 1];
    memcpy(decrypt_buf, temp_buf, len - padding_len);

    return len - padding_len;
}

unsigned char *expandTo16Bytes(uint32_t uuid) {
    static unsigned char enc_key[16];
    std::memset(enc_key, 0, sizeof(enc_key));

    std::stringstream ss_uuid;
    ss_uuid << std::hex << std::setw(8) << std::setfill('0') << uuid;
    std::string str_uuid = ss_uuid.str();

    for (size_t i = 0; i < 16; ++i) {
        enc_key[i] = (i < str_uuid.size()) ? str_uuid[i] : '0';
    }

    return enc_key;
}
