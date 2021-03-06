/***********************************************************************
**** Includes
***********************************************************************/

#include <stdio.h>

#include <include/mpack/mpack.h>
#include <include/nanomsg/nn.h>

#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>

/***********************************************************************
**** Internals
***********************************************************************/

#define NANOLOG_WHERE "nanolog_handler"

static const char *_level_to_str[4] = {
        [LOG_INFO] = "info",
        [LOG_WARNING] = "warning",
        [LOG_ERROR] = "error",
        [LOG_DBG] = "debug"
};

extern "C" void ct_nano_log_handler(enum ct_log_level level,
                                    time_t time,
                                    char worker_id,
                                    const char *where,
                                    const char *msg,
                                    void *_data) {

    CETECH_STATIC_ASSERT_MSG(sizeof(void *) >= sizeof(int), "AUTHOR IS IDIOT");

    int socket = *((int *) _data); // TODO: problem if sizeof(void*) < int

    if (!socket) {
        return;
    }

    int bytes;
    char *data;
    size_t size;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);
    mpack_start_map(&writer, 5);

    mpack_write_cstr(&writer, "level");
    mpack_write_cstr(&writer, _level_to_str[level]);

    mpack_write_cstr(&writer, "time");
    mpack_write_i64(&writer, time);

    mpack_write_cstr(&writer, "worker_id");
    mpack_write_i8(&writer, worker_id);

    mpack_write_cstr(&writer, "where");
    mpack_write_cstr(&writer, where);

    mpack_write_cstr(&writer, "msg");
    mpack_write_cstr(&writer, msg);

    mpack_finish_map(&writer);

    CETECH_ASSERT(NANOLOG_WHERE, mpack_writer_destroy(&writer) == mpack_ok);

    bytes = nn_send(socket, data, size, 0);
    CETECH_ASSERT(NANOLOG_WHERE, (size_t) bytes == size);
    free(data);
}
