#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

namespace xblab {

class Server {

private:
static uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size);
static void echo_write(uv_write_t *req, int status);
static void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
static void fatal(const char *what);

public:
    static int get_config(char* filename);
    static void on_new_connection(uv_stream_t *server, int status);
};

// C linkage for libuv callback
extern "C" {
    void on_connection(uv_stream_t *server, int status);
}

} // namespace xblab

#endif