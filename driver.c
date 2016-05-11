#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

class semaphore {
    public:
        semaphore(size_t init_count=0);

        void wait();
        void notify();

    private:
        size_t count_;
        std::mutex mutex_;
        std::condition_variable cv_;
};

semaphore::semaphore(size_t init_count) :
    count_(init_count) {
}

void semaphore::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] () { return count_ > 0; });
    --count_;
}

void semaphore::notify() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
}

void driver_thread(semaphore& movable, semaphore& openable) {
    while (true) {
        movable.wait();
        std::cout << "started" << std::endl;
        std::cout << "driving" << std::endl;
        std::this_thread::sleep_for(1s);
        std::cout << "stopped" << std::endl;
        openable.notify();
    }
}

void conductor_thread(semaphore& movable, semaphore& openable) {
    while (true) {
        openable.wait();
        std::cout << "           opened" << std::endl;
        std::cout << "           conducting" << std::endl;
        std::this_thread::sleep_for(1s);
        std::cout << "           closed" << std::endl;
        movable.notify();
    }
}

int main() {
    semaphore movable(1);
    semaphore openable(0);

    std::cout << "Driver    Conductor" << std::endl;

    std::thread driver(driver_thread, std::ref(movable), std::ref(openable));
    std::thread conductor(conductor_thread, std::ref(movable), std::ref(openable));

    driver.join();
    conductor.join();

    return 0;
}