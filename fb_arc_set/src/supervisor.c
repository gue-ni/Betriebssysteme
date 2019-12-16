/**
 * @file supervisor.c
 * 
 * 
 * 
 */ 
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>  
#include <fcntl.h> 
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "common.h"

static int shmfd = -1;
sem_t *free_sem, *used_sem;
volatile sig_atomic_t quit = 0;
struct circ_buf *buf;
char *prog;

/**
 * @brief
 * @details
 * @param
 * @return
 */
void print_solution(struct edge *solution, int solsize)
{
    printf("[%s] solution with %d edges: ", prog, solsize);
    for (int i = 0; i < solsize; i++){
        printf("%d-%d ", solution[i].u, solution[i].v);
    }
    printf("\n");
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
void write_message(char *msg)
{
    printf("[%s] %s\n", prog, msg);
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
void exit_error(char *msg)
{
    fprintf(stderr, "%s %s\n", prog, msg);
    exit(EXIT_FAILURE);
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
void handle_signal(int s)
{
    //write(0, "handle signal\n", 14);
    quit = 1;
    buf->quit = 1;
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
void allocate_resources(void)
{
    if ((shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600)) == -1)
        exit_error("shm_open failed");

    if (ftruncate(shmfd, sizeof(struct circ_buf)) < 0)
        exit_error("ftruncate failed");

    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (buf == MAP_FAILED) exit_error("mmap failed");

    free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, MAX_DATA);
    used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    
    free_sem = sem_open(FREE_SEM, 0);
    if (free_sem == SEM_FAILED)
        exit_error("free_sem failed");
  
    used_sem = sem_open(USED_SEM, 0);
    if (used_sem == SEM_FAILED)
        exit_error("used_sem failed");
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
void free_resources(void)
{
    if (shmfd != -1){
        write_message("free resources\n");
        if (munmap(buf, sizeof(*buf)) == -1) exit_error("munmap failed");
        if (close(shmfd) == -1) exit_error("close failed");
        if (shm_unlink(SHM_NAME) == -1) exit_error("shm_unlink failed");
        if (sem_close(free_sem) == -1) exit_error("sem_close failed");
        if (sem_close(used_sem) == -1) exit_error("sem_close failed");
        if (sem_unlink(FREE_SEM) == -1) exit_error("sem_unlink failed");
        if (sem_unlink(USED_SEM) == -1) exit_error("sem_uknlink failed");
    }
}

/**
 * @brief
 * @details
 * @param
 * @return
 */
int main(int argc, char *argv[])
{
    prog = argv[0];
    if (atexit(free_resources) != 0)
        exit_error("resources not freed");
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    allocate_resources();

    buf->read_pos = 0;
    buf->write_pos = 0;
    buf->quit = 0;
    struct edge *solution = malloc(sizeof(struct edge) * MAX_SOLUTION_SIZE);

    int solsize, min_solution = 10000000;
    while(!quit){

        if (sem_wait(used_sem) == -1){
            if (errno == EINTR){
                continue;
            }
            exit_error("something happended");
        }
            
        solsize = buf->solution_size[buf->read_pos];

        if (solsize < min_solution){
            if (solsize == 0){
                printf("[%s] graph is acyclic!\n", prog);
            } else {
                memcpy(solution, buf->data[buf->read_pos], solsize*sizeof(struct edge));
                print_solution(solution, solsize);
            }
            min_solution = solsize;
        } 
        
        sem_post(free_sem);
        buf->read_pos = (buf->read_pos + 1) % MAX_DATA;
    }
    return EXIT_SUCCESS;
}