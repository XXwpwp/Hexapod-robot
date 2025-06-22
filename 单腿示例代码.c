/*这是一个连接在PWM信号0、1口的两自由度机械臂的测试代码，0为大腿、1为小腿、通过可调电源调节至5V即可运行（当你运行的是完整体时一共是12只舵机，根据数据手册以及实验需要7.2v的稳定电压以及接近10安左右的电流才能够最大负载运行）
 */ 

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


#define SERVO_MIN 150    
#define SERVO_MAX 600   
#define PWM_FREQ 50      

const int servo_pins[2] = {0, 1}; 


int current_angles[2] = {90, 90}; 

const int joint_limits[2][2] = {
  {40, 140},   
  {50, 120}    
};

void move_servo(int pin, int angle);
void smooth_move(int joint_index, int target_angle, int base_delay = 10, int step_size = 1);
void reset_servos();
void test_gait();


void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(PWM_FREQ);
  
  Serial.println(F("\n单腿平滑运动测试系统"));
  Serial.println(F("命令列表:"));
  Serial.println(F("M U/D 角度 → 移动关节(如: M U 120)"));
  Serial.println(F("S 延时 步长 → 设置平滑参数(如: S 5 1)"));
  Serial.println(F("G → 执行测试步态"));
  Serial.println(F("R → 复位到90°"));
  
  reset_servos();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("M")) {
    
      char joint = cmd.charAt(2);
      int angle = cmd.substring(4).toInt();
      
      int joint_index = (joint == 'U') ? 0 : 1;
      angle = constrain(angle, joint_limits[joint_index][0], joint_limits[joint_index][1]);
      
      smooth_move(joint_index, angle, 10, 1);
    }
    else if (cmd.startsWith("S")) {
   
      int step_delay = cmd.substring(2, cmd.indexOf(' ')).toInt();
      int step_size = cmd.substring(cmd.indexOf(' ')+1).toInt();
      
      Serial.print(F("设置平滑参数: 延时="));
      Serial.print(step_delay);
      Serial.print(F("ms 步长="));
      Serial.println(step_size);
    }
    else if (cmd == "G") {
      test_gait();
    }
    else if (cmd == "R") {
      reset_servos();
    }
  }
}


void smooth_move(int joint_index, int target_angle, int base_delay, int step_size) {
  int start_angle = current_angles[joint_index];
  int distance = abs(target_angle - start_angle);
  int direction = (target_angle > start_angle) ? 1 : -1;
  
  Serial.print(joint_index == 0 ? "大腿" : "小腿");
  Serial.print(F("移动: "));
  Serial.print(start_angle);
  Serial.print(F("° → "));
  Serial.print(target_angle);
  Serial.println(F("°"));

  for (int step = 0; step <= distance; step += step_size) {
    int current_angle = start_angle + direction * min(step, distance);
    float progress = (float)step / distance;
    int dynamic_delay = base_delay * (1.0 + 3.0 * (0.5 - abs(progress - 0.5)));
    
    move_servo(servo_pins[joint_index], current_angle);
    delay(dynamic_delay);
  }
  
  current_angles[joint_index] = target_angle;
}

void move_servo(int pin, int angle) {
  angle = constrain(angle, 0, 180);
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(pin, 0, pulse);
}

void reset_servos() {
  Serial.println(F("\n复位所有舵机到90°"));
  for (int i = 0; i < 2; i++) {
    smooth_move(i, 90, 15, 1);
  }
}

void test_gait() {
  Serial.println(F("\n执行测试步态..."));


  smooth_move(1, 180, 1, 4);
  
  
  smooth_move(0, 50, 10, 2);
  

  smooth_move(1, 55, 1, 3);

  smooth_move(0, 90);
  smooth_move(1, 90);
  
  Serial.println(F("步态完成!"));
}
