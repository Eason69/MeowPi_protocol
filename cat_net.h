#ifndef MEOWPI_PROTOCOL_LIBRARY_H
#define MEOWPI_PROTOCOL_LIBRARY_H

/* ===========================================================
      Start
      -----------------------------------------------------------
      对盒子实现自定义协议接口抽象
      按需实现  initialize     runServer   stopServer   mouseListen
               keyboardListen lockKeyListen cleanup
      =========================================================== */
#include <functional>
#include <linux/input.h>

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
 * 开启网络服务
 * @param server_port 端口
 * @return 是否启动成功 true or false
 */
bool runServer(uint32_t server_port);

/**
 * 关闭网络服务
 */
void stopServer();

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

#endif //MEOWPI_PROTOCOL_LIBRARY_H
