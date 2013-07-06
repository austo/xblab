#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

namespace xblab {

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size);
void echo_write(uv_write_t *req, int status);
void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
void fatal(const char *what);
int get_config(char* filename);


extern "C" {
    void on_new_connection(uv_stream_t *server, int status);
}


} // namespace xblab

#endif