#include <iostream>
#include <vector>
#include <thread>
#include <numeric>
#include <future>

using namespace std::chrono;

// used for threads
void sum_in_range(long long &sum, int start, int end) {
    for(int i = start; i < end; i++)
        sum += i;
}
// used for async tasks
long long get_sum_in_range(int start, int end) {
    long long sum = 0;
    for(int i = start; i < end; i++)
        sum += i;
    return sum;
}
long long single_thread(int n) {
    long long sum = 0;
    sum_in_range(sum, 0, n);
    return sum;
}

long long two_threads(int n) {
    int num_threads = 2;
    int step = n / num_threads;
    long long sum1 = 0, sum2 = 0;
    std::thread t1(sum_in_range, std::ref(sum1), 0, step);
    std::thread t2(sum_in_range, std::ref(sum2), step, n);

    t1.join();
    t2.join();
    long long sum = sum1 + sum2;
    return sum;
}

long long x_threads(int n, int num_of_threads) {
    int step = n / num_of_threads;
    std::vector<long long> partial_sums(num_of_threads, 0);
    std::vector<std::thread> threads;
    for(int i = 0; i < num_of_threads - 1; i++) {
        threads.push_back(std::thread(sum_in_range, std::ref(partial_sums[i]), i * step, (i + 1) * step));
    }
    // last thread should calculate sum up until n. (i+1)*step won't always return n, when i+1 = num_of_threads
    threads.push_back(std::thread(sum_in_range, std::ref(partial_sums[num_of_threads-1]), (num_of_threads-1) * step, n));
    
    for(std::thread &t: threads)
        if(t.joinable())
            t.join();
    long long sum = std::accumulate(partial_sums.begin(), partial_sums.end(), (long long) 0);
    return sum;
}

long long tasks_sum(int n, int num_of_tasks) {
    std::vector<std::future<long long>> tasks;
    int step = n / num_of_tasks;
    for(int i = 0; i < num_of_tasks - 1; i++) {
        tasks.push_back(std::async(get_sum_in_range, i * step, (i + 1) * step));
    }
    tasks.push_back(std::async(get_sum_in_range, (num_of_tasks-1) * step, n));

    long long sum = 0;
    for(auto &t: tasks)
        sum += t.get();
    return sum;
}
int main() {
    std::cout << "~~~~~THREADS~~~~~" << std::endl;
    int n = 1000001;
    auto start = high_resolution_clock::now();
    long long sum_single_thread = single_thread(n);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start).count();
    std::cout << "1 thread: time = " << duration << ", sum = " << sum_single_thread << std::endl;
    start = high_resolution_clock::now(); 
    long long sum_two_threads = two_threads(n);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start).count();
    std::cout << "2 threads: time = " << duration << ", sum = " << sum_two_threads << std::endl;
    
    for(int i = 3; i < 8; i++) {
        start = high_resolution_clock::now(); 
        long long curr_sum = x_threads(n, i);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start).count();
        std::cout << i << " threads: time = " << duration << ", sum = " << curr_sum << std::endl;
    }

    std::cout << "~~~~~TASKS~~~~~" << std::endl;
    for(int i = 1; i < 8; i++) {
        start = high_resolution_clock::now(); 
        long long curr_sum = tasks_sum(n, i);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start).count();
        std::cout << i << " threads: time = " << duration << ", sum = " << curr_sum << std::endl;
    }
}