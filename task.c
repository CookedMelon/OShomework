



#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h> // 确保包含了这个头文件
#define MAX_PROCESSES 10 // 最大进程数
#define LEVELS 4         // 队列级别
#define MAXTIME 99999    // 最大时间
// 基础时间片大小，可以根据需要调整
int T_list[LEVELS] = {1,2,4,8};
// 当前时间
int time = 0;

typedef struct Process{
    int pid;         // 进程标识符
    int level;       // 进程所在队列级别
    bool is_active;  // 进程是否处于活跃状态
    int time_left;   // 进程剩余执行时间
    int time_left_in_level; // 进程在当前队列中剩余执行时间
    int come_time;   // 进程到达时间
    int needed_time;  // 进程所需执行时间
}Process;

Process queues[LEVELS][MAX_PROCESSES]; // 多级队列
int front[LEVELS] = {0}, rear[LEVELS] = {0}; // 队列头尾指针

// 存储即将来的进程
Process comelist[MAX_PROCESSES];
// 目前来了几个进程
int comenum = 0;

// 初始化队列
void initialize_queues() {
    for (int i = 0; i < LEVELS; ++i) {
        for (int j = 0; j < MAX_PROCESSES; ++j) {
            queues[i][j].is_active = false;
        }
        front[i] = rear[i] = 0;
    }
}

// 判断队列是否满
bool is_full(int level) {
    return (rear[level] + 1) % MAX_PROCESSES == front[level];
}

// 将进程放入队列
void enqueue(int level, Process process) {
    if(is_full(level)) {
        printf("Queue %d is full!\n", level);
        enqueue(level + 1, process);
        return;
    }
    queues[level][rear[level]] = process;
    rear[level] = (rear[level] + 1) % MAX_PROCESSES;
}

// 判断队列是否空
bool is_empty(int level) {
    return front[level] == rear[level];
}
// 判断是否都为空
bool is_all_empty() {
    for (int i = 0; i < LEVELS; ++i) {
        if(!is_empty(i))
            return false;
    }
    return true;
}
// 输出红色字体
void color_printf(const char* color_code, const char* format, ...) {
    va_list args;
    printf("%s", color_code); // 设置颜色
    va_start(args, format);
    vprintf(format, args); // 打印格式化的文本
    va_end(args);
    printf("\033[0m"); // 重置到默认颜色
}
void red_printf(const char* format, ...) {
    va_list args;
    printf("\033[31m"); // 设置颜色
    va_start(args, format);
    vprintf(format, args); // 打印格式化的文本
    va_end(args);
    printf("\033[0m"); // 重置到默认颜色
}
void green_printf(const char* format, ...) {
    va_list args;
    printf("\033[32m"); // 设置颜色
    va_start(args, format);
    vprintf(format, args); // 打印格式化的文本
    va_end(args);
    printf("\033[0m"); // 重置到默认颜色
}
void yellow_printf(const char* format, ...) {
    va_list args;
    printf("\033[33m"); // 设置颜色
    va_start(args, format);
    vprintf(format, args); // 打印格式化的文本
    va_end(args);
    printf("\033[0m"); // 重置到默认颜色
}
// 从队列中取出进程
Process dequeue(int level) {
    if(is_empty(level)) {
        // printf("Queue %d is empty!\n", level);
        if(level < LEVELS - 1)
            return dequeue(level + 1);
        else{
            yellow_printf("all Queue is empty!\n");
            Process process = {0, 0, false, 0, 0, 0, 0};
            return process;
        }
    }
    Process process = queues[level][front[level]];
    // 弹出队列头
    front[level] = (front[level] + 1) % MAX_PROCESSES;
    return process;
}

// 打印队列中的进程
void print_queue(int level) {
    printf("Queue %d: ", level);
    for (int i = front[level]; i != rear[level]; i = (i + 1) % MAX_PROCESSES) {
        printf("%d ", queues[level][i].pid);
    }
    printf("\n");
}

