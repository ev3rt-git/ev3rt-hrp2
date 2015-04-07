#include <cstdio>
#include "libcpp-test.h"
#include "ev3api.h"

LibSampleClass::LibSampleClass() {
    member = 0x12345678;
}

void LibSampleClass::draw() {
    static char buf[256];
    sprintf(buf, "Lib Member is 0x%08x.", member);
    ev3_lcd_draw_string(buf, 0, 64);
}

extern void libcpp_test_cpp_echo_function(int val) {
    LibSampleClass a;
    a.draw();
    printf("%s(): %d\n", __FUNCTION__, val);
}

