#include "mousejacker_ducky.h"

static const char ducky_cmd_comment[] = {"REM"};
static const char ducky_cmd_delay[] = {"DELAY "};
static const char ducky_cmd_string[] = {"STRING "};
static const char ducky_cmd_altstring[] = {"ALTSTRING "};
static const char ducky_cmd_repeat[] = {"REPEAT "};

static uint8_t LOGITECH_HID_TEMPLATE[] =
    {0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t LOGITECH_HELLO[] = {0x00, 0x4F, 0x00, 0x04, 0xB0, 0x10, 0x00, 0x00, 0x00, 0xED};
static uint8_t LOGITECH_KEEPALIVE[] = {0x00, 0x40, 0x00, 0x55, 0x6B};

uint8_t prev_hid = 0;
static bool holding_ctrl = false;
static bool holding_shift = false;
static bool holding_alt = false;
static bool holding_gui = false;

#define RT_THRESHOLD 50
#define LOGITECH_MIN_CHANNEL 2
#define LOGITECH_MAX_CHANNEL 83
#define LOGITECH_KEEPALIVE_SIZE 5
#define LOGITECH_HID_TEMPLATE_SIZE 10
#define LOGITECH_HELLO_SIZE 10
#define TAG "mousejacker_ducky"

MJDuckyKey mj_ducky_keys[] = {{" ", 44, 0},         {"!", 30, 2},          {"\"", 52, 2},
                              {"#", 32, 2},         {"$", 33, 2},          {"%", 34, 2},
                              {"&", 36, 2},         {"'", 52, 0},          {"(", 38, 2},
                              {")", 39, 2},         {"*", 37, 2},          {"+", 46, 2},
                              {",", 54, 0},         {"-", 45, 0},          {".", 55, 0},
                              {"/", 56, 0},         {"0", 39, 0},          {"1", 30, 0},
                              {"2", 31, 0},         {"3", 32, 0},          {"4", 33, 0},
                              {"5", 34, 0},         {"6", 35, 0},          {"7", 36, 0},
                              {"8", 37, 0},         {"9", 38, 0},          {":", 51, 2},
                              {";", 51, 0},         {"<", 54, 2},          {"=", 46, 0},
                              {">", 55, 2},         {"?", 56, 2},          {"@", 31, 2},
                              {"A", 4, 2},          {"B", 5, 2},           {"C", 6, 2},
                              {"D", 7, 2},          {"E", 8, 2},           {"F", 9, 2},
                              {"G", 10, 2},         {"H", 11, 2},          {"I", 12, 2},
                              {"J", 13, 2},         {"K", 14, 2},          {"L", 15, 2},
                              {"M", 16, 2},         {"N", 17, 2},          {"O", 18, 2},
                              {"P", 19, 2},         {"Q", 20, 2},          {"R", 21, 2},
                              {"S", 22, 2},         {"T", 23, 2},          {"U", 24, 2},
                              {"V", 25, 2},         {"W", 26, 2},          {"X", 27, 2},
                              {"Y", 28, 2},         {"Z", 29, 2},          {"[", 47, 0},
                              {"\\", 49, 0},        {"]", 48, 0},          {"^", 35, 2},
                              {"_", 45, 2},         {"`", 53, 0},          {"a", 4, 0},
                              {"b", 5, 0},          {"c", 6, 0},           {"d", 7, 0},
                              {"e", 8, 0},          {"f", 9, 0},           {"g", 10, 0},
                              {"h", 11, 0},         {"i", 12, 0},          {"j", 13, 0},
                              {"k", 14, 0},         {"l", 15, 0},          {"m", 16, 0},
                              {"n", 17, 0},         {"o", 18, 0},          {"p", 19, 0},
                              {"q", 20, 0},         {"r", 21, 0},          {"s", 22, 0},
                              {"t", 23, 0},         {"u", 24, 0},          {"v", 25, 0},
                              {"w", 26, 0},         {"x", 27, 0},          {"y", 28, 0},
                              {"z", 29, 0},         {"{", 47, 2},          {"|", 49, 2},
                              {"}", 48, 2},         {"~", 53, 2},          {"BACKSPACE", 42, 0},
                              {"", 0, 0},           {"ALT", 0, 4},         {"SHIFT", 0, 2},
                              {"CTRL", 0, 1},       {"GUI", 0, 8},         {"SCROLLLOCK", 71, 0},
                              {"ENTER", 40, 0},     {"F12", 69, 0},        {"HOME", 74, 0},
                              {"F10", 67, 0},       {"F9", 66, 0},         {"ESCAPE", 41, 0},
                              {"PAGEUP", 75, 0},    {"TAB", 43, 0},        {"PRINTSCREEN", 70, 0},
                              {"F2", 59, 0},        {"CAPSLOCK", 57, 0},   {"F1", 58, 0},
                              {"F4", 61, 0},        {"F6", 63, 0},         {"F8", 65, 0},
                              {"DOWNARROW", 81, 0}, {"DELETE", 42, 0},     {"RIGHT", 79, 0},
                              {"F3", 60, 0},        {"DOWN", 81, 0},       {"DEL", 76, 0},
                              {"END", 77, 0},       {"INSERT", 73, 0},
                              {"NUMLOCK", 83, 0},     {"F5", 62, 0},
                              {"LEFTARROW", 80, 0}, {"RIGHTARROW", 79, 0}, {"PAGEDOWN", 78, 0},
                              {"PAUSE", 72, 0},     {"SPACE", 44, 0},      {"UPARROW", 82, 0},
                              {"F11", 68, 0},       {"F7", 64, 0},         {"UP", 82, 0},
                              {"LEFT", 80, 0},      {"NUM 1", 89, 0},      {"NUM 2", 90, 0},
                              {"NUM 3", 91, 0},     {"NUM 4", 92, 0},      {"NUM 5", 93, 0},
                              {"NUM 6", 94, 0},     {"NUM 7", 95, 0},      {"NUM 8", 96, 0},
                              {"NUM 9", 97, 0},     {"NUM 0", 98, 0}};

/*
static bool mj_ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(sscanf(param, "%lu", &value) == 1) {
        *val = value;
        return true;
    }
    return false;
}
*/

static uint32_t mj_ducky_get_command_len(const char* line) {
    uint32_t len = strlen(line);
    for(uint32_t i = 0; i < len; i++) {
        if(line[i] == ' ') return i;
    }
    return 0;
}

static bool mj_get_ducky_key(char* key, size_t keylen, MJDuckyKey* dk) {
    //FURI_LOG_D(TAG, "looking up key %s with length %d", key, keylen);
    for(size_t i = 0; i < sizeof(mj_ducky_keys) / sizeof(MJDuckyKey); i++) {
        if(strlen(mj_ducky_keys[i].name) == keylen &&
           !strncmp(mj_ducky_keys[i].name, key, keylen)) {
            memcpy(dk, &mj_ducky_keys[i], sizeof(MJDuckyKey));
            return true;
        }
    }

    return false;
}

static void checksum(uint8_t* payload, size_t len) {
    // This is also from the KeyKeriki paper
    // Thanks Thorsten and Max!
    uint8_t cksum = 0xff;
    for(size_t n = 0; n < len - 2; n++) cksum = (cksum - payload[n]) & 0xff;
    cksum = (cksum + 1) & 0xff;
    payload[len - 1] = cksum;
}

static void inject_packet(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    uint8_t* payload,
    size_t payload_size,
    PluginState* plugin_state) {
    uint8_t rt_count = 0;
    while(1) {
        if(!plugin_state->is_thread_running || plugin_state->close_thread_please) {
            return;
        }
        if(nrf24_txpacket(handle, payload, payload_size, true)) {
            break;
        }

        rt_count++;
        // retransmit threshold exceeded, scan for new channel
        if(rt_count > RT_THRESHOLD) {
            if(nrf24_find_channel(
                   handle,
                   addr,
                   addr,
                   addr_size,
                   rate,
                   LOGITECH_MIN_CHANNEL,
                   LOGITECH_MAX_CHANNEL,
                   true) > LOGITECH_MAX_CHANNEL) {
                return; // fail
            }
            //FURI_LOG_D("mj", "find channel passed, %d", tessst);

            rt_count = 0;
        }
    }
}

static void build_hid_packet(uint8_t mod, uint8_t hid, uint8_t* payload) {
    memcpy(payload, LOGITECH_HID_TEMPLATE, LOGITECH_HID_TEMPLATE_SIZE);
    payload[2] = mod;
    payload[3] = hid;
    checksum(payload, LOGITECH_HID_TEMPLATE_SIZE);
}

static void release_key(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    PluginState* plugin_state) {
    // This function release keys currently pressed, but keep pressing special keys
    // if holding mod keys variable are set to true

    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    build_hid_packet(
        0 | holding_ctrl | holding_shift << 1 | holding_alt << 2 | holding_gui << 3,
        0,
        hid_payload);
    inject_packet(
        handle,
        addr,
        addr_size,
        rate,
        hid_payload,
        LOGITECH_HID_TEMPLATE_SIZE,
        plugin_state); // empty hid packet
}

static void send_hid_packet(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    uint8_t mod,
    uint8_t hid,
    PluginState* plugin_state) {
    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    if(hid == prev_hid) release_key(handle, addr, addr_size, rate, plugin_state);

    prev_hid = hid;
    build_hid_packet(
        mod | holding_ctrl | holding_shift << 1 | holding_alt << 2 | holding_gui << 3,
        hid,
        hid_payload);
    inject_packet(
        handle, addr, addr_size, rate, hid_payload, LOGITECH_HID_TEMPLATE_SIZE, plugin_state);
    furi_delay_ms(12);
}

static bool ducky_end_line(const char chr) {
    return ((chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n'));
}

// returns false if there was an error processing script line
static bool mj_process_ducky_line(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    char* line,
    char* prev_line,
    PluginState* plugin_state) {
    MJDuckyKey dk;
    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    char* line_tmp = line;
    uint32_t line_len = strlen(line);
    if(!plugin_state->is_thread_running || plugin_state->close_thread_please) {
        return true;
    }
    for(uint32_t i = 0; i < line_len; i++) {
        if((line_tmp[i] != ' ') && (line_tmp[i] != '\t') && (line_tmp[i] != '\n')) {
            line_tmp = &line_tmp[i];
            break; // Skip spaces and tabs
        }
        if(i == line_len - 1) return true; // Skip empty lines
    }

    FURI_LOG_D(TAG, "line: %s", line_tmp);

    // General commands
    if(strncmp(line_tmp, ducky_cmd_comment, strlen(ducky_cmd_comment)) == 0) {
        // REM - comment line
        return true;
    } else if(strncmp(line_tmp, ducky_cmd_delay, strlen(ducky_cmd_delay)) == 0) {
        // DELAY
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        uint32_t delay_val = 0;
        delay_val = atoi(line_tmp);
        if(delay_val > 0) {
            uint32_t delay_count = delay_val / 10;
            build_hid_packet(0, 0, hid_payload);
            inject_packet(
                handle,
                addr,
                addr_size,
                rate,
                hid_payload,
                LOGITECH_HID_TEMPLATE_SIZE,
                plugin_state); // empty hid packet
            for(uint32_t i = 0; i < delay_count; i++) {
                if(!plugin_state->is_thread_running || plugin_state->close_thread_please) {
                    return true;
                }
                inject_packet(
                    handle,
                    addr,
                    addr_size,
                    rate,
                    LOGITECH_KEEPALIVE,
                    LOGITECH_KEEPALIVE_SIZE,
                    plugin_state);
                furi_delay_ms(10);
            }
            return true;
        }
        return false;
    } else if(strncmp(line_tmp, ducky_cmd_string, strlen(ducky_cmd_string)) == 0) {
        // STRING
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        for(size_t i = 0; i < strlen(line_tmp); i++) {
            if(!mj_get_ducky_key(&line_tmp[i], 1, &dk)) return false;

            send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        }

        return true;
    } else if(strncmp(line_tmp, ducky_cmd_altstring, strlen(ducky_cmd_altstring)) == 0) {
        // ALTSTRING
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        for(size_t i = 0; i < strlen(line_tmp); i++) {
            if((line_tmp[i] < ' ') || (line_tmp[i] > '~')) {
                continue; // Skip non-printable chars
            }

            char alt_code[4];
            // Getting altcode of the char
            snprintf(alt_code, 4, "%u", line_tmp[i]);

            uint8_t j = 0;
            while(!ducky_end_line(alt_code[j])) {
                char pad_num[5] = {'N', 'U', 'M', ' ', alt_code[j]};
                if(!mj_get_ducky_key(pad_num, 5, &dk)) return false;
                holding_alt = true;
                FURI_LOG_D(TAG, "Sending %s", pad_num);
                send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
                j++;
            }
            holding_alt = false;
            release_key(handle, addr, addr_size, rate, plugin_state);
        }

        return true;
    } else if(strncmp(line_tmp, ducky_cmd_repeat, strlen(ducky_cmd_repeat)) == 0) {
        // REPEAT
        uint32_t repeat_cnt = 0;
        if(prev_line == NULL) return false;

        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        repeat_cnt = atoi(line_tmp);
        if(repeat_cnt < 2) return false;

        FURI_LOG_D(TAG, "repeating %s %ld times", prev_line, repeat_cnt);
        for(uint32_t i = 0; i < repeat_cnt; i++)
            mj_process_ducky_line(handle, addr, addr_size, rate, prev_line, NULL, plugin_state);

        return true;
    } else if(strncmp(line_tmp, "ALT", strlen("ALT")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_alt = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_alt = false;
        return true;
    } else if(
        strncmp(line_tmp, "GUI", strlen("GUI")) == 0 ||
        strncmp(line_tmp, "WINDOWS", strlen("WINDOWS")) == 0 ||
        strncmp(line_tmp, "COMMAND", strlen("COMMAND")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_gui = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_gui = false;
        return true;
    } else if(
        strncmp(line_tmp, "CTRL-ALT", strlen("CTRL-ALT")) == 0 ||
        strncmp(line_tmp, "CONTROL-ALT", strlen("CONTROL-ALT")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_ctrl = true;
        holding_alt = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_ctrl = false;
        holding_alt = false;
        return true;
    } else if(
        strncmp(line_tmp, "CTRL-SHIFT", strlen("CTRL-SHIFT")) == 0 ||
        strncmp(line_tmp, "CONTROL-SHIFT", strlen("CONTROL-SHIFT")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_ctrl = true;
        holding_shift = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_ctrl = false;
        holding_shift = false;
        return true;
    } else if(
        strncmp(line_tmp, "CTRL", strlen("CTRL")) == 0 ||
        strncmp(line_tmp, "CONTROL", strlen("CONTROL")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_ctrl = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_ctrl = false;
        return true;
    } else if(strncmp(line_tmp, "SHIFT", strlen("SHIFT")) == 0) {
        line_tmp = &line_tmp[mj_ducky_get_command_len(line_tmp) + 1];
        if(!mj_get_ducky_key(line_tmp, strlen(line_tmp), &dk)) return false;
        holding_shift = true;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        holding_shift = false;
        return true;
    } else if(
        strncmp(line_tmp, "ESC", strlen("ESC")) == 0 ||
        strncmp(line_tmp, "APP", strlen("APP")) == 0 ||
        strncmp(line_tmp, "ESCAPE", strlen("ESCAPE")) == 0) {
        if(!mj_get_ducky_key("ESCAPE", 6, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(strncmp(line_tmp, "ENTER", strlen("ENTER")) == 0) {
        if(!mj_get_ducky_key("ENTER", 5, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(
        strncmp(line_tmp, "UP", strlen("UP")) == 0 ||
        strncmp(line_tmp, "UPARROW", strlen("UPARROW")) == 0) {
        if(!mj_get_ducky_key("UP", 2, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(
        strncmp(line_tmp, "DOWN", strlen("DOWN")) == 0 ||
        strncmp(line_tmp, "DOWNARROW", strlen("DOWNARROW")) == 0) {
        if(!mj_get_ducky_key("DOWN", 4, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(
        strncmp(line_tmp, "LEFT", strlen("LEFT")) == 0 ||
        strncmp(line_tmp, "LEFTARROW", strlen("LEFTARROW")) == 0) {
        if(!mj_get_ducky_key("LEFT", 4, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(
        strncmp(line_tmp, "RIGHT", strlen("RIGHT")) == 0 ||
        strncmp(line_tmp, "RIGHTARROW", strlen("RIGHTARROW")) == 0) {
        if(!mj_get_ducky_key("RIGHT", 5, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(strncmp(line_tmp, "SPACE", strlen("SPACE")) == 0) {
        if(!mj_get_ducky_key("SPACE", 5, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(strncmp(line_tmp, "TAB", strlen("TAB")) == 0) {
        if(!mj_get_ducky_key("TAB", 3, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    } else if(strncmp(line_tmp, "NUMLOCK", strlen("NUMLOCK")) == 0) {
        if(!mj_get_ducky_key("NUMLOCK", 7, &dk)) return false;
        send_hid_packet(handle, addr, addr_size, rate, dk.mod, dk.hid, plugin_state);
        return true;
    }

    return false;
}

void mj_process_ducky_script(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    char* script,
    PluginState* plugin_state) {
    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    char* prev_line = NULL;

    inject_packet(
        handle, addr, addr_size, rate, LOGITECH_HELLO, LOGITECH_HELLO_SIZE, plugin_state);
    char* line = strtok(script, "\n");
    while(line != NULL) {
        if(strcmp(&line[strlen(line) - 1], "\r") == 0) line[strlen(line) - 1] = (char)0;

        if(!mj_process_ducky_line(handle, addr, addr_size, rate, line, prev_line, plugin_state))
            FURI_LOG_D(TAG, "unable to process ducky script line: %s", line);

        prev_line = line;
        line = strtok(NULL, "\n");
    }
    build_hid_packet(0, 0, hid_payload);
    inject_packet(
        handle,
        addr,
        addr_size,
        rate,
        hid_payload,
        LOGITECH_HID_TEMPLATE_SIZE,
        plugin_state); // empty hid packet at end
}