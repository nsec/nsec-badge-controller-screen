#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern TaskHandle_t console_task_handle;

void console_task(void *parm);
void console_create_task();

#ifdef __cplusplus
}
#endif
