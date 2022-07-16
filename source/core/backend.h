/* date = July 5th 2022 6:32 pm */

#ifndef BACKEND_H
#define BACKEND_H

#include "defines.h"
#include "base/base.h"

#include "os/window.h"

//~ Initialization
void B_BackendInit(OS_Window* window);
void B_BackendInitShared(OS_Window* window, OS_Window* share);

void B_BackendSelectRenderWindow(OS_Window* window);
void B_BackendSwapchainNext(OS_Window* window);
void B_BackendFree(OS_Window* window);

#endif //BACKEND_H
