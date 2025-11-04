#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <sched.h>     // For CPU affinity functions (Linux specific)
#include <pthread.h>   // For getting the native handle (Linux specific)
#include <string>
#include <mutex>         // For synchronizing output

// Global flag to control detailed debugging output
bool debug_mode = false; 

// Mutex specifically for synchronizing print operations
std::mutex print_mutex;

/**
 * @brief Synchronized print function protected by a mutex.
 * @param message The string to print.
 */
void sync_print(const std::string& message) {
    // Lock the mutex before printing, and unlock automatically when exiting scope
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << message << std::endl;
}

// ----------------------------------------------------
// Core worker function: Performs high-intensity C++ FPU stress.
// (All output removed from here to ensure ordered printing from main thread)
// ----------------------------------------------------
void stress_worker(int core_id) {
    // Define the constant values for the stress arithmetic
    const double a = 0.499999999999; 
    const double b = 0.999999999999;
    const double epsilon = 0.000000000001; 
    
    // Infinite loop to keep the core busy
    while (true) {
        
        // --- High-Intensity C++ Calculation Block ---
        double sum1 = a;
        double sum2 = b;
        
        // Use a large fixed iteration count to ensure deep computational load
        for (int i = 0; i < 10000000; ++i) { 
             // sum1 = sum1 * a + b
             sum1 = sum1 * a + b;
             // sum2 = sum2 * b + a
             sum2 = sum2 * b + a;
             
             // Check the exit condition less frequently to maximize arithmetic throughput
             if (i % 1000000 == 0 && std::abs(sum1 - sum2) < epsilon) {
                 break;
             }
        }
    }
}

// ----------------------------------------------------
// Main function: Handles arguments, creates threads and sets affinity
// ----------------------------------------------------
int main(int argc, char* argv[]) {
    // 1. Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug") {
            debug_mode = true;
            break;
        }
    }

    // 2. Get the number of CPU cores
    unsigned int num_cores = std::thread::hardware_concurrency();
    if (num_cores == 0) {
        num_cores = 4; // Fallback default value
    }

    if (debug_mode) {
        // Use sync_print for main program messages
        sync_print("Entering debug mode.");
        sync_print("Detected " + std::to_string(num_cores) + " logical cores.");
        sync_print("Starting threads and setting affinity...");
    }
    
    std::vector<std::thread> threads;
    
    // 3. Create threads and set affinity
    // Printing order is guaranteed because success messages are printed within this single-threaded loop.
    for (unsigned int i = 0; i < num_cores; ++i) {
        // A. Create thread (Thread starts running immediately)
        threads.emplace_back(stress_worker, i); 
        
        // B. Set affinity (Linux specific)
        pthread_t native_handle = threads.back().native_handle();
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset); 
        
        int result = pthread_setaffinity_np(native_handle, sizeof(cpu_set_t), &cpuset);

        // C. Print result in main thread
        if (result != 0) {
            std::string msg = "Warning: Could not bind thread to core " + std::to_string(i) + ". Error code: " + std::to_string(result);
            sync_print(msg); // Warnings are always synchronized
        } else {
            if (debug_mode) {
                // Success messages are printed sequentially by the main thread
                std::cout << "Successfully bound Thread-" << i << " to Core-" << i << std::endl;
            }
        }
    }
    
    // 4. Print the final runtime message (synchronized)
    sync_print("\nAll threads started and bound. The program will run continuously until manually terminated (Ctrl+C).");
    
    // 5. Wait for all threads to finish (i.e., wait for user Ctrl+C)
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    return 0;
}
