; Pin definitions
SPI_SCLK            .set    5       ; PRU1_5 GPIO2_11 P8_42
SPI_MOSI            .set    0       ; PRU1_0 GPIO2_6  P8_45 (DIN)
SPI_MISO            .set    3       ; PRU1_3 GPIO2_9  P8_44 (DOUT)
SPI_CS              .set    7       ; PRU1_7 GPIO2_13 P8_40
ADS_START           .set    4       ; PRU1_4 GPIO2_10 P8_41
ADS_RESET           .set    2       ; PRU1_2 GPIO2_8  P8_43
ADS_DRDY            .set    1       ; PRU1_1 GPIO2_7  P8_46

; SPI_SCLK_DELAY = floor( (t_SCLK / 5 ns) / 4 ); SPI_SCLK_DELAY >= 3
SPI_SCLK_DELAY      .set    9

; Interrupt
PRU_INT_VALID       .set    32
PRU0_PRU1_INTERRUPT .set    1       ; PRU_EVTOUT_
PRU1_PRU0_INTERRUPT .set    2       ; PRU_EVTOUT_
PRU0_ARM_INTERRUPT  .set    3       ; PRU_EVTOUT_0
PRU1_ARM_INTERRUPT  .set    4       ; PRU_EVTOUT_1
ARM_PRU0_INTERRUPT  .set    5       ; PRU_EVTOUT_
ARM_PRU1_INTERRUPT  .set    6       ; PRU_EVTOUT_

; Name PRU register banks
XFR_BANK0           .set    10
XFR_BANK1           .set    11
XFR_BANK2           .set    12
XFR_PRU             .set    14


; Code starts here
    .text
    .retain
    .retainrefs
    .global         main

; Include ADS131 driver
    .include "ads131e08s.inc"

main:

; Start up sequence for the ADS


    ADS_STARTUP

    ADS_WAIT        2000
    ADS_WRITE_REG   CONFIG3,0xE0  ;vref = 4V
    ADS_WAIT        2000
    ADS_WRITE_REG   CONFIG1, 0x92

    ;ADS_WRITE_REG   CONFIG2, 0xF3 ;internal signal test + DC
    ;ADS_WRITE_REG   CONFIG2, 0xF0 ;internal signal test + Pulse fclk/2^21
    ;ADS_WRITE_REG   CONFIG2, 0xF1 ;internal signal test + Pulse fclk/2^20
    ;ADS_WRITE_REG   CONFIG2, 0xE0 ;external signal test

; Channel 1-8: PGA gain 1x, normal
    ADS_WRITE_REG   CH1SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH2SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH3SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH4SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH5SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH6SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH7SET, CH_GAIN_1 + CH_NORMAL
    ADS_WRITE_REG   CH8SET, CH_GAIN_1 + CH_NORMAL

; Calibrate offset
    ADS_SEND_CMD    ADS_CMD_OFFSETCAL

; Put ADS back in continuous data conversion mode
    ADS_SEND_CMD    ADS_CMD_RDATAC

; START = 1
    SET     r30, r30, ADS_START

; Start of main loop
mainloop:
    
     ADS_GET_DATA24  r1, r2, r3, r4, r5, r6, r7
     ADS_GET_DATA24  r1, r8, r9, r10, r11, r12, r13
     ADS_GET_DATA24  r1, r14, r15, r16, r17, r18, r19
     ADS_GET_DATA24  r1, r20, r21, r22, r23, r24, r25
     LDI  r1.b3,222
     LDI  r1.b2,173
     LDI  r1.b1,190
     LDI  r1.b0,239
;     LDI  r4.b1,10
;     LDI  r4.b2,11
;     LDI  r4.b3,12     
    XOUT    XFR_BANK0, &r1, 100                         ; Save to scratch pad
    LDI     r31.b0, PRU_INT_VALID + PRU1_PRU0_INTERRUPT ; Signal PRU0

    JMP mainloop        ; [TODO]: make loop conditional

; Stop PRU
    HALT


    ADS_INIT
