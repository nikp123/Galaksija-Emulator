/// @file 

#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Checks whether file exists.
 * @param[in] file - UTF-8 encoded relative/absolute file path (native slashes)
 * @return bool - Whether the file exists or not
 **/
bool util_file_exists(const char *file);

/**
 * Checks the file size.
 * @param[in] file - UTF-8 encoded relative/absolute file path (native slashes)
 * @return size_t - System-specific file size (size_t), 0 is returned on error
 **/
size_t util_file_size(const char *file);

/**
 * Reads the contents of a file into a already allocated memory buffer. No safety checks.
 * @param[in] file - UTF-8 encoded relative/absolute file path (native slashes)
 * @param[out] buffer - memory buffer for the contents to be loaded into
 * @return size_t - System-specific file size (size_t), 0 is returned on error
 **/
size_t util_file_to_existing_buffer_unsafe(const char *file, void *buffer);

/**
 * Reads the contents of a file into a new memory buffer, managed by the user,
 * don't forget to free().
 * @param[in] file - UTF-8 encoded relative/absolute file path (native slashes)
 * @param[out] buffer - pointer to the pointer of the memory buffer where the contents are to be loaded into
 * @return size_t - System-specific file size (size_t), 0 is returned on error
 **/
size_t util_file_to_new_buffer(const char *file, void **buffer);

/**
 * Writes to a file buffer, but with the endianess swapped
 * @param[in] ptr - Pointer to data to be written from
 * @param[in] size - System specific element size
 * @param[in] nmemb - Element count to be written
 * @param[out] stream - File buffer to be written into
 * @return size_t - System-specific writen bytes
 **/
size_t util_fwrite_swapped_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Reads off of a file buffer, but with the endianess swapped
 * @param[out] ptr - Pointer to data to be read into
 * @param[in] size - System specific element size
 * @param[in] nmemb - Element count to be read
 * @param[in] stream - File buffer to be read from
 * @return size_t - System-specific read bytes
 **/
size_t util_fread_swapped_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Writes to a file buffer, but with the data treated little endian
 * @param[in] ptr - Pointer to data to be written from
 * @param[in] size - System specific element size
 * @param[in] nmemb - Element count to be written
 * @param[out] stream - File buffer to be written into
 * @return size_t - System-specific writen bytes
 **/
size_t util_fwrite_little_endian(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Reads off of a file buffer, but with the data treated little endian
 * @param[out] ptr - Pointer to data to be read into
 * @param[in] size - System specific element size
 * @param[in] nmemb - Element count to be read
 * @param[in] stream - File buffer to be read from
 * @return size_t - System-specific read bytes
 **/
size_t util_fread_little_endian(void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif

