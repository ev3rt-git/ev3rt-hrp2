#pragma once

ER application_load_menu();
void application_terminate_request();
void application_terminate_wait();

// internal use only, not thread-safe
void apploader_store_file(const char *filepath, void *buffer, uint32_t size);
