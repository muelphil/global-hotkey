#include <napi.h>
#include <windows.h>
#include <atomic>
#include <vector>
#include <iostream>
#include <thread>

// Structure to represent a key event
struct KeyEvent {
    bool ctrl;
    bool meta;
    bool alt;
    bool shift;
    int32_t keyCode; // Virtual key code of the key, see https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
};

// Global variables
HHOOK hhkLowLevelKybd;  // Handle for the low-level keyboard hook
KBDLLHOOKSTRUCT kbdStruct;
std::vector<std::pair<int, KeyEvent>> keyEventVec;  // Vector to store registered key event listeners
std::thread worker;  // Thread for running the hook message loop
std::atomic<bool> stop_thread(false);  // Atomic flag to signal the thread to stop
Napi::ThreadSafeFunction tsfn;  // Thread-safe function for calling JavaScript callbacks

const int DUMMY_KEY_CODE = 0xFF;  // Dummy key code to prevent Start Menu activation

// Variables to track the state of modifier keys
bool isSuperKeyPressed = false;
bool isAltPressed = false;
bool isShiftPressed = false;
bool isControlPressed = false;

/**
 * Low-level keyboard hook callback function.
 *
 * @param nCode Hook code passed to the hook procedure.
 * @param wParam The virtual-key code of the key.
 * @param lParam A pointer to a KBDLLHOOKSTRUCT structure.
 * @return The result of calling the next hook procedure in the hook chain.
 */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {  // Process keyboard input
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

        // Check if a key is pressed down
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (kbdStruct.vkCode == VK_LWIN || kbdStruct.vkCode == VK_RWIN) {
                isSuperKeyPressed = true;
            } else if (kbdStruct.vkCode == VK_MENU || kbdStruct.vkCode == VK_LMENU || kbdStruct.vkCode == VK_RMENU) {
                isAltPressed = true;
            } else if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT) {
                isShiftPressed = true;
            } else if (kbdStruct.vkCode == VK_CONTROL || kbdStruct.vkCode == VK_LCONTROL || kbdStruct.vkCode == VK_RCONTROL) {
                isControlPressed = true;
            } else if (isSuperKeyPressed || isAltPressed || isControlPressed) {  // Check for registered key combinations
                for (const auto& pair : keyEventVec) {
                    KeyEvent event = pair.second;

                    // Check if the current key event matches a registered listener
                    if (event.ctrl == isControlPressed &&
                        event.shift == isShiftPressed &&
                        event.meta == isSuperKeyPressed &&
                        event.alt == isAltPressed &&
                        event.keyCode == kbdStruct.vkCode) {

                        if (isSuperKeyPressed) {
                            // Thanks to Windows PowerToys HotkeyManager.cpp line 35-40
                            // After invoking the hotkey send a dummy key to prevent Start Menu from activating
                            INPUT input = {0};
                            input.type = INPUT_KEYBOARD;
                            input.ki.wVk = DUMMY_KEY_CODE;
                            input.ki.dwFlags = KEYEVENTF_KEYUP;
                            SendInput(1, &input, sizeof(INPUT));
                        }

                        int id = pair.first;  // Get the listener ID
                        // Call the JavaScript callback associated with the listener ID
                        tsfn.BlockingCall([id](Napi::Env env, Napi::Function jsCallback) {
                            jsCallback.Call({Napi::Number::New(env, id)});
                        });
                        return 1;  // Prevent further processing of this key event
                    }
                }
            }
        }

        // Check if a key is released
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (kbdStruct.vkCode == VK_LWIN || kbdStruct.vkCode == VK_RWIN) {
                isSuperKeyPressed = false;
            } else if (kbdStruct.vkCode == VK_MENU || kbdStruct.vkCode == VK_LMENU || kbdStruct.vkCode == VK_RMENU) {
                isAltPressed = false;
            } else if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT) {
                isShiftPressed = false;
            } else if (kbdStruct.vkCode == VK_CONTROL || kbdStruct.vkCode == VK_LCONTROL || kbdStruct.vkCode == VK_RCONTROL) {
                isControlPressed = false;
            }
        }
    }
    return CallNextHookEx(hhkLowLevelKybd, nCode, wParam, lParam);  // Pass the hook information to the next hook procedure
}

