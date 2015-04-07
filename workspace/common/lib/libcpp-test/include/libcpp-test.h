#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void libcpp_test_c_echo_function(int val);

extern void libcpp_test_cpp_echo_function(int val);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class LibSampleClass {
public:
    LibSampleClass();

    void draw();
private:
    int member;
};
#endif
