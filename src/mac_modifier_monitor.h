#pragma once

#ifdef __APPLE__
typedef void (*MacTriggerCallback)(bool pressed, void* ctx);
void* startMacModifierMonitor(MacTriggerCallback cb, void* ctx);
void stopMacModifierMonitor(void* handle);
#endif