/**
 * Thread function to manage the keyboard hook.
 *
 * @return 0 on successful completion.
 */
int ThreadFunction() {
    MSG msg;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Set the low-level keyboard hook
    hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    if (!hhkLowLevelKybd) {
        throw std::runtime_error("Failed to install hook!");
    }

    // Process messages until the stop flag is set
    while (GetMessage(&msg, NULL, 0, 0) && !stop_thread.load()) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hhkLowLevelKybd);  // Unhook the keyboard hook
    return 0;
}

/**
 * Starts the keyboard hook thread.
 *
 * @param info Napi::CallbackInfo object containing the callback function.
 */
void StartThread(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function jsCallback = info[0].As<Napi::Function>();

    stop_thread.store(false);  // Reset the stop flag
    tsfn = Napi::ThreadSafeFunction::New(env, jsCallback, "Callback", 0, 1);  // Create a thread-safe function
    worker = std::thread(ThreadFunction);  // Start the hook thread
    worker.detach();  // Detach the thread to run independently
}

/**
 * Adds a new key event listener.
 *
 * @param info Napi::CallbackInfo object containing the listener ID and key event details.
 */
void addListener(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Number id = info[0].As<Napi::Number>();

    if (!info[1].IsObject()) {
        Napi::TypeError::New(env, "Expected an object as second argument").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object obj = info[1].As<Napi::Object>();
    KeyEvent event;
    event.ctrl = obj.Get("ctrl").As<Napi::Boolean>().Value();
    event.shift = obj.Get("shift").As<Napi::Boolean>().Value();
    event.meta = obj.Get("meta").As<Napi::Boolean>().Value();
    event.alt = obj.Get("alt").As<Napi::Boolean>().Value();
    event.keyCode = obj.Get("keyCode").As<Napi::Number>().Int32Value();

    keyEventVec.push_back({id.Int32Value(), event});  // Add the listener to the vector
}

/**
 * Removes a key event listener by its ID.
 *
 * @param info Napi::CallbackInfo object containing the listener ID.
 */
void removeListener(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Number id = info[0].As<Napi::Number>();
    double nativeId = id.DoubleValue();
    for (auto it = keyEventVec.begin(); it != keyEventVec.end();) {
        if (it->first == nativeId) {
            it = keyEventVec.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Removes all event listener.
 *
 * @param info Napi::CallbackInfo object.
 */
void removeAllListeners(const Napi::CallbackInfo& info) {
    keyEventVec.clear();
}

/**
 * Stops the keyboard hook thread and releases resources.
 *
 * @param info Napi::CallbackInfo object.
 */
void StopThread(const Napi::CallbackInfo& info) {
    stop_thread.store(true);  // Set the stop flag to terminate the thread
    tsfn.Release();  // Release the thread-safe function
    if (worker.joinable()) {
        worker.join();  // Wait for the thread to finish
    }
}

/**
 * Initializes the N-API module and exports the functions.
 *
 * @param env Napi::Env object.
 * @param exports Napi::Object to which functions are added.
 * @return The exports object with functions added.
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "startThread"), Napi::Function::New(env, StartThread));
    exports.Set(Napi::String::New(env, "stopThread"), Napi::Function::New(env, StopThread));
    exports.Set(Napi::String::New(env, "removeListener"), Napi::Function::New(env, removeListener));
    exports.Set(Napi::String::New(env, "removeAllListeners"), Napi::Function::New(env, removeAllListeners));
    exports.Set(Napi::String::New(env, "addListener"), Napi::Function::New(env, addListener));
    return exports;
}

// Define the entry point for the Node.js module
NODE_API_MODULE(napi_threading, Init)