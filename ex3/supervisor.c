/**
 * @file supervisor.c
 * @author Jakob G. Maier <e11809618@student.tuwien.ac.at>
 * @date
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
#include <limits.h>
#include "common.h"

static char *prog = "not set";
static int shmfd = -1;
static circ_buf_t *buf = MAP_FAILED;
static sem_t *free_sem = SEM_FAILED;
static sem_t *used_sem = SEM_FAILED;

/** Print solution
 * @brief Print array of solutions
 * @details
 * @param s Array of edges
 * @param size Size of edge_t array
 * @return void
 */
static void print_solution(edge_t *s, int size)
{
    printf("[%s] Solution with %d edges: ", prog, size);
    for (int i = 0; i < size; i++){
        printf("%d-%d ", s[i].u, s[i].v);
    }
    printf("\n");
}

/** Signal handler
 * @brief Exit from loop and quit and tell generators to exit as well
 * @details Uses the variable quit in the shared memory to 
 * communicate between supervisor and generators
 * @param s Signal s
 */
static void handle_signal(int s)
{
    buf->quit = 1;
}

/** Print usage
 * @brief
 * @details
 * @param void
 */
static void usage(void)
{
    fprintf(stderr, "USAGE: %s\n", prog);
    exit(EXIT_FAILURE);
}

/** Allocate resources
 * @brief Allocate resources for shared memory and semaphores.
 * @details
 * @param void
 */
static void allocate_resources(void)
{
    if ((shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600)) == -1)
        error_exit(prog, "shm_open failed");

    if (ftruncate(shmfd, sizeof(circ_buf_t)) < 0)
        error_exit(prog, "ftruncate failed");

    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (buf == MAP_FAILED) 
        error_exit(prog, "mmap failed");

    free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if (free_sem == SEM_FAILED) 
        error_exit(prog, "free_sem failed");

    used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    if (used_sem == SEM_FAILED) 
        error_exit(prog, "used_sem failed");
}

/** Free resources 
 * @brief
 * @details
 * @param void 
 */
static void free_resources(void)
{
    if (shmfd != -1){
        close(shmfd);
        if (shm_unlink(SHM_NAME) == -1) 
            error_exit(prog, "shm_unlink failed");
        shmfd = -1;
    }

    if (buf != MAP_FAILED){
        if (munmap(buf, sizeof(*buf)) == -1) 
            error_exit(prog, "munmap failed");
        buf = MAP_FAILED;
    }

    if (free_sem != SEM_FAILED){
        sem_close(free_sem);
        if (sem_unlink(FREE_SEM) == -1) 
            error_exit(prog, "sem_unlink failed");
        free_sem = SEM_FAILED;
    }

    if (used_sem != SEM_FAILED){
        sem_close(used_sem);
        if (sem_unlink(USED_SEM) == -1) 
            error_exit(prog, "sem_unlink failed");
        used_sem = SEM_FAILED;
    }
}

/**
 * @brief
 * @details
 * @param
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
    prog = argv[0];

    if (argc != 1)
        usage();

    if (atexit(free_resources) != 0)
        error_exit(prog, "resources not freed");
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    allocate_resources();

    buf->rp   = 0;
    buf->wp   = 0;
    buf->quit = 0;

    edge_t solution[MAX_SOLUTION_SIZE];
    if (solution == NULL) error_exit(prog, "malloc failed");

    int solution_size, min_solution = INT_MAX;
    
    while(!buf->quit){

        if (sem_wait(used_sem) == -1){
            if (errno == EINTR) 
                continue;
            error_exit(prog, "something happended");
        }
            
        solution_size = buf->size[buf->rp];

        if (solution_size < min_solution){
            if (solution_size == 0){
                printf("[%s] graph is acyclic!\n", prog);

            } else {
                memcpy(solution, buf->data[buf->rp], solution_size * sizeof(edge_t));
                print_solution(solution, solution_size);

            }
            min_solution = solution_size;
        } 
        
        sem_post(free_sem);
        buf->rp = (buf->rp + 1) % MAX_DATA;
    }
    return EXIT_SUCCESS;
}