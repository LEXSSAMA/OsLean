#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROC_DIR   "/proc/"
#define MAX_COLUMN 256
#define MAX_ROW    1024

static char tree_matrix[MAX_ROW][MAX_COLUMN];

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

static proc_cache_t* proc_cache_head = NULL;

static proc_t* proc_find(int pid)
{
    proc_t* p_ret = NULL;

    proc_cache_t* p_head = proc_cache_head;

    while (p_head)
    {
        if (p_head->pid == pid)
        {
            p_ret = p_head->p_proc;
            break;
        }
        p_head = p_head->p_next;
    }

    return p_ret;
}

static proc_t* proc_create(int pid)
{
    proc_t* p_ret = NULL;

    p_ret           = (proc_t*)malloc(sizeof(proc_t));
    p_ret->pid      = pid;
    p_ret->p_parent = NULL;
    p_ret->p_child  = NULL;

    char filename[32] = {0};
    snprintf(filename, 32, "/proc/%d/comm", p_ret->pid);

    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        return NULL;
    }
    fscanf(fp, "%s", p_ret->proc_name);
    fclose(fp);

    return p_ret;
}

static proc_cache_t* proc_cache_insert(proc_t* p_proc)
{
    proc_cache_t* p_ret = NULL;

    p_ret         = (proc_cache_t*)malloc(sizeof(proc_cache_t));
    p_ret->pid    = p_proc->pid;
    p_ret->p_proc = p_proc;
    p_ret->p_next = proc_cache_head;

    proc_cache_head = p_ret;

    return proc_cache_head;
}

static int proc_ppid_get(int pid)
{
    int ppid          = 0;
    char filename[32] = {0};
    FILE* fp          = NULL;

    snprintf(filename, 32, "/proc/%d/stat", pid);

    fp = fopen(filename, "r");
    fscanf(fp, "%*d %*s %*c %d %*s", &ppid);
    fclose(fp);

    return ppid;
}

static bool proc_child_insert(proc_t* p_pproc, proc_t* p_proc)
{
    if (!p_pproc || !p_proc)
    {
        // Log something
        return false;
    }

    if (!p_pproc->p_child)
    {
        p_pproc->p_child         = (proc_node_t*)malloc(sizeof(proc_node_t));
        p_pproc->p_child->p_next = NULL;
        p_pproc->p_child->p_proc = p_proc;
        return true;
    }

    proc_node_t* p_head = p_pproc->p_child;

    while (p_head)
    {
        if (!p_head->p_next)
        {
            p_head->p_next         = (proc_node_t*)malloc(sizeof(proc_node_t));
            p_head->p_next->p_next = NULL;
            p_head->p_next->p_proc = p_proc;
            break;
        }
        p_head = p_head->p_next;
    }

    return true;
}

static int make_tree(proc_node_t* p_node, int row, int col)
{
    int orgin_row       = row;
    int orgin_col       = col;
    char head_sign      = ' ';
    int len             = 0;
    proc_t* p_proc      = NULL;
    proc_node_t* p_head = p_node;

    while (p_head)
    {
        p_proc = (proc_t*)p_head->p_proc;

        if (!p_head->p_next)
        {
            head_sign = '`';
        }
        else
        {
            head_sign = '|';
        }

        if (p_proc->p_child)
        {
            len = snprintf(&tree_matrix[row][col], MAX_COLUMN, "%c--%s(%d)+\n", head_sign, p_proc->proc_name,
                           p_proc->pid);
        }
        else
        {
            len =
                snprintf(&tree_matrix[row][col], MAX_COLUMN, "%c--%s(%d)\n", head_sign, p_proc->proc_name, p_proc->pid);
        }

        row    = make_tree(p_proc->p_child, row + 1, col + len - 2);
        col    = orgin_col;
        p_head = p_head->p_next;

        if (p_head)
        {
            for (int i = orgin_row; i < row; ++i)
            {
                tree_matrix[i][orgin_col] = '|';
            }
        }
    }

    return row;
}

static void tree_print(int row)
{
    for (int i = 0; i < row; ++i)
    {
        printf("%s", tree_matrix[i]);
    }
    return;
}

int main()
{
    int len                  = 0;
    int row                  = 0;
    int col                  = 0;
    int pid                  = 0;
    int ppid                 = 0;
    proc_t* p_proc           = NULL;
    proc_t* p_pproc          = NULL;
    DIR* p_proc_dir          = NULL;
    struct dirent* p_dir_ent = NULL;

    if ((p_proc_dir = opendir(PROC_DIR)) == NULL)
    {
        perror("Error: ");
        return -1;
    }

    while ((p_dir_ent = readdir(p_proc_dir)) != NULL)
    {
        pid = 0;
        if ((p_dir_ent->d_type & DT_DIR) == 0 || (pid = atoi(p_dir_ent->d_name)) == 0)
        {
            continue;
        }

        p_proc = proc_find(pid);

        if (!p_proc)
        {
            p_proc = proc_create(pid);
            proc_cache_insert(p_proc);
        }

        ppid = proc_ppid_get(pid);

        p_pproc = proc_find(ppid);

        if (!p_pproc && ppid > 0)
        {
            p_pproc = proc_create(ppid);
            proc_cache_insert(p_pproc);
        }

        p_proc->p_parent = p_pproc;
        if (p_pproc)
        {
            proc_child_insert(p_pproc, p_proc);
        }
    }

    p_proc = proc_find(1);
    memset(tree_matrix, ' ', MAX_ROW * MAX_COLUMN);

    len = snprintf(&tree_matrix[row][col], MAX_COLUMN, "%s(%d)+\n", p_proc->proc_name, p_proc->pid);
    snprintf(&tree_matrix[row + 1][col], MAX_COLUMN - col, "%*c", len, ' ');
    row = make_tree(p_proc->p_child, row + 1, col + len - 2);
    tree_print(row);
    return 0;
}
