
bool need_lt_work(int need_i[], int work[]);
bool is_safe_state();
int request_resources(int thread_num, int request[]);
int release_resources(int thread_num, int request[]);
void *thread(int n);
void print_state();
void print_available();
char *req_to_str(int req[]);
void menu();