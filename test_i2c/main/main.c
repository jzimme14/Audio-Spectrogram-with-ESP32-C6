#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "sdkconfig.h"
#include "esp_log.h"

void app_main(void)
{
    while (true) {
        printf("Hello from app_main!\n");
        sleep(1);
    }
}
