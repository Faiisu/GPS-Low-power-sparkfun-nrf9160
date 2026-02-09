#include <zephyr/kernel.h>

/* 1. นิยาม Semaphore: เริ่มต้นที่ค่า 0 และมีค่าสูงสุดได้ที่ 1 (Binary Semaphore) */
K_SEM_DEFINE(my_sensor_sem, 0, 1);

struct sensor_data {
    float temperature;
    float humidity;
} shared_data;

// --- Thread สำหรับอ่านค่าเซนเซอร์ ---
void sensor_thread(void *dummy1, void *dummy2, void *dummy3) {
    while (1) {
        // จำลองการอ่านค่าพิกัดพิกัดนกหรือช้าง
        shared_data.temperature = 30.5; 
        shared_data.humidity = 60.0;

        printk("Sensor: Data collected. Signaling logger...\n");

        /* 2. Give Semaphore: ส่งสัญญาณให้ Thread อื่นรู้ว่าข้อมูลพร้อมแล้ว */
        k_sem_give(&my_sensor_sem);

        k_msleep(5000); // อ่านทุกๆ 5 วินาที
    }
}

// --- Thread สำหรับบันทึกหรือส่งข้อมูล (LTE/GPS) ---
void logger_thread(void *dummy1, void *dummy2, void *dummy3) {
    while (1) {
        /* 3. Take Semaphore: รอจนกว่าจะได้สัญญาณ (รอไปเรื่อยๆ K_FOREVER) */
        if (k_sem_take(&my_sensor_sem, K_FOREVER) == 0) {
            
            // เมื่อได้ Semaphore แล้วจึงทำงานส่วนนี้
            printk("Logger: Received signal! Processing data: T=%.2f, H=%.2f\n",
                    shared_data.temperature, shared_data.humidity);
            
            // จำลองการส่งข้อมูลผ่านโมเด็ม nRF9160
        }
    }
}

// การนิยาม Thread ใน Zephyr
K_THREAD_DEFINE(sensor_tid, 1024, sensor_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(logger_tid, 1024, logger_thread, NULL, NULL, NULL, 7, 0, 0);