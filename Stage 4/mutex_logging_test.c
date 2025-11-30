#include <stdio.h>
#include <stdbool.h>
#include "mutex_logging.h"



int main() {
    record_log("test");
    record_log("test");
    record_log("test");
    record_log("test");
    record_log("test");
    record_log("test");

    getchar(); // stops the command window from closing after its done
    return 0;
}
