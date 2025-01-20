#include <Arduino.h>
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
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

int steps_to_move = 0;
void error_loop() {
  while(1) {
    delay(100);
  }
}

void stepper_motor_callback(const void *msg_in)
{
    const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msg_in;

    steps_to_move = msg->data;
    Serial.print("Received steps: ");
    Serial.println(steps_to_move);

    // Set direction
    if (steps_to_move > 0) {
        digitalWrite(DIR_PIN, HIGH);
    } else {
        digitalWrite(DIR_PIN, LOW);
        steps_to_move = -steps_to_move; // Make positive
    }

    // Step motor
    for (int i = 0; i < steps_to_move; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(100);
    }
}

void setup()
{
    // Initialize GPIO pins
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    
    // Initialize micro-ROS
    Serial.begin(115200);
    set_microros_serial_transports(Serial);
    delay(2000);
    allocator = rcl_get_default_allocator();

    //create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // create node
    RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

    // Create subscriber
    RCCHECK(rclc_subscription_init_default(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "stepper_command"));

    // Create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &stepper_motor_callback, ON_NEW_DATA));

}

void loop()
{
  delay(10);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    // micro-ROS logic is handled in setup, no loop logic needed
}