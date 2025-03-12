#include <Arduino.h>
#include <pthread.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#define STEP_PIN 18
#define DIR_PIN 16
#define ENABLE_PIN 7 

rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

volatile int current_command = 0;  // Stores latest joystick command
int last_command = 99;  // Unlikely starting value
unsigned long last_step_time = 0;
const int step_delay = 200; // Microseconds between steps

void error_loop() {
    while(1) {
        delay(100);
    }
}

void *ros_spin_thread(void *arg) {
  while (1) {
      RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
      delay(10); // Small delay to avoid high CPU usage
  }
  return NULL;
}

hw_timer_t *step_timer = NULL;

void IRAM_ATTR stepperISR() {
    if (current_command != 0) {  
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(50);  // Short pulse width
        digitalWrite(STEP_PIN, LOW);
    }
}

void stepper_motor_callback(const void *msg_in)
{
    const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msg_in;

    // Only update if the command is different
    if (msg->data != last_command) {
        last_command = msg->data;
        current_command = msg->data;
        
        Serial.print("New joystick command: ");
        Serial.println(current_command);

        if (current_command == 1) {
            digitalWrite(DIR_PIN, HIGH);  // Move forward
            digitalWrite(ENABLE_PIN, LOW); // Enable motor
        } 
        else if (current_command == -1) {
            digitalWrite(DIR_PIN, LOW);  // Move backward
            digitalWrite(ENABLE_PIN, LOW); // Enable motor
        } 
        else if (current_command == 0) {
            digitalWrite(ENABLE_PIN, HIGH);  // Stop motor
        }
    }
}

void setup()
{
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, HIGH); // Start with motor disabled

    Serial.begin(115200);
    set_microros_serial_transports(Serial);
    delay(2000);
    allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

    RCCHECK(rclc_subscription_init_default(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "stepper_command"));

    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &stepper_motor_callback, ON_NEW_DATA));
    pthread_t rosThread;
    pthread_create(&rosThread, NULL, ros_spin_thread, NULL);

    step_timer = timerBegin(0, 80, true);  // Timer 0, prescaler 80 (1us per count)
    timerAttachInterrupt(step_timer, &stepperISR, true);
    timerAlarmWrite(step_timer, step_delay, true);
    timerAlarmEnable(step_timer);
}

void loop()
{
  
}
