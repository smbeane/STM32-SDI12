# STM32-SDI12
Its a [SDI-12](https://www.sdi-12.org/) library for [STM32](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html) microcontrollers.

## Method
Single UART/IO pin configured as Single Wire (Half-Duplex) and toggled between GPIO, Receiver, and Transmitter

## Config (7E1)
- Baud Rate: 1200 Bits/s
- Word Length: 8 Bit (include parity)
- Parity: Even
- Stop Bits: 1
- TX Pin Active Level Inversion: True
- RX Pin Active Level Inversion: True

## Tested on (fork)
- [x] STM32G031K8

## Supports
*Where **a** or **b** is a devices address and **n** is a stored data index.*
- [x] Acknowledge active (**a**!)
- [x] Send identification (**a**I!)
- [x] Change address (**a**A**b**!)
- [x] Start measurement (**a**M!)
- [x] Send data (**a**D**n**!) where **n** = 0 to 9
- [x] Start verification (**a**V!)
- [ ] Start concurrent measurement (**a**C!)

## License
This work is MIT licensed as found in the [LICENSE](https://github.com/HarveyBates/STM32-SDI12/blob/master/LICENSE) file.
