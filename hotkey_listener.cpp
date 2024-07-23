#include <napi.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

HHOOK hhkLowLevelKybd;
KBDLLHOOKSTRUCT kbdStruct;
bool isSuperKeyPressed = false;
bool isAltPressed = false;
bool isShiftPressed = false;
bool isControlPressed = false;
bool ignoreNextSuper = false;

struct KeyEvent {
  bool ctrl;
  bool meta;
  bool alt;
  bool shift;
  int keyCode;
};

std::vector<std::pair<int, KeyEvent>> keyEventVec;

std::thread worker;
std::atomic<bool> stop_thread(false);
Napi::ThreadSafeFunction tsfn;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  //    std::cout << "VK_MENU=" << VK_ME
  if (nCode == HC_ACTION) {
    kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
      if (kbdStruct.vkCode == VK_LWIN || kbdStruct.vkCode == VK_RWIN) {
        isSuperKeyPressed = true;
      } else if (kbdStruct.vkCode == VK_LMENU || kbdStruct.vkCode == VK_RMENU) {
        isAltPressed = true;
      } else if (kbdStruct.vkCode == VK_LSHIFT ||
                 kbdStruct.vkCode == VK_RSHIFT) {
        isShiftPressed = true;
      } else if (kbdStruct.vkCode == VK_LCONTROL) {
        isControlPressed = true;
      } else if (isSuperKeyPressed || isAltPressed ||
                 isControlPressed) {  // only detect key combinations?

        for (const auto& pair : keyEventVec) {
          KeyEvent event = pair.second;
          if (event.ctrl == isControlPressed && event.shift == isShiftPressed &&
              event.meta == isSuperKeyPressed && event.alt == isAltPressed &&
              event.keyCode == kbdStruct.vkCode) {
            if (isSuperKeyPressed) {
              ignoreNextSuper = true;
            }
            int id = pair.first;
            tsfn.BlockingCall([id](Napi::Env env, Napi::Function jsCallback) {
              Napi::Value napiId = Napi::Number::New(env, id);
              jsCallback.Call({napiId});
            });
            return 1;
          }
        }
      }
    }
    if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
      if (kbdStruct.vkCode == VK_LWIN || kbdStruct.vkCode == VK_RWIN) {
        isSuperKeyPressed = false;
        if (ignoreNextSuper) {
          // Prevent the default behavior
          ignoreNextSuper = false;
          return 1;
        }
      } else if (kbdStruct.vkCode == VK_MENU) {
        isAltPressed = false;
      } else if (kbdStruct.vkCode == VK_SHIFT) {
        isShiftPressed = false;
      } else if (kbdStruct.vkCode == VK_CONTROL) {
        isControlPressed = false;
      }
    }
  }
  return CallNextHookEx(hhkLowLevelKybd, nCode, wParam, lParam);
}

int ThreadFunctionTwo() {
  MSG msg;
  HINSTANCE hInstance = GetModuleHandle(NULL);
  hhkLowLevelKybd =
      SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
  if (!hhkLowLevelKybd) {
    std::cerr << "Failed to install hook!\n";
    return 1;
  }

  while (!stop_thread.load() & GetMessage(&msg, NULL, 0, 0)) {
    if (stop_thread.load()) break;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hhkLowLevelKybd);
  return 0;
}

void StartThread(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function jsCallback = info[0].As<Napi::Function>();

  stop_thread.store(false);
  tsfn = Napi::ThreadSafeFunction::New(env, jsCallback, "Callback", 0, 1);
  worker = std::thread(ThreadFunctionTwo);
  worker.detach();
}

void addListener(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Number id = info[0].As<Napi::Number>();
  if (!info[1].IsObject()) {
    Napi::TypeError::New(env, "Expected an object")
        .ThrowAsJavaScriptException();
    return;
  }
  Napi::Object obj = info[1].As<Napi::Object>();
  bool ctrl = obj.Get("ctrl").As<Napi::Boolean>().Value();
  bool shift = obj.Get("shift").As<Napi::Boolean>().Value();
  bool meta = obj.Get("meta").As<Napi::Boolean>().Value();
  bool alt = obj.Get("alt").As<Napi::Boolean>().Value();
  std::string keyCode = obj.Get("keyCode").As<Napi::String>().Utf8Value();

  // Check if the string length is 1
  if (keyCode.length() != 1) {
    Napi::TypeError::New(env, "String must be exactly one character long")
        .ThrowAsJavaScriptException();
    return;
  }
  // Extract the character
  char character = keyCode[0];
  int keycode = static_cast<int>(character);

  keyEventVec.push_back({id, {ctrl, meta, alt, shift, keycode}});
}

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
  // TODO return number indicating if it was successful
}

void StopThread(const Napi::CallbackInfo& info) {
  stop_thread.store(true);
  tsfn.Release();
  if (worker.joinable()) {
    worker.join();
  }
}

int printOnShortcut() {
  MSG msg;
  HINSTANCE hInstance = GetModuleHandle(NULL);
  hhkLowLevelKybd =
      SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
  if (!hhkLowLevelKybd) {
    std::cerr << "Failed to install hook!\n";  // TODO js error
    return 1;
  }

  // Main message loop
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    // Exit if ESC is pressed
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
      break;
    }
  }

  UnhookWindowsHookEx(hhkLowLevelKybd);
  return 0;
}

void PrintOnShortcut(const Napi::CallbackInfo& info) { printOnShortcut(); }

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "startThread"), Napi::Function::New(env, StartThread));
  exports.Set(Napi::String::New(env, "stopThread"), Napi::Function::New(env, StopThread));
  exports.Set(Napi::String::New(env, "removeListener"), Napi::Function::New(env, removeListener));
  exports.Set(Napi::String::New(env, "addListener"), Napi::Function::New(env, addListener));
  exports.Set(Napi::String::New(env, "printOnShortcut"), Napi::Function::New(env, PrintOnShortcut));
  return exports;
}

NODE_API_MODULE(napi_threading, Init)