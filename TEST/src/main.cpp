
#include "mbed.h"

// ---------------- UART ----------------
UnbufferedSerial pc(USBTX, USBRX, 115200);

// ---------------- LCD PINS (4-bit mode) ----------------
DigitalOut lcd_rs(PA_8);   // D7
DigitalOut lcd_en(PA_9);   // D8
DigitalOut lcd_d4(PC_7);   // D9
DigitalOut lcd_d5(PB_6);   // D10
DigitalOut lcd_d6(PA_7);   // D11
DigitalOut lcd_d7(PA_6);   // D12

// ---------------- Sensors & Actuators ----------------
AnalogIn soil(A0);
AnalogIn ldr(A1);
DigitalOut pump(D3);
DigitalOut buzzer(D5);
DigitalIn pir(D4);

// ---------------- LCD FUNCTIONS ----------------
void lcd_pulse() {
    lcd_en = 1;
    wait_us(1);
    lcd_en = 0;
    wait_us(50);
}

void lcd_write4(int value) {
    lcd_d4 = (value >> 0) & 1;
    lcd_d5 = (value >> 1) & 1;
    lcd_d6 = (value >> 2) & 1;
    lcd_d7 = (value >> 3) & 1;
    lcd_pulse();
}

void lcd_cmd(int cmd) {
    lcd_rs = 0;
    lcd_write4(cmd >> 4);
    lcd_write4(cmd & 0x0F);
    wait_us(40);
}

void lcd_data(int data) {
    lcd_rs = 1;
    lcd_write4(data >> 4);
    lcd_write4(data & 0x0F);
    wait_us(40);
}

void lcd_init() {
    wait_us(50000);
    lcd_rs = 0;
    lcd_en = 0;

    lcd_write4(0x03);
    wait_us(4500);
    lcd_write4(0x03);
    wait_us(4500);
    lcd_write4(0x03);
    wait_us(150);
    lcd_write4(0x02);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    wait_us(2000);
}

void lcd_set(int col, int row) {
    int offsets[] = {0x00, 0x40};
    lcd_cmd(0x80 | (col + offsets[row]));
}

void lcd_print(const char *s) {
    while (*s) lcd_data(*s++);
}

// ---------------- MAIN ----------------
int main() {
    lcd_init();
    lcd_set(0, 0);
    lcd_print("Smart System");
    thread_sleep_for(1500);
    lcd_cmd(0x01);

    while (true) {

        // ---------------- Read Sensors ----------------
        float soil_raw = soil.read();          // 0.0 – 1.0
        float moisture = (1.0f - soil_raw) * 100.0f;

        int ldrValue = ldr.read_u16() >> 4;    // scale to 0–4095
        int motion = pir.read();

        // ---------------- Pump Logic ----------------
        bool motorState = false;

        if (moisture < 50.0f) {
            pump = 1;
            motorState = true;
        } else {
            pump = 0;
            motorState = false;
        }

        // ---------------- Buzzer Logic ----------------
        if (motion || ldrValue > 2000) {
            buzzer = 1;
        } else {
            buzzer = 0;
        }

        // ---------------- LCD DISPLAY ----------------
        lcd_set(0, 0);
        lcd_print("Moist:");
        char buf[16];
        sprintf(buf, "%.0f%%   ", moisture);
        lcd_print(buf);

        lcd_set(0, 1);
        lcd_print("Motor:");
        lcd_print(motorState ? "ON " : "OFF");

        lcd_print(" M:");
        lcd_print(motion ? "YES" : "NO ");

        // ---------------- Serial Output ----------------
        char msg[128];
        sprintf(msg,
                "Moisture: %.1f%% | Motor: %s | Motion: %s | LDR: %d\r\n",
                moisture,
                motorState ? "ON" : "OFF",
                motion ? "YES" : "NO",
                ldrValue);

        pc.write(msg, strlen(msg));

        thread_sleep_for(2000);
    }
}
