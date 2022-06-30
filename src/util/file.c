#include <sys/stat.h>

#include "endianess.h"

#include "file.h"

bool util_file_exists(const char *file) {
    struct stat st;
    int result = stat(file, &st);
    return result == 0;
}

size_t util_file_size(const char *file) {
    struct stat st;
    stat(file, &st);
    return st.st_size;
}

size_t util_file_to_existing_buffer_unsafe(const char *file, void *buffer) {
    FILE *fp = fopen(file, "rb");
    if(fp == NULL) {
        return 0;
    }

    size_t size = util_file_size(file);

    fread(buffer, 1, size, fp);
    fclose(fp);

    return size;
}

size_t util_file_to_new_buffer(const char *file, void **buffer) {
    if(buffer == NULL) {
        return 0;
    }

    FILE *fp = fopen(file, "rb");
    if(fp == NULL) {
        return 0;
    }

    size_t size = util_file_size(file);

    (*buffer) = malloc(size);
    if((*buffer) == NULL) {
        fclose(fp);
        return 0;
    }

    fread((*buffer), 1, size, fp);
    fclose(fp);

    return size;
}

size_t util_fwrite_swapped_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    unsigned char *buffer_src = (unsigned char*)ptr;
    unsigned char *buffer_dst = (unsigned char*)calloc(nmemb, sizeof(unsigned char*)*size);
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t ix = 0; ix < size; ix++) {
            buffer_dst[size * i + (size - 1 - ix)] = buffer_src[size * i + ix];
        }
    }
    size_t result = fwrite(buffer_dst, size, nmemb, stream);
    free(buffer_dst);
    return result;
}

size_t util_fread_swapped_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    unsigned char *buffer_src = (unsigned char*)calloc(nmemb, sizeof(unsigned char*)*size);
    size_t result = fread(buffer_src, size, nmemb, stream);
    unsigned char *buffer_dst = (unsigned char*)ptr;
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t ix = 0; ix < size; ix++) {
            buffer_dst[size * i + (size - 1 - ix)] = buffer_src[size * i + ix];
        }
    }
    free(buffer_src);
    return result;
}

size_t util_fwrite_little_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    #ifdef UTIL_ENDIANESS_BIG
        return endian_swap_fwrite(ptr, size, nmemb, stream);
    #elif defined(UTIL_ENDIANESS_LITTLE)
        return fwrite(ptr, size, nmemb, stream);
    #else
    #error "I don't know what architecture this is!"
    #endif
}

size_t util_fread_little_endian(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    #ifdef UTIL_ENDIANESS_BIG
    return endian_swap_fread(ptr, size, nmemb, stream);
    #elif defined(UTIL_ENDIANESS_LITTLE)
    return fread(ptr, size, nmemb, stream);
    #else
    #error "I don't know what architecture this is!"
    #endif
}
