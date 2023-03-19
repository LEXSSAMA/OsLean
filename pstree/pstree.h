#ifndef __PSTREE_H__
#define __PSTREE_H__

#define PROC_DIR "/proc/"

typedef struct proc_node_s
{
    void* p_proc;
    struct proc_node_s* p_next;
} proc_node_t;

typedef struct proc_s
{
    int pid;
    char proc_name[32];
    struct proc_s* p_parent;
    struct proc_node_s* p_child;
} proc_t;

typedef struct proc_cache_s
{
    int pid;
    struct proc_s* p_proc;
    struct proc_cache_s* p_next;
} proc_cache_t;
#endif /* __PSTREE_H__ */
