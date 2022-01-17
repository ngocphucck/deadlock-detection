#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "config.c"


int num_threads;
int num_resources;
int* available;
int** maximum;
int** allocation;
int** need;
int* thread_order;


bool need_lt_work(int need_i[], int work[]) 
{
    for (int i = 0; i < num_resources; i++)
    {
        if (need_i[i] > work[i]) 
        {
            return false;
        }
    }
    return true;
}

bool is_safe_state() 
{
    int work[num_resources];
    for (int i = 0; i < num_resources; i++)
    {
        work[i] = available[i];
    }

    bool* finish = (bool*) malloc(num_threads * sizeof(bool));
    int finish_count = 0;
    bool changed;

    while (finish_count != num_threads) 
    {
        changed = false;
        for (int i = 0; i < num_threads; i++)
        {
            if (!finish[i] && need_lt_work(need[i], work)) 
            {
                for (int j = 0; j < num_resources; j++)
                {
                    work[j] += allocation[i][j];
                }
                
                finish[i] = true;
                thread_order[finish_count] = i;
                finish_count++;
                changed = true;
            }
        }
        if (!changed) {
            return false;
        }
    }

    return true;
}

int request_resources(int thread_num, int request[]) {
    for (int i = 0; i < num_resources; i++)
    {
        if (request[i] > need[thread_num][i] || request[i] > available[i]) {
            return -1;
        }
    }

    for (int i = 0; i < num_resources; i++)
    {
        available[i] -= request[i];
        allocation[thread_num][i] += request[i];
        need[thread_num][i] -= request[i];
    }

    //Neu thoa man trang thai an toan thi ok
    if (is_safe_state()) {
        printf(FGR_GREEN "*" RESET);
        printf("Thread %d Requests [%s\b] -> %s\n", thread_num + 1, req_to_str(request), BGR_GREEN "accepted" RESET);
        printf("Safe state order: ");
        for (int i = 0; i < num_threads; ++i)
        {
            printf("T%d ", thread_order[i] + 1);
            if (i == num_threads - 1)
                printf("\n");
        }
        print_available();

        return 0;
    }

    // Neu khong thoa man trang thai an toan thi thu hoi tai nguyen
    for (int i = 0; i < num_resources; i++)
    {
        available[i] += request[i];
        allocation[thread_num][i] -= request[i];
        need[thread_num][i] += request[i];
    }

    return -1;
}

int release_resources(int thread_num, int request[]) {

    for (int i = 0; i < num_resources; i++)
    {
        available[i] += request[i];
        allocation[thread_num][i] -= request[i];
        need[thread_num][i] += request[i];
    }

    printf("%sThread %d Releases [%s\b]\n", FGR_GREEN "*" RESET, thread_num + 1, req_to_str(request));
    print_available();
}

char *req_to_str(int req[]) {
    char *ret = malloc(100);
    char buf[5] = {0};
    for (int i = 0; i < num_resources; i++)
    {
        sprintf(buf, "%d", req[i]);
        strcat(ret, buf);
        strcat(ret, " ");
    }
    return ret;
}

