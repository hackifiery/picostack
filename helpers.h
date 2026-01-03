#pragma once
#include <stdbool.h>

bool isInt(const char* str);
char* safe_realloc(char* original_ptr, size_t new_size);
bool isKeyw(char* c);
void strip_char(char* str, const char stripval);
void process_str_tok(char* str);
char** split_str(const char* str, const char* delims, int* count_out);
bool isString(char* str);
int* string_to_params(char* str, int* out_len);
void insert_int_array(int** arr, int* arr_len, const int* insert, const int insert_len, int index);
int* slice_int_array(const int* arr, int arr_len, int start, int end, int* out_len);
char** split_lines(const char* src, int* out_count);