// 打印全部队列
void print_queues() {
    for (int i = 0; i < LEVELS; ++i) {
        print_queue(i);
    }
}
void print_porcess(Process process){
    printf("pid:%d, level:%d, is_active:%d, time_left:%d, time_left_in_level:%d, come_time:%d, needed_time:%d\n",process.pid,process.level,process.is_active,process.time_left,process.time_left_in_level,process.come_time,process.needed_time);
}

// 调度
void schedule() {
    printf("------------------------------------------------------------------------------------------\n");
    yellow_printf("now time:%d\n",time);
    // 先获得最近一次进程即将到来的时间
    int next_come_time = comelist[comenum].come_time;
    if(comenum == MAX_PROCESSES){
        next_come_time = MAXTIME;
    }
    // 当没有更多任务且队列为空时，结束调度
    if(next_come_time == MAXTIME && is_all_empty()){
        red_printf("No process in queue and no process will come!!!\n");
        return;
    }
    // 当前时间与最近一次进程即将到来的时间只差
    int time_com = next_come_time - time;
    // 获得优先调度的任务
    Process process = dequeue(0);
    //即将执行的任务信息
    puts("next process info:");
    print_porcess(process);
    if(process.is_active == false) {
        yellow_printf("No process in queue!\n");
        time = next_come_time;
        if(comenum < MAX_PROCESSES){
            green_printf("Process %d come\n", comelist[comenum].pid);
            enqueue(0, comelist[comenum++]);
        }
        return;
    }
    // 如果进程未在执行状态，把他放进CPU
    if(!process.time_left_in_level){
        process.time_left_in_level = T_list[process.level];
    }
    // 获取time_left_in_level和time_left中的较小值
    int time_left = process.time_left_in_level < process.time_left ? process.time_left_in_level : process.time_left;
    yellow_printf("time_left:%d, next_come_time:%d\n",time_left,time_com);
    // 如果进程剩余执行时间小于时间片
    if(time_left <= time_com){
        // 将进程执行完
        time += time_left;
        process.time_left -= time_left;
        process.time_left_in_level -= time_left;
        green_printf("Process %d run %d time slices\n", process.pid, time_left);
        // 如果进程还未执行完
        if(process.time_left){
            // 将进程放入下一级队列
            if(process.level < LEVELS - 1)
                process.level++;
            enqueue(process.level, process);
            print_porcess(process);
        }
        // 如果进程完成
        else{
            process.is_active = false;
            green_printf("Process %d finished\n", process.pid);
            print_porcess(process);
        }
        // 完成这轮后正好新任务来了
        if(time_left == time_com){
            // 将新任务放入队列
            if(comenum < MAX_PROCESSES){
                green_printf("Process %d come\n", comelist[comenum].pid);
                enqueue(0, comelist[comenum++]);
            }
            return;
        }
        return;
        
    }
    // 如果进程进行到一半时新任务来了
    else if(time_left > time_com){
        // 将进程执行time_com时间
        time += time_com;
        process.time_left -= time_com;
        process.time_left_in_level -= time_com;
        green_printf("Process %d run %d time slices\n", process.pid, time_com);
        // 将进程放入下一级队列
        if(process.level < LEVELS - 1)
            process.level++;
        enqueue(process.level, process);
        // 将新任务放入队列
        if(comenum < MAX_PROCESSES){
            green_printf("Process %d come\n", comelist[comenum].pid);
            enqueue(0, comelist[comenum++]);
        }
        print_porcess(process);
        return;
    }

    // 我不想看到他执行的地方
    red_printf("[*] maybe error");


}

void init_process() {
    int pid_list[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int time_need[] = {8, 6, 17, 3, 4, 5, 9, 10, 11, 12};
    int come_time[] = {0, 5,  13, 20, 21, 25, 30, 32,40,50};
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        comelist[i].pid = pid_list[i];
        comelist[i].time_left = time_need[i];
        comelist[i].come_time = come_time[i];
        comelist[i].needed_time = time_need[i];
        comelist[i].is_active = true;
    }
}

int main() {
    init_process();
    initialize_queues();

    for(int i =0;i<100;i++){
        schedule();
        print_queues();
        // 暂停
        getchar();
    }

    return 0;
}
