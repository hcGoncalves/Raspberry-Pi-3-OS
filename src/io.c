#include <io.h>
// GPIO

void mmioWrite(long reg, unsigned int val) { *(volatile unsigned int *)reg = val; }
unsigned int mmioRead(long reg) { return *(volatile unsigned int *)reg; }

unsigned int gpioCall(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
  
    if (pin_number > field_max) return 0;
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    unsigned int reg = base + ((pin_number / num_fields) * 4);
    unsigned int shift = (pin_number % num_fields) * field_size;

    unsigned int curval = mmioRead(reg);
    curval &= ~(field_mask << shift);
    curval |= value << shift;
    mmioWrite(reg, curval);

    return 1;
}

unsigned int gpioSet     (unsigned int pin_number, unsigned int value) { return gpioCall(pin_number, value, GPSET0_ADDRESS, 1, GPIO_MAX_PIN); }
unsigned int gpioClear   (unsigned int pin_number, unsigned int value) { return gpioCall(pin_number, value, GPCLR0_ADDRESS, 1, GPIO_MAX_PIN); }
unsigned int gpioPull    (unsigned int pin_number, unsigned int value) { return gpioCall(pin_number, value, GPPUPPDN0_ADDRESS, 2, GPIO_MAX_PIN); }
unsigned int gpioFunction(unsigned int pin_number, unsigned int value) { return gpioCall(pin_number, value, GPFSEL0_ADDRESS, 3, GPIO_MAX_PIN); }

void gpioUseAsAlt3(unsigned int pin_number) {
    gpioPull(pin_number, Pull_None);
    gpioFunction(pin_number, GPIO_FUNCTION_ALT3);
}

void gpioUseAsAlt5(unsigned int pin_number) {
    gpioPull(pin_number, Pull_None);
    gpioFunction(pin_number, GPIO_FUNCTION_ALT5);
}

void gpioInitOutputPinWithPullNone(unsigned int pin_number) {
    gpioPull(pin_number, Pull_None);
    gpioFunction(pin_number, GPIO_FUNCTION_OUT);
}

void gpioSetPinOutputBool(unsigned int pin_number, unsigned int onOrOff) {
    if (onOrOff) {
        gpioSet(pin_number, 1);
    } else {
        gpioClear(pin_number, 1);
    }
}

// UART

enum {
    AUX_BASE        = PERIPHERAL_BASE + 0x215000,
    AUX_IRQ         = AUX_BASE,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_MSR_REG  = AUX_BASE + 88,
    AUX_MU_SCRATCH  = AUX_BASE + 92,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_STAT_REG = AUX_BASE + 100,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK  = 500000000,
    UART_MAX_QUEUE  = 16 * 1024
};

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

unsigned char uart_output_queue[UART_MAX_QUEUE];
unsigned int uart_output_queue_write = 0;
unsigned int uart_output_queue_read = 0;

void uartInit() {
    mmioWrite(AUX_ENABLES, 1); //enable UART1
    mmioWrite(AUX_MU_IER_REG, 0);
    mmioWrite(AUX_MU_CNTL_REG, 0);
    mmioWrite(AUX_MU_LCR_REG, 3); //8 bits
    mmioWrite(AUX_MU_MCR_REG, 0);
    mmioWrite(AUX_MU_IER_REG, 0);
    mmioWrite(AUX_MU_IIR_REG, 0xC6); //disable interrupts
    mmioWrite(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpioUseAsAlt5(14);
    gpioUseAsAlt5(15);
    mmioWrite(AUX_MU_CNTL_REG, 3); //enable RX/TX
}

unsigned int uartIsOutputQueueEmpty() {
    return uart_output_queue_read == uart_output_queue_write;
}

unsigned int uartIsReadByteReady()  { return mmioRead(AUX_MU_LSR_REG) & 0x01; }
unsigned int uartIsWriteByteReady() { return mmioRead(AUX_MU_LSR_REG) & 0x20; }

unsigned char uartReadByte() {
    while (!uartIsReadByteReady());
    return mmioRead(AUX_MU_IO_REG) & 0xFF;
}

void uartWriteByteBlockingActual(unsigned char ch) {
    while (!uartIsWriteByteReady()); 
    mmioWrite(AUX_MU_IO_REG, (unsigned int)ch);
}

void uartLoadOutputFifo() {
    while (!uartIsOutputQueueEmpty() && uartIsWriteByteReady()) {
        uartWriteByteBlockingActual(uart_output_queue[uart_output_queue_read]);
        uart_output_queue_read = (uart_output_queue_read + 1) & (UART_MAX_QUEUE - 1);
    }
}

void uartWriteByteBlocking(unsigned char ch) {
    unsigned int next = (uart_output_queue_write + 1) & (UART_MAX_QUEUE - 1);

    while (next == uart_output_queue_read) uartLoadOutputFifo();

    uart_output_queue[uart_output_queue_write] = ch;
    uart_output_queue_write = next;
}

void uartWriteText(char *buffer) {
    while (*buffer) {
       if (*buffer == '\n') uartWriteByteBlocking('\r');
       uartWriteByteBlocking(*buffer++);
    }
}

void uartDrainOutputQueue() {
    while (!uartIsOutputQueueEmpty()) uartLoadOutputFifo();
}

void uartUpdateEcho() {
    uartLoadOutputFifo();

    if (uartIsReadByteReady()) {
       unsigned char ch = uartReadByte();
       if (ch == '\r') uartWriteText("\n"); else uartWriteByteBlocking(ch);
    }
}

char uartUpdateReturn() {
    uartLoadOutputFifo();

    if (uartIsReadByteReady()) {
       return uartReadByte();
    }

    return '\0';
}