#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <lib.h>
#include <registers.h>

#define BUFFER_DIM 128

//cada tecla tiene su nombre para poder ser identificada facilmente
typedef enum key_t
{
    VK_ESCAPE = 0x1,
    VK_1 = 0x2,
    VK_2 = 0x3,
    VK_3 = 0x4,
    VK_4 = 0x5,
    VK_5 = 0x6,
    VK_6 = 0x7,
    VK_7 = 0x8,
    VK_8 = 0x9,
    VK_9 = 0x0A,
    VK_0 = 0x0B,
    VK_OEM_MINUS = 0x0C,
    VK_OEM_PLUS = 0x0D,
    VK_BACK = 0x0E,
    VK_TAB = 0x0F,
    VK_Q = 0x10,
    VK_W = 0x11,
    VK_E = 0x12,
    VK_R = 0x13,
    VK_T = 0x14,
    VK_Y = 0x15,
    VK_U = 0x16,
    VK_I = 0x17,
    VK_O = 0x18,
    VK_P = 0x19,
    VK_OEM_4 = 0x1A,
    VK_OEM_6 = 0x1B,
    VK_RETURN = 0x1C,
    VK_LCONTROL = 0x1D,
    VK_A = 0x1E,
    VK_S = 0x1F,
    VK_D = 0x20,
    VK_F = 0x21,
    VK_G = 0x22,
    VK_H = 0x23,
    VK_J = 0x24,
    VK_K = 0x25,
    VK_L = 0x26,
    VK_OEM_1 = 0x27,
    VK_OEM_7 = 0x28,
    VK_OEM_3 = 0x29,
    VK_LSHIFT = 0x2A,
    VK_OEM_5 = 0x2B,
    VK_Z = 0x2C,
    VK_X = 0x2D,
    VK_C = 0x2E,
    VK_V = 0x2F,
    VK_B = 0x30,
    VK_N = 0x31,
    VK_M = 0x32,
    VK_OEM_COMMA = 0x33,
    VK_OEM_PERIOD = 0x34,
    VK_OEM_2 = 0x35,
    VK_RSHIFT = 0x36,
    VK_MULTIPLY = 0x37,
    VK_LMENU = 0x38,
    VK_SPACE = 0x39,
    VK_CAPITAL = 0x3A,
    VK_F1 = 0x3B,
    VK_F2 = 0x3C,
    VK_F3 = 0x3D,
    VK_F4 = 0x3E,
    VK_F5 = 0x3F,
    VK_F6 = 0x40,
    VK_F7 = 0x41,
    VK_F8 = 0x42,
    VK_F9 = 0x43,
    VK_F10 = 0x44,
    VK_NUMLOCK = 0x45,
    VK_SCROLL = 0x46,
    VK_HOME_NUMPAD = 0x47,
    VK_UP_NUMPAD = 0x48,
    VK_PRIOR_NUMPAD = 0x49,
    VK_SUBTRACT = 0x4A,
    VK_LEFT_NUMPAD = 0x4B,
    VK_CLEAR = 0x4C,
    VK_RIGHT_NUMPAD = 0x4D,
    VK_ADD = 0x4E,
    VK_END_NUMPAD = 0x4F,
    VK_DOWN_NUMPAD = 0x50,
    VK_NEXT_NUMPAD = 0x51,
    VK_INSERT_NUMPAD = 0x52,
    VK_DELETE_NUMPAD = 0x53,
    VK_SNAPSHOT = 0x54,
    VK_OEM_102 = 0x56,
    VK_F11 = 0x57,
    VK_F12 = 0x58,
    VK_OEM_WSCTRL = 0x5A,
    VK_DBE_KATAKANA = 0x5B,
    VK_OEM_JUMP = 0x5C,
    VK_DBE_FLUSHSTRING = 0x5D,
    VK_OEM_BACKTAB = 0x5E,
    VK_OEM_AUTO = 0x5F,
    VK_DBE_NOCODEINPUT = 0x62,
    VK_HELP = 0x63,
    VK_F13 = 0x64,
    VK_F14 = 0x65,
    VK_F15 = 0x66,
    VK_F16 = 0x67,
    VK_F17 = 0x68,
    VK_F18 = 0x69,
    VK_F19 = 0x6A,
    VK_F20 = 0x6B,
    VK_F21 = 0x6C,
    VK_F22 = 0x6D,
    VK_F23 = 0x6E,
    VK_OEM_PA3 = 0x6F,
    VK_OEM_RESET = 0x71,
    VK_ABNT_C1 = 0x73,
    VK_F24 = 0x76,
    VK_OEM_PA1 = 0x7B,
    VK_ABNT_C2 = 0x7E,
    VK_MEDIA_PREV_TRACK = -0x10,
    VK_MEDIA_NEXT_TRACK = -0x19,
    VK_RETURN_NUMPAD = -0x1C,
    VK_RCONTROL = -0x1D,
    VK_VOLUME_MUTE = -0x20,
    VK_LAUNCH_APP2 = -0x21,
    VK_MEDIA_PLAY_PAUSE = -0x22,
    VK_MEDIA_STOP = -0x24,
    VK_VOLUME_DOWN = -0x2E,
    VK_VOLUME_UP = -0x30,
    VK_BROWSER_HOME = -0x32,
    VK_DIVIDE = -0x35,
    VK_SNAPSHOT_NUMPAD = -0x37,
    VK_RMENU = -0x38,
    VK_CANCEL = -0x46,
    VK_HOME = -0x47,
    VK_UP = -0x48,
    VK_PRIOR = -0x49,
    VK_LEFT = -0x4B,
    VK_RIGHT = -0x4D,
    VK_END = -0x4F,
    VK_DOWN = -0x50,
    VK_NEXT = -0x51,
    VK_INSERT = -0x52,
    VK_DELETE = -0x53,
    VK_LWIN = -0x5B,
    VK_RWIN = -0x5C,
    VK_APPS = -0x5D,
    VK_SLEEP = -0x5F,
    VK_BROWSER_SEARCH = -0x65,
    VK_BROWSER_FAVORITES = -0x66,
    VK_BROWSER_REFRESH = -0x67,
    VK_BROWSER_STOP = -0x68,
    VK_BROWSER_FORWARD = -0x69,
    VK_BROWSER_BACK = -0x6A,
    VK_LAUNCH_APP1 = -0x6B,
    VK_LAUNCH_MAIL = -0x6C,
    VK_LAUNCH_MEDIA_SELECT = -0x6D
} key_t;

typedef enum action_t
{
    PRESSED, RELEASED
} action_t;

typedef struct kbEvent_t    //struct de un evento del teclado, almacena que tecla fue y si se apreto o no
{
    key_t key;
    action_t action;
} kbEvent_t;

static char printableKeys[2][128] =    //mapa de caracteres printeables para cada tecla no especial de la distribucion US en su versión normal y shifteada
        {//Default US keys
                {
                        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
                        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
                        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
                        'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
                        0, 0, 0, '+', 0
                },//Shifted US Keys
                {
                        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
                        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
                        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',  'Z',
                        'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
                        0, 0, 0, '+', 0
                }};

//rutina de atención
void keyboard_handler();

//syscalls
void readPrintables(char *bufferString, uint8_t count);

void getNextKey(struct kbEvent_t *kbEvent);

void cleanBuffer();

#endif
