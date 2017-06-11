// author : lizs
// 2017.2.22
#pragma once

#ifdef NET_DYNAMIC
#ifdef EXPORT
#define NET_API __declspec( dllexport )
#else
#define NET_API __declspec( dllimport )
#endif
#else
#define NET_API
#endif

#include "uv.h"
#include <memory>
#include <functional>

#pragma region("����")
#define MESSAGE_TRACK_ENABLED false
#define TRACK_MESSAGE_AS_BINARY true
#define MONITOR_ENABLED true
#define CBUF_RESERVED_SIZE 32
#define MAX_PACKAGE_SIZE 1024
#define KEEP_ALIVE_INTERVAL 10 * 1000
#define KEEP_ALIVE_COUNTER_DEAD_LINE 5
#pragma endregion

#define PRINT_UV_ERR(code)         \
 do {                                                     \
	if (code) {                                          \
		fprintf(stderr,                                       \
            "%s : %s\n",    \
			uv_err_name(code),				\
			uv_strerror(code));                                       \
	 }                                                       \
 } while (0)

#define PRINT_UV_ERR_DETAIL(code)         \
 do {                                                     \
	if (code) {                                          \
		fprintf(stderr,                                       \
            "%s : %s [%s : %s : %d]\n",    \
			uv_err_name(code),				\
			uv_strerror(code),					\
            __FILE__,                                     \
			__FUNCTION__	,							\
            __LINE__);                                       \
	}                                                       \
 } while (0)

#define LOG_UV_ERR(code) PRINT_UV_ERR(code)

#define LOG(format, ...)         \
 do {                                                     \
    fprintf(stdout,                                       \
            format,										\
			##__VA_ARGS__);                                       \
	printf("\n");                                      \
 } while (0)

/* Die with fatal error. */
#define FATAL(msg)                                        \
  do {                                                    \
    fprintf(stderr,                                       \
            "Fatal error in %s on line %d: %s\n",         \
            __FILE__,                                     \
            __LINE__,                                     \
            msg);                                         \
    fflush(stderr);                                       \
    abort();                                              \
  } while (0)

#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)


#define PRINT_MESSAGE_AS_BINARY(buf, len, format, ...)         \
 do {                                                     \
    fprintf(stdout,                                       \
            format,										\
			##__VA_ARGS__);                                       \
	printf("[");                                      \
	for (auto i = 0; i < len; ++i) {			\
		if(i == len -1)	\
			printf("%02x", buf[i]);	\
		else	\
			printf("%02x ", buf[i]);				\
	}													\
	printf("]\n");                                      \
 } while (0)


#define PRINT_MESSAGE_AS_STRING(buf, len, format, ...)         \
 do {                                                     \
    fprintf(stdout,                                       \
            format,										\
			##__VA_ARGS__);                                       \
	printf("[");                                      \
		printf(buf);				\
	printf("]\n");                                      \
 } while (0)

#if TRACK_MESSAGE_AS_BINARY
#define PRINT_MESSAGE PRINT_MESSAGE_AS_BINARY
#else
#define PRINT_MESSAGE PRINT_MESSAGE_AS_STRING
#endif

/* This macro cleans up the main loop. This is used to avoid valgrind
* warnings about memory being "leaked" by the main event loop.
*/
#define MAKE_VALGRIND_HAPPY()                       \
  do {                                              \
    close_loop(uv_default_loop());                  \
    auto r = uv_loop_close(uv_default_loop());  \
	LOG_UV_ERR(r);	\
  } while (0)

/* Fully close a loop */
static void close_walk_cb(uv_handle_t* handle, void* arg) {
	if (!uv_is_closing(handle))
		uv_close(handle, nullptr);
}

static void close_loop(uv_loop_t* loop) {
	uv_walk(loop, close_walk_cb, nullptr);
	uv_run(loop, UV_RUN_DEFAULT);
}

#define RUN_UV_DEFAULT_LOOP()		\
do{												\
	int r;												\
	uv_loop_t* loop;							\
	loop = uv_default_loop();			\
	Singleton<Monitor>::instance().start(); \
	r = uv_run(loop, UV_RUN_DEFAULT);	\
	if(r)												\
		LOG_UV_ERR(r);						\
	MAKE_VALGRIND_HAPPY();		\
}while(0)


#pragma region("typedefs")

typedef int16_t ops_t;
typedef uint8_t byte_t;
typedef uint16_t pack_size_t;
typedef uint16_t serial_t;
typedef uint16_t error_no_t;
typedef uint16_t node_id_t;
typedef uint8_t node_type_t;
typedef uint16_t session_id_t;
typedef uint16_t mask_t;
typedef uint16_t cbuf_len_t;
typedef int16_t cbuf_offset_t;
typedef uint8_t pattern_t;
typedef uint8_t component_id_t;

namespace VK {
	namespace Net {
		class Session;
		class CircularBuf;
		class Monitor;

		template<typename T, size_t Capacity = 5>
		class MemoryPool;

		template<typename T>
		class Singleton;
	}
}

typedef VK::Net::Session session_t;
typedef VK::Net::CircularBuf cbuf_t;
typedef std::shared_ptr<cbuf_t> cbuf_ptr_t;
typedef VK::Net::MemoryPool<cbuf_t> cbuf_pool_t;
typedef VK::Net::Monitor monitor_t;
typedef std::function<void(bool, session_t*)> send_cb_t;
typedef std::function<void(bool, session_t*)> open_cb_t;
typedef std::function<void(session_t*)> close_cb_t;
typedef std::function<void(session_t*, error_no_t, cbuf_ptr_t)> req_cb_t;
typedef std::function<void(session_t*, cbuf_ptr_t)> push_handler_t;
typedef std::function<void(error_no_t, cbuf_ptr_t)> resp_cb_t;
typedef std::function<void(session_t*, cbuf_ptr_t, resp_cb_t)> req_handler_t;

#pragma endregion 

#pragma region("enums")

enum Pattern : pattern_t {
	Push,
	Request,
	Response,
	Ping,
	Pong,
};

enum NetError : error_no_t {
	Success = 0,
	NE_Write,
	NE_Read,
	NE_SerialConflict,
	NE_NoHandler,
	NE_ReadErrorNo,
	NE_SessionClosed,
	NE_End,
};

#pragma endregion 

#pragma region structs


typedef struct {
	enum _ {
		SliceCount = 10
	};
	uv_write_t req;
	send_cb_t cb;
	session_t * session;
	cbuf_ptr_t pcb_array[SliceCount];
	uv_buf_t uv_buf_array[SliceCount];

	void clear() {
		//pcb_array.clear();
		for (auto i = 0; i < SliceCount; ++i) {
			pcb_array[i] = nullptr;
		}
		cb = nullptr;
		session = nullptr;
	}
} write_req_t;

typedef VK::Net::Singleton<VK::Net::MemoryPool<write_req_t, 1>> wr_pool_t;

typedef struct {
	uv_connect_t req;
	session_t* session;
	std::function<void(bool, session_t*)> cb;
} connect_req_t;

typedef struct {
	uv_shutdown_t req;
	session_t* session;
	std::function<void(session_t*)> cb;
} shutdown_req_t;

typedef struct {
	uv_getaddrinfo_t req;
	std::function<void(bool, sockaddr *)> cb;
} getaddr_req_t;

#pragma endregion 
