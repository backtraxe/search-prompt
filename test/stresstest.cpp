#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <sys/select.h>
#include <unistd.h>

#include "../src/Client.hpp"

using namespace std;

struct RequestParam {
    vector<string> *query_list;
    int replay_num;
    int thread_no; // 线程的编号
};

struct StatInfo {
    int req_cnt;
    int rsp_succ_cnt;
    int rsp_fail_cnt;
    int timecost_ms;
};

struct ProcessArgs {
    int thread_num;
    uint32_t request_cnt;
    string query_filename;
    string setting_filename;
};

StatInfo g_stat_info;

pthread_mutex_t g_mutex_for_queue;
pthread_mutex_t g_mutex_for_stat;

// 统计时间间隔
const int kStatInterval = 3000; // 3s
static bool g_is_stopped = false;

const char *ip = "9.135.34.93";
int port = 6789;

bool doRequest(const char *ip, int port, const string &query) {
    Client client(ip, port);
    return client.doRequest(query);
}

/**
 * @brief 发起请求的线程
 *
 * @param arg 参数
 * @return void*
 */
void *StressThread(void *arg) {
    RequestParam *param = (RequestParam *)arg;
    // 随机种子
    unsigned int seed = clock();

    for (int i = 0; i < param->replay_num && !g_is_stopped; ++i) {
        // 随机选择 query
        int rand_num = rand_r(&seed) % param->query_list->size();
        string query = param->query_list->at(rand_num);

        // 开始时间
        auto start_time = chrono::steady_clock::now();
        // 发起请求
        bool ret = doRequest(ip, port, query);
        if (!ret) {
            printf("request failed: ret=%d query=%s\n", ret, query.c_str());
        }
        // 结束时间
        auto end_time = chrono::steady_clock::now();

        // 响应时间（毫秒）
        int cost_millis = (end_time - start_time).count() / 1000000;

        // 统计信息（加互斥锁）
        pthread_mutex_lock(&g_mutex_for_stat);
        g_stat_info.req_cnt++;
        ret ? g_stat_info.rsp_succ_cnt++ : g_stat_info.rsp_fail_cnt++;
        g_stat_info.timecost_ms += cost_millis;
        pthread_mutex_unlock(&g_mutex_for_stat);
    }
    return NULL;
}

/**
 * @brief 统计线程
 *
 * @param arg
 * @return void*
 */
void *StatThread(void *arg) {
    // 开始时间
    auto start_time = chrono::steady_clock::now();

    while (!g_is_stopped) {
        auto end_time = chrono::steady_clock::now();
        long timediff = (end_time - start_time).count() / 1000000L;

        // 时间间隔
        if (timediff >= kStatInterval) {
            start_time = end_time;

            // 计算 qps
            pthread_mutex_lock(&g_mutex_for_stat);
            double qps = g_stat_info.req_cnt * 1000.0 / (double)kStatInterval;
            double timecost = 0.0f;

            if (g_stat_info.req_cnt > 0) {
                timecost =
                    (double)g_stat_info.timecost_ms / g_stat_info.req_cnt;
            }

            printf("QPS: %-10d  AvgTimecost: %-10d Total/Succ/Fail: %d/%d/%d\n",
                   (int)qps, (int)timecost, g_stat_info.req_cnt,
                   g_stat_info.rsp_succ_cnt, g_stat_info.rsp_fail_cnt);

            memset(&g_stat_info, 0, sizeof(g_stat_info));
            pthread_mutex_unlock(&g_mutex_for_stat);
        } else {
            // 休眠 0.1s
            usleep(100000);
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    int ret = 0;

    // 参数解析
    ProcessArgs process_args;
    char ch;
    while ((ch = getopt(argc, argv, "t:c:q:")) != -1) {
        switch (ch) {
        case 't':
            process_args.thread_num = atoi(optarg);
            break;
        case 'c':
            process_args.request_cnt = strtoul(optarg, NULL, 10);
            break;
        case 'q':
            process_args.query_filename = optarg;
            break;
        default:
            break;
        }
    }

    // 读取参数
    vector<string> query_list;
    ifstream ifs(process_args.query_filename, ios::in);
    if (!ifs.is_open()) {
        cout << "file open error\n";
        return -1;
    }
    string line;
    while (getline(ifs, line)) {
        query_list.emplace_back(string());
        query_list.back().swap(line);
    }

    // 设置请求线程
    int thread_num = process_args.thread_num;
    vector<RequestParam> param_list(thread_num);

    for (size_t i = 0; i < thread_num; ++i) {
        RequestParam &param = param_list[i];
        param.query_list = &query_list;
        param.replay_num = process_args.request_cnt;
        param.thread_no = i;
    }

    // 初始化
    memset(&g_stat_info, 0, sizeof(g_stat_info));
    pthread_mutex_init(&g_mutex_for_stat, NULL);

    vector<pthread_t> thread_ids(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        ret =
            pthread_create(&thread_ids[i], NULL, StressThread, &param_list[i]);
        if (ret) {
            printf("create pthread error!/n");
            return -1;
        }
    }

    // 统计信息的线程
    pthread_t stat_thread_id;
    ret = pthread_create(&stat_thread_id, NULL, StatThread, NULL);
    if (ret) {
        printf("create pthread error!/n");
        return -1;
    }

    while (!g_is_stopped) {
        sleep(1);
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(thread_ids[i], NULL);
    }
    pthread_join(stat_thread_id, NULL);

    return 0;
}