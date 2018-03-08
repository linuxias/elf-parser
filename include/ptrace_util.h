#ifndef __PTRACE_UTIL_H__
#define __PTRACE_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ptrace_info *ptrace_info_s;

struct ptrace_info {
	pid_t pid;
	void *addr;
	void *data;
} ptrace_info;

ptrace_info_s ptrace_util_init_info(pid_t pid, void *addr, void *data);

void ptrace_util_destroy_info(ptrace_info_s info);

void *ptrace_util_get_addr(ptrace_info_s info);
int ptrace_util_set_addr(ptrace_info_s info, void *addr);

void *ptrace_util_get_data(ptrace_info_s info);
int ptrace_util_set_data(ptrace_info_s info, void *data);

int ptrace_util_attach(ptrace_info_s info);

#ifdef __cplusplus
}
#endif
#endif /* __PTRACE_UTIL_H__ */
