/* ************************************************************************
 * File:   rtos.h
 *
 * (c) 2002-2007, ETAS GmbH
 *
 * RTOS abstraction for ETAS LABCAR-RTPC
 *
 * *********************************************************************** */

#ifndef RTOS_H
#define RTOS_H
#ifdef __cplusplus
extern "C" {
#endif

/* *********************************************************************** *
 *   I N C L U D E S
 * *********************************************************************** */
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>


/* *********************************************************************** *
 *   D E F I N E S 
 * *********************************************************************** */

#define THREAD_PRIO_HIGHEST     95
#define THREAD_PRIO_HIGH        90
#define THREAD_PRIO_STANDARD    50 
#define THREAD_PRIO_LOW         12
#define THREAD_PRIO_LOWEST      2


#define MICROSECONDS 1000LL
#define MILLISECONDS (1000LL * MICROSECONDS)
#define SECONDS (1000LL * MILLISECONDS)



/* *********************************************************************** *
 *   T Y P E S  
 * *********************************************************************** */

typedef void volatile rtos_thread_t;

typedef void rtos_mutex_t;

typedef void rtos_event_t;

typedef void rtos_sem_t;

typedef unsigned long long rtos_time_t;

struct rtos_iovec
{
    void *iov_base;
    int   iov_len;
};


typedef union
{
    volatile unsigned int all;
    struct {
        volatile char system;   // System Stop request (caused by a signal)
        volatile char user;     // User Stop request
        volatile char ext1;
        volatile char ext2;
    } t;
} rtos_stop_request_t;

typedef unsigned long rtos_handle_t;

typedef struct {
    char    name[32];
    int     countmax;
    int     count;
    rtos_time_t start;
    rtos_time_t accu;
    rtos_time_t min;
    rtos_time_t max;
    rtos_time_t sum;
} rtos_runtime_meas_t;


/* *********************************************************************** *
 *   G L O B A L S   
 * *********************************************************************** */
extern volatile rtos_stop_request_t rtos_stop_request;

extern char *rtos_rtpc_version;

extern int rtos_program_id;  


extern __thread rtos_time_t rtos_thread_dt_ns;
extern __thread rtos_time_t rtos_thread_t_ns;

#define rtos_thread_get_dt_ns() rtos_thread_dt_ns
#define rtos_thread_get_t_ns()  rtos_thread_t_ns

/* *********************************************************************** *
 *   P U B L I C   F U N C T I O N S    
 * *********************************************************************** */


/* ----------------------------------------------------------------------
 * Thread management
 * ---------------------------------------------------------------------- */

extern int rtos_thread_create(rtos_thread_t  **id, 
                       char *name,
                       void *(* start_routine)(rtos_thread_t *, void *),
                       void *arg,
                       int priority,
                       int attribute);

#define RTOS_THREAD_ATTR_JOINABLE       0x1
#define RTOS_THREAD_ATTR_LARGE_STACK    0x2
#define RTOS_THREAD_ATTR_SYSTEM_THREAD  0x4
#define RTOS_THREAD_ATTR_CPU(cpu)         (((cpu & 0x7f)+1) << 16)


typedef struct
{
    char   *name;
    void   *(* start_routine)(rtos_thread_t *, void *);
    void   *arg;
    int     priority;
    char    joinable;
    char    large_stack;
    char    system_thread;
    unsigned short  *cpulist;
    unsigned short   cpulist_el;
} rtos_thread_create_attr_t;


extern int rtos_thread_create_attr(rtos_thread_t  **id, 
                       rtos_thread_create_attr_t *attribute);

extern int rtos_getcpu(void);
extern int rtos_thread_join(rtos_thread_t *id, void **status);
extern int rtos_thread_is_running(rtos_thread_t *thread);

extern int rtos_thread_execute_in_background(
                        char *name,
                        void *(* start_routine)(rtos_thread_t *, void *),
                        void *arg);

extern int rtos_thread_execute_in_background_priority(
                        char *name,
                        void *(* start_routine)(rtos_thread_t *, void *),
                        void *arg,
                        int priority
                        );

extern rtos_thread_t * rtos_thread_self(void);

extern void rtos_thread_yield(void);

extern int rtos_thread_delete(rtos_thread_t *thread);

extern int rtos_thread_get_priority(rtos_thread_t *thread);
extern char *rtos_thread_get_name(rtos_thread_t *thread);

extern int rtos_thread_set_user_pointer(rtos_thread_t *thread, void *ptr);
extern void *rtos_thread_get_user_pointer(rtos_thread_t *thread);

extern int rtos_thread_set_user_id(rtos_thread_t *thread, int id);
extern int rtos_thread_get_user_id(rtos_thread_t *thread);

extern unsigned long rtos_thread_get_id(rtos_thread_t *thread);
extern unsigned long rtos_thread_self_id(void);

extern int rtos_thread_cpu_available(void);
extern int rtos_thread_cpu_default(void);

extern int rtos_thread_make_periodic_relative(rtos_thread_t *thread, 
        rtos_time_t delay, rtos_time_t period);

extern int rtos_thread_make_periodic_absolute(rtos_thread_t *thread,
                                 rtos_time_t start_time,
                                 rtos_time_t period);

extern int rtos_thread_stop_periodic(rtos_thread_t *thread);
extern void rtos_thread_wait_period(rtos_thread_t *thread);

extern int rtos_thread_kill(rtos_thread_t *thread, int signo);




#define RTOS_MAX_CPU_LOAD_INFO 32
extern int rtos_update_cpu_load(unsigned int maxcpus, double * cpu_load) __attribute__ ((deprecated));

/* ----------------------------------------------------------------------
 * Timer management
 * ---------------------------------------------------------------------- */
extern int rtos_thread_sleep(rtos_thread_t *thread, rtos_time_t delay);


extern rtos_time_t rtos_get_time_ns(void);
extern unsigned long rtos_get_time_64us_ticks(void);
extern double rtos_time_in_seconds(rtos_time_t ti);
extern void rtos_determine_cpu_frequency(void);
extern double rtos_thread_get_dt(void);



/* ----------------------------------------------------------------------
 * Mutexes 
 * ---------------------------------------------------------------------- */
extern int rtos_mutex_init(rtos_mutex_t **mutex, int attribute, char *name);
extern int rtos_mutex_destroy(rtos_mutex_t *mutex);
extern int rtos_mutex_lock(rtos_mutex_t *mutex);
extern int rtos_mutex_lock_timeout(rtos_mutex_t *mutex, rtos_time_t timeout) __attribute__ ((deprecated));
extern int rtos_mutex_unlock(rtos_mutex_t *mutex);


/* ---------------------------------------------------------------------
 * Eventfd
 * --------------------------------------------------------------------- */
int rtos_eventfd_create(void);
int rtos_eventfd_delete(int fd);
int rtos_eventfd_signal(int fd);
int rtos_eventfd_wait(int fd);
int rtos_eventfd_wait_timeout(int fd, rtos_time_t timeout);



/* ----------------------------------------------------------------------
 * Logging 
 * ---------------------------------------------------------------------- */


/* Please use the constants as defined in <sys/syslog.h> for defining
 * the log level:
 * LOG_EMERG    0
 * LOG_ALERT    1
 * LOG_CRIT     2
 * LOG_ERR      3
 * LOG_WARNING  4
 * LOG_NOTICE   5
 * LOG_INFO     6
 * LOG_DEBUG    7
 * */

/*
 * There are log functions with or without tagging.
 * If a tag string is specified, it will be printed before the string specified
 * by 'format'.
 * If 'RTOS_LOG_TAG' is defined as a string it will be used with rtos_log and rtos_logva
 * as (automatic) tag.
 * If 'RTOS_LOG_TAG' is defined but no tag desired, use rtos_log_ or rtos_logva_ instead
 * */
extern int rtos_log_tagged(const int priority, const char *tag, const char *format, ...) 
               __attribute__ ((format (printf, 3,4)));

extern int rtos_logva_tagged(const int priority, const char *tag, const char *format, va_list arg);

extern int rtos_log(const int priority, const char *format, ...) 
           __attribute__ ((format (printf, 2,3)));
extern int rtos_logva(const int priority, const char *format, va_list arg);

#define rtos_log_(prio, ...)                rtos_log_tagged(prio, NULL, __VA_ARGS__)
#define rtos_logva_(prio, format, arg)      rtos_logva_tagged(prio, NULL, format, arg)

#ifdef RTOS_LOG_TAG

    #define rtos_log(prio, ...)              rtos_log_tagged(prio, RTOS_LOG_TAG,  __VA_ARGS__)
    #define rtos_logva(prio, format, args)   rtos_logva_tagged(prio, RTOS_LOG_TAG, format, args)

#endif


extern int rtos_get_log_level(void);
/* rtos_set_log_level sets the log level to be used and returns the previous log level */
extern int rtos_set_log_level(int);
extern int rtos_log_flush(void);

extern void rtos_log_get_counter(unsigned int counter[8], int clear);

extern void rtos_rt_log(const char *str);
extern void rtos_rt_log_reset(void);



/* ----------------------------------------------------------------------
 * Messages / Triggers 
 * ---------------------------------------------------------------------- */
extern int rtos_trigger_continue(rtos_thread_t *thread);
extern int rtos_trigger_continue_store(rtos_thread_t *thread);
extern int rtos_wait_for_trigger(rtos_thread_t *thread);
extern int rtos_trigger_get_fd(rtos_thread_t *thread);


/* ----------------------------------------------------------------------
 * Periodic callbacks
 * ---------------------------------------------------------------------- */
extern int  rtos_periodic_callback_register( int (*callback)(int,int), int userdata, rtos_time_t t_first_callback);
extern int  rtos_periodic_callback_unregister( int );
extern int  rtos_periodic_callback_unregister_all( void );

extern rtos_time_t rtos_periodic_base_period(void);
extern int rtos_periodic_start(void);

/* ----------------------------------------------------------------------
 *  Blocking calls (for Event Tasks) 
 * ---------------------------------------------------------------------- */

extern void rtos_blocking_mark_call(void);
extern void rtos_blocking_register_trace(void (*func)(void));


/* ----------------------------------------------------------------------
 * COMMUNICATION 
 * ---------------------------------------------------------------------- */
extern int rtos_comm_open(char *address, ...); /* __attribute__ ((format (printf, 1,2))); */
extern int rtos_comm_is_open(int handle);
extern int rtos_comm_get_opened(int *start, char *address);
extern int rtos_comm_close(int handle);
extern int rtos_comm_close_all(void);
extern int rtos_comm_exists(char *address);

extern int rtos_comm_write(int handle, const void *buf, const size_t count);
extern int rtos_comm_write_offset(int handle, const int offset, const void *buf, const size_t count);
extern int rtos_comm_writev(int handle, const struct rtos_iovec *iovec, const size_t iovec_len);
extern int rtos_comm_write_string(int handle, const char *string);

extern int rtos_comm_read(int handle, void *buf, const size_t count, const rtos_time_t timeout);
extern int rtos_comm_read_offset(int handle, const int offset, void *buf, const size_t count, const rtos_time_t timeout);
extern int rtos_comm_read_string(int handle, char *string, const size_t count, const rtos_time_t timeout);


enum rtos_comm_ioctl_t { RTOS_COMM_IOCTL_SYNC_WR,
                         RTOS_COMM_IOCTL_SYNC_RD,
                         RTOS_COMM_IOCTL_FLUSH,
                         RTOS_COMM_IOCTL_ADDRESS,
                         RTOS_COMM_IOCTL_RESTART,
                         RTOS_COMM_IOCTL_STATUS,
                         RTOS_COMM_IOCTL_SET_TIMEOUT,
                         RTOS_COMM_IOCTL_ACCESS,
                         RTOS_COMM_IOCTL_SELECT,
                         RTOS_COMM_IOCTL_FD,
                         RTOS_COMM_IOCTL_INFO,
                         RTOS_COMM_IOCTL_STOP,
                         RTOS_COMM_IOCTL_START,
                         RTOS_COMM_IOCTL_PAUSE,
                         RTOS_COMM_IOCTL_CONTINUE,
                         RTOS_COMM_IOCTL_INTERFACE,
  
                         /* The following element must be the last in the list: */
                         RTOS_COMM_IOCTL_END
};

#define RTOS_COMM_IOCTL_DRIVER_SPECIFIC 512

extern int rtos_comm_ioctl(int handle, int request, void *argp);
extern int rtos_comm_ioctl_all(int request, void *argp);
extern int rtos_comm_info(int handle, char *info_str, size_t info_size);

extern int rtos_comm_get_instance(int handle);
extern char *rtos_comm_get_basename(int handle);
extern char *rtos_comm_get_options(int handle);


extern int rtos_comm_sendto(int handle, const void *buf, const size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
extern int rtos_comm_recvfrom(int handle, void *buf, const size_t len, const int flags, struct sockaddr *from, socklen_t *fromlen);

/* ----------------------------------------------------------------------- 
 * D y n a m i c   L i b r a r i e s
 * ----------------------------------------------------------------------- */
extern void *rtos_dynamic_lib_open(char *name, FILE *err);
extern void *rtos_dynamic_lib_symbol(void *handle, char *symbol, int options);
extern int rtos_dynamic_lib_close(void *handle);
extern int rtos_dynamic_lib_list(FILE *stream, int filter);
extern void *rtos_dynamic_lib_address(char *libname);

/* ----------------------------------------------------------------------- 
 * S y s t e m   A c c e s s 
 * ----------------------------------------------------------------------- */
extern int rtos_call_system(char *cmd, ...);
extern int rtos_call_function(int (*funcptr)(void *), void *argument);
extern int rtos_call_app(char *output, int output_len, char *argv[]);

/* ----------------------------------------------------------------------- 
 *  S y m b o l   A c c e s s 
 * ----------------------------------------------------------------------- */
extern void *rtos_symbol_address(char *symbol);
extern int rtos_caller_stack(char *str, size_t maxlen);

typedef union
{
    unsigned int u;
    struct {
        unsigned char float_type:1;
        unsigned char unsigned_type:1;
        unsigned char array_type:1;
        unsigned char raw_type:1;
        unsigned short bytesize;  /* Size of the base type. Array size if not set. */
    } 
    ty;
}
rtos_type_info_t;

extern void *rtos_label_address(char *label, char *requested_type);
extern void *rtos_label_address_info(char *label, rtos_type_info_t *p_type, int *p_id);
extern int rtos_label_name_of_address(void *address, char *name, size_t namelen);


/* ----------------------------------------------------------------------- 
 *  L i n u x    i n t e r a c t i o n 
 * ----------------------------------------------------------------------- */
extern int rtos_linux_call_app(char *output, int output_len, char *argv[]);

extern int rtos_register_crash_callback(unsigned int slot, void (*cbfunc)(FILE *fp));
/* ----------------------------------------------------------------------- 
 *  F l o a t i n g   P o i n t    
 * ----------------------------------------------------------------------- */
extern int rtos_enable_floating_point_exception(void);
extern int rtos_disable_floating_point_exception(void);

/* ----------------------------------------------------------------------- 
 * I / O
 * ----------------------------------------------------------------------- */

extern int rtos_io_open(char *address);
extern int rtos_io_close(int handle);
extern int rtos_io_close_all(void);
extern int rtos_io_info(int handle, char *info_str, size_t info_size);
extern int rtos_io_out(int handle, double val);
extern int rtos_io_digout(int handle, unsigned char val);
extern int rtos_io_in(int handle, double *val);
extern int rtos_io_digin(int handle, unsigned char *val);

/* ----------------------------------------------------------------------- 
 * Watchdog
 * ----------------------------------------------------------------------- */
extern int rtos_watchdog_start(rtos_time_t low_prio_cycle, rtos_time_t high_prio_cycle, 
                        int counter_threshold, void (*emergency_callback)(int));


/* ----------------------------------------------------------------------- 
 * Exception handlers
 * ----------------------------------------------------------------------- */
void *rtos_exception_segv_handler(void (*new_handler)(void));
void *rtos_exception_fpe_handler(void (*new_handler)(void));

/* ----------------------------------------------------------------------- 
 * CPU specific stuff
 * ----------------------------------------------------------------------- */

/**
 * Clear interrupt enable flag and return the previous state of the flag.
 * Multi CPU safe.
 * For better performance use the rtos_critical_section_xxx functions.
 * */
extern void rtos_suspend_interrupts( void );

/**
 * Set interrupt flag.
 * Multi CPU safe.
 * */
extern void rtos_resume_interrupts(void);

extern void rtos_atomic_memcpy_8byte(void *to, const void *from);
extern void rtos_atomic_memcpy_16byte(void *to, const void *from);
extern void rtos_atomic_memcpy_16byte_aligned(void *to, const void *from);

/**
 * Critical section handling - Recommended into multi core environments
 * as it performs better than rtos_suspend/resume_interrupts */
typedef pthread_mutex_t rtos_critical_section_t;
extern void rtos_critical_section_init(rtos_critical_section_t *psect);
extern void rtos_critical_section_destroy(rtos_critical_section_t *psect);
extern void rtos_critical_section_enter(rtos_critical_section_t *psect);
extern void rtos_critical_section_leave(rtos_critical_section_t *psect);


extern unsigned long long rtos_get_cpu_cycles(void);

extern rtos_time_t rtos_convert_cpu_cycles_to_ns(unsigned long long delta);
extern unsigned long long rtos_convert_ns_to_cpu_cycles(rtos_time_t delta);

/* ----------------------------------------------------------------------- 
 * Swapping routines
 * ----------------------------------------------------------------------- */
extern unsigned long long rtos_swap64(unsigned long long val);
extern unsigned int rtos_swap32(unsigned int val);
extern unsigned short rtos_swap16(unsigned short val);

/* ----------------------------------------------------------------------- 
 * Misc stuff
 * ----------------------------------------------------------------------- */

extern int rtos_lic_check_feature_name_version(const char *feature_name, const char *feature_version, unsigned long long *data);
extern int rtos_encrypt(char *key, char *data, unsigned datalen);
extern int rtos_decrypt(char *key, char *data, unsigned datalen);

/* ----------------------------------------------------------------------- 
 * Time measurement routines
 * ----------------------------------------------------------------------- */
extern void rtos_runtime_meas_init(rtos_runtime_meas_t *p_tm, const char *name, const int updaterate);
extern void rtos_runtime_meas_start(rtos_runtime_meas_t *p_tm);
extern void rtos_runtime_meas_stop(rtos_runtime_meas_t *p_tm);
extern void rtos_runtime_meas_pause(rtos_runtime_meas_t *p_tm);
extern void rtos_runtime_meas_print(rtos_runtime_meas_t *p_tm);


/* ----------------------------------------------------------------------- 
 * Tracing
 * ----------------------------------------------------------------------- */
extern void rtos_trace_interval_begin(int id, int arg);
extern void rtos_trace_interval_end(int id);
extern void rtos_trace_point(int id, int arg);


extern void rtos_trace_trigger(void);
extern void rtos_trace_stop(void);
extern void rtos_trace_getstate(unsigned char *p_state, unsigned char *p_pct, unsigned char *p_fileid);

extern char rtos_trace_active;

/* ----------------------------------------------------------------------- 
 * Dummy functions
 * ----------------------------------------------------------------------- */
extern int rtos_dummy_int(void);
extern int rtos_dummy_int_negative(void);
extern void rtos_dummy_void(void);

/* ----------------------------------------------------------------------- 
 * CRC-32
 * ----------------------------------------------------------------------- */
extern void rtos_crc32_init(unsigned long *crc);
extern void rtos_crc32_update(unsigned long *crc, void *data, int len);
extern void rtos_crc32_result(unsigned long *crc);

/* ----------------------------------------------------------------------- 
 * Optimization macros to help the compiler to generate optimized code
 * in "if" conditions
 * ----------------------------------------------------------------------- */
#ifdef LIKELY
#undef LIKELY
#endif
#define LIKELY(cond) __builtin_expect((!!(cond)), 1)

#ifdef UNLIKELY
#undef UNLIKELY
#endif
#define UNLIKELY(cond) __builtin_expect((!!(cond)), 0)


/* ----------------------------------------------------------------------- 
 * Debug stuff
 * ----------------------------------------------------------------------- */
#if defined(__x86_64__) || defined(__i386__)
#include <sys/io.h>
#define SET_LPT_BIT(bit) (outb(inb(0x378) | (1 << (bit)), 0x378))
#define CLR_LPT_BIT(bit) (outb(inb(0x378) & 0xff & ~(1 << (bit)), 0x378))
#define SET_LPT_BYTE(b) (outb(b, 0x378))

// Write out a value to the POST port 0x80 that can be seen on some mainboards
#define rtos_post_code(x) (outb(x,0x80))
#else

#define SET_LPT_BIT(bit)
#define CLR_LPT_BIT(bit)
#define SET_LPT_BYTE(b) 

#endif


extern int rtos_kernel_tracing_enable(int);
extern int rtos_kernel_tracing_marker(const char *);


/* ----------------------------------------------------------------------- 
 * NOTE:
 *
 * The following functions are highly experimental.
 * Changes to the interface are possible without further notice.
 *
 * ----------------------------------------------------------------------- */


extern int rtos_checked_memcpy(void *dst, void *src, size_t size);
extern int rtos_checked_indirect_memrd(void *dest, void *baseaddr, int offsetno, int *offset, int size);
extern int rtos_checked_indirect_memwr(void *baseaddr, int offsetno, int *offset, void *src, int size);


extern void *rtos_global_var_address(char *s);

/* ----------------------------------------------------------------------- 
 * Variable Snapshots
 * ----------------------------------------------------------------------- */
extern int rtos_variable_snapshot(char *name);

/* ----------------------------------------------------------------------- 
 * Datalogger
 * ----------------------------------------------------------------------- */
typedef void rtos_datalog_t;
extern int rtos_datalog_init(rtos_datalog_t **dl, 
                                    char *options,
                                    void *data,
                                    size_t data_size,
                                    struct rtos_iovec *list,
                                    int (*enable_func)(void*),
                                    void *enable_data);

extern int rtos_datalog_close(rtos_datalog_t *dl);
extern int rtos_datalog_sample(rtos_datalog_t *dl);
extern int rtos_datalog_flush(rtos_datalog_t *dl);
extern int rtos_datalog_info(rtos_datalog_t *dl, FILE *fp);


/* ----------------------------------------------------------------------- 
 * Configuration value access
 * ----------------------------------------------------------------------- */
int rtos_conf_get_value_int(const char *key, int default_value);
int rtos_conf_get_value(const char *key, char *val, const size_t size);

int rtos_conf_get_eth_device(const char *indev, char *outdev, int outsize);

#ifdef __cplusplus
}
#endif
#endif
