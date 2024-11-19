#ifndef MEOWPI_PROTOCOL_LIBRARY_H
#define MEOWPI_PROTOCOL_LIBRARY_H

/* ===========================================================
      Start
      -----------------------------------------------------------
      对盒子实现自定义协议接口抽象
      按需实现  initialize     receive      mouseListen
               keyboardListen lockKeyListen cleanup
      =========================================================== */
#include <asio.hpp>
#include <functional>
#include <linux/input.h>

/**
 * 发送鼠标按键HID事件
 * @param buffer 消息数据
 * @param sendpoint 目标信息（IP、端口）
 * @return std::size_t 消息数据长度
 */
typedef std::size_t (*SendHidCallback)(const asio::mutable_buffer &buffer, const asio::ip::udp::endpoint &sendpoint);
static SendHidCallback sendHid = nullptr;

/**
 * 鼠标按键HID事件
 * @param code 按键名
 * @param value 0释放 1按下
 */
typedef void (*MouseButtonPassThroughCallback)(uint16_t code, uint16_t value);
static MouseButtonPassThroughCallback mouseButtonPassThrough = nullptr;

/**
 * 触发鼠标相对移动HID事件
 * @param code x or y
 * @param value -32768-32767
 */
typedef void (*MouseAxisPassThroughCallback)(uint8_t code, int16_t value);
static MouseAxisPassThroughCallback mouseAxisPassThrough = nullptr;

/**
 * 键盘按键HID事件
 * @param code 按键名
 * @param value 0释放 1按下
 */
typedef void (*KeyboardPassThroughCallback)(uint16_t code, uint16_t value);
static KeyboardPassThroughCallback keyboardPassThrough = nullptr;

/**
 * 屏蔽鼠标HID事件
 * @param code 按键名
 * @param isBlocked true屏蔽 false取消屏蔽
 */
typedef void (*MouseBlockedCallback)(uint16_t code, bool isBlocked);
static MouseBlockedCallback mouseBlocked = nullptr;

/**
 * 屏蔽键盘HID事件
 * @param code 按键名
 * @param isBlocked true屏蔽 false取消屏蔽
 */
typedef void (*KeyboardBlockedCallback)(uint16_t code, bool isBlocked);
static KeyboardBlockedCallback keyboardBlocked = nullptr;

/**
 * 清除所有鼠标HID屏蔽
 */
typedef void (*MouseAllUnblockedCallback)();
static MouseAllUnblockedCallback mouseAllUnblocked = nullptr;

/**
 * 清除所有键盘HID屏蔽
 */
typedef void (*KeyboardAllUnblockedCallback)();
static KeyboardAllUnblockedCallback keyboardAllUnblocked = nullptr;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 盒子传递的参数配置，根据需要进行设置
 * @param uuid uuid
 * @param is_encrypt 是否开启加密 true开启 false关闭
 */
void initialize(uint32_t uuid, bool is_encrypt);

/**
 * 服务端发送到盒子的消息回调
 * @param received_endpoint 服务端信息（IP、端口）
 * @param buffer 消息数据
 * @param buffer_len 消息数据长度
 */
void
receive(const asio::ip::udp::endpoint &received_endpoint, const std::array<char, 1024> &buffer, std::size_t buffer_len);

/**
 * 盒子鼠标HID监听回调
 * @param ev 按键信息
 */
void mouseListen(struct input_event &ev);

/**
 * 盒子键盘HID监听回调
 * @param ev 按键信息
 */
void keyboardListen(struct input_event &ev);

/**
 * 盒子键盘锁定键（NumLock、CapsLock、ScrollLock）监听回调
 * @param lockState 按键信息
 */
void lockKeyListen(uint8_t lockState);

/**
 * 退出时清理
 */
void cleanup();

void setSendCallback(SendHidCallback cb) {
    sendHid = cb;
}

void setMouseButtonPassThroughCallback(MouseButtonPassThroughCallback cb) {
    mouseButtonPassThrough = cb;
}

void setMouseAxisPassThroughCallback(MouseAxisPassThroughCallback cb) {
    mouseAxisPassThrough = cb;
}

void setKeyboardPassThroughCallback(KeyboardPassThroughCallback cb) {
    keyboardPassThrough = cb;
}

void setMouseBlockedCallback(MouseBlockedCallback cb) {
    mouseBlocked = cb;
}

void setKeyboardBlockedCallback(KeyboardBlockedCallback cb) {
    keyboardBlocked = cb;
}

void setMouseAllUnblockedCallback(MouseAllUnblockedCallback cb) {
    mouseAllUnblocked = cb;
}

void setKeyboardAllUnblockedCallback(KeyboardAllUnblockedCallback cb) {
    keyboardAllUnblocked = cb;
}
#ifdef __cplusplus
}
#endif
/* ===========================================================
      end
      =========================================================== */

// MeowPi 协议
#include "cat_cmd.h"

unsigned char *m_key;

bool is_monitor = false;
asio::ip::udp::endpoint target_endpoint;

HidData hid_data{};

void mouseAutoMove(int x, int y, int ms);

void send(const std::string &msg, const asio::ip::udp::endpoint &received_endpoint);

void sendAck(CmdData data, const asio::ip::udp::endpoint &received_endpoint);

void sendHidData();

int aes128CBCEncrypt(const unsigned char *buf, int buf_len, const unsigned char *key, const unsigned char *iv,
                     unsigned char *encrypt_buf);

int aes128CBCDecrypt(const unsigned char *encrypt_buf, int encrypt_buf_len, const unsigned char *key,
                     unsigned char *decrypt_buf);

unsigned char *expandTo16Bytes(uint32_t mac);

#endif //MEOWPI_PROTOCOL_LIBRARY_H
