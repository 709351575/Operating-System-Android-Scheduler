# Operating-System-Android-Scheduler

A process scheduler under Weighted Round Robin policy in Anroid Linux Kernel.

## Important Files
- `Test File/jni/test.c`: the test program

- `sched/`: contains all modified files in the android kernel
    - `wrr.c`: the main scheduling program following Weighted Round Robin policy

- `goldfish_armv7_defconfig`: the kernel config file

- `sched.h`: a kernel file

Note: For any implementation details, please refer to my report `report.pdf`.