void print_state() {
    printf("================");
    printf(FGR_BLUE "State" RESET);
    printf("=================\n");
    printf(FGR_YELLOW "Available:\n" RESET);
    for (int i = 0; i < num_resources; i++)
    {
        printf("%d ", available[i]);
    }
    printf("\n");

    printf(FGR_YELLOW "Maximum:\n" RESET);
    for (int i = 0; i < num_threads; i++)
    {
        for (int j = 0; j < num_resources; j++)
        {
            printf("%d ", maximum[i][j]);
        }
        printf("\n");
    }
    printf(FGR_YELLOW "Allocation:\n" RESET);
    for (int i = 0; i < num_threads; i++)
    {
        for (int j = 0; j < num_resources; j++)
        {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }
    printf(FGR_YELLOW "Need:\n" RESET);
    for (int i = 0; i < num_threads; i++)
    {
        for (int j = 0; j < num_resources; j++)
        {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
    printf("=============================================\n");
}

void print_available() {
    printf(FGR_YELLOW "Available:\n" RESET);
    for (int i = 0; i < num_resources; i++)
    {
        printf("%d ", available[i]);
    }
    printf("\n");

    printf(FGR_YELLOW "Allocation:\n" RESET);
    for (int i = 0; i < num_threads; i++)
    {
        for (int j = 0; j < num_resources; j++)
        {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }
    printf(FGR_YELLOW "Need:\n" RESET);
    for (int i = 0; i < num_threads; i++)
    {
        for (int j = 0; j < num_resources; j++)
        {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
}

void input_resource()
{
    deallocate_resource();
    printf("Enter number of threads: ");
    scanf("%d", &num_threads);
    printf("Enter number of resource type: ");
    scanf("%d", &num_resources);
    allocation_resource();
}

void allocation_resource()
{
    // Memory allocation
    available = (int*) malloc(num_resources * sizeof(int));
    maximum = (int**) malloc(num_threads * sizeof(int*));
    for(int i = 0; i < num_threads; ++i)
        maximum[i] = (int*) malloc(num_resources * sizeof(int));

    allocation = (int**) malloc(num_threads * sizeof(int*));
    for(int i = 0; i < num_threads; ++i)
        allocation[i] = (int*) malloc(num_resources * sizeof(int));

    need = (int**) malloc(num_threads * sizeof(int*));
    for(int i = 0; i < num_threads; ++i)
        need[i] = (int*) malloc(num_resources * sizeof(int));

    thread_order = (int*) malloc(num_threads * sizeof(int));

    printf("%s", "Available: \n");
    for (int i = 0; i < num_resources; ++i)
        scanf("%d", &available[i]);
    printf("%s", "Maximum: \n");
    for(int i = 0; i < num_threads; ++i)
        for(int j = 0; j < num_resources; ++j)
        {
            scanf("%d", &maximum[i][j]);
        }

    printf("%s", "Allocation: \n");
    for(int i = 0; i < num_threads; ++i)
        for(int j = 0; j < num_resources; ++j)
        {
            scanf("%d", &allocation[i][j]);
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
}

void deallocate_resource()
{
    if (num_resources == 0)
        return;

    free(available);
    free(thread_order);
    for(int i = 0; i < num_threads; ++i)
    {
        free(maximum[i]);
        free(allocation[i]);
        free(need[i]);
    }
    free(maximum);
    free(allocation);
    free(need);
}

void add_resource()
{
    printf("%s", "Enter additional resources for each type: ");
    int num;
    for(int i = 0; i < num_resources; ++i)
    {
        scanf("%d", &num);
        available[i] += num;
    }
    
    printf("%s", "complete!");
}


void add_thread()
{
    num_threads++;

    printf("%s", "Enter alloction resources for a new thread: ");
    allocation = (int**) realloc(allocation, num_threads * sizeof(int*));
    allocation[num_threads - 1] = (int*) malloc(num_resources * sizeof(int));
    for(int i = 0; i < num_resources; ++i)
        scanf("%d", &allocation[num_threads - 1][i]);

    printf("%s", "Enter maximum resources for a new thread request: ");
    maximum = (int**) realloc(maximum, num_threads * sizeof(int*));
    maximum[num_threads - 1] = (int*) malloc(num_resources * sizeof(int));
    for(int i = 0; i < num_resources; ++i)
        scanf("%d", &maximum[num_threads - 1][i]);

    need = (int**) realloc(need, num_threads * sizeof(int*));
    need[num_threads - 1] = (int*) malloc(num_resources * sizeof(int)); 
    for(int i = 0; i < num_resources; ++i)
        need[num_threads - 1][i] = allocation[num_threads - 1][i] - maximum[num_threads - 1][i];

    printf("%s", "Complete!");
}


void menu()
{
    while (true)
    {
        printf("%s\n", "=====================Select option=======================");
        printf("%s\n", "1. Input resourse");
        printf("%s\n", "2. Request");
        printf("%s\n", "3. Release");
        printf("%s\n", "4. Add resource");
        printf("%s\n", "5. Add thread");
        printf("%s\n", "6. Print state");
        printf("%s\n", "7. Exit");
        printf("%s\n", "=========================================================");

        char choice;
        printf("You choose: ");
        scanf("%c", &choice);

        system("clear");
        if (choice == '1')
        {
            input_resource();
            printf("%s", "complete!");
        }
        else if (choice == '2')
        {
            int id;
            int* req = (int*) malloc(num_resources * sizeof(int));

            printf("%s ", "Enter the thread id: ");
            scanf("%d", &id);
            printf("%s ", "Enter the request: ");
            for(int i = 0; i < num_resources; ++i)
                scanf("%d", &req[i]);

            int done = request_resources(id, req);
            if (done != 0)
            {
                printf("%sThread %d Requests [%s\b] -> %s\n", FGR_GREEN "*" RESET, id + 1, req_to_str(req), BGR_RED "not accepted" RESET);
            }
            else
                printf(" ");
        }

        else if (choice == '3')
        {
            int id;
            int* req = (int*) malloc(num_resources * sizeof(int));

            printf("%s ", "Enter the thread id: ");
            scanf("%d", &id);
            printf("%s ", "Enter the release: ");
            for(int i = 0; i < num_resources; ++i)
                scanf("%d", &req[i]);

            int done = release_resources(id, req);
        }
        else if (choice == '4')
        {
            add_resource();
        }
        else if (choice == '5')
        {
            add_thread();
        }
        else if (choice == '6')
        {
            print_state();
        }
        else if (choice == '7')
        {
            printf("%s", "Exiting !\n");
            break;
        }
        else 
        {
            printf("%s", "Invalid choice!");
        }
        getchar();
        getchar();
        system("clear");
    }

}
