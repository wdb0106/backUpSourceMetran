#include "TestMotorSPI.h"
#include "SystemConfigure.h"
#include "Delay.h"

//P3.3 SCK(1)   => (2) SSP0_SCK - R39. xanh la
//P3.6 MISO(1)  => (0) GPIO 0[6] - R41 tim
//P3.7 MOSI(1)  => (2) SSPO_MISO  - R40 trang
//P3.8 CS(4) - GPIO 5(11) => (2)SSPO -MOSI

#define SCK_PORT                    (0x1)
#define SCK_PIN                     (19)

#define MISO_PORT                   (0x1)
#define MISO_PIN                    (3)

#define MOSI_PORT                   (0x1)
#define MOSI_PIN                    (4)

#define CS1_PORT                     (0x1)
#define CS1_PIN                      (5)
#define CS1_GPIO_PORT                (1)
#define CS1_GPIO_PIN                 (8)

#define CS2_PORT                     (0x1)
#define CS2_PIN                      (6)
#define CS2_GPIO_PORT                (1)
#define CS2_GPIO_PIN                 (9)

#define CS3_PORT                     (0x1)
#define CS3_PIN                      (7)
#define CS3_GPIO_PORT                (1)
#define CS3_GPIO_PIN                 (0)

#define CS4_PORT                     (0x1)
#define CS4_PIN                      (8)
#define CS4_GPIO_PORT                (1)
#define CS4_GPIO_PIN                 (1)

#define LPC_SSP           LPC_SSP1
#define SSP_IRQ           SSP0_IRQn
#define SSP_BIT_RATE                (1500000)

#define CLK0                        (0)

#define MAX_STROKE_VOL_ADJUSTMENT  200
#define MIN_STROKE_VOL_ADJUSTMENT  0
#define NUM_STROKE_INDEX (MAX_STROKE_VOL_ADJUSTMENT - MIN_STROKE_VOL_ADJUSTMENT + 1)

#define NUM_FREQUENCY_INDEX 14

static const int32_t steps_for_big_ml[NUM_FREQUENCY_INDEX][NUM_STROKE_INDEX] = {
        {0, 628, 1253, 1873, 2486, 3089, 3679, 4256, 4815, 5356,
                5875, 6372, 6843, 7287, 7702, 8087, 8441, 8760, 9046, 9296,
                9509, 9684, 9822, 9920, 9980, 10000, 9981, 9922, 9825, 9688,
                9514, 9301, 9053, 8768, 8449, 8097, 7712, 7298, 6854, 6384,
                5888, 5369, 4829, 4270, 3694, 3104, 2501, 1889, 1268, 643,
                16, -612, -1237, -1857, -2470, -3074, -3665, -4241, -4801, -5342,
                -5862, -6359, -6831, -7276, -7692, -8078, -8432, -8753, -9039, -9290,
                -9504, -9680, -9819, -9918, -9979, -10000, -9982, -9924, -9827, -9692,
                -9518, -9307, -9059, -8776, -8458, -8106, -7723, -7309, -6866, -6396,
                -5901, -5383, -4843, -4285, -3709, -3119, -2517, -1904, -1284, -659, -32},
        {0, 753, 1502, 2242, 2969, 3679, 4369, 5034, 5670, 6274,
                6843, 7372, 7860, 8303, 8699, 9046, 9341, 9583, 9771, 9904,
                9980, 9999, 9962, 9868, 9718, 9514, 9255, 8943, 8581, 8170,
                7712, 7211, 6669, 6089, 5475, 4829, 4156, 3460, 2743, 2012,
                1268, 518, -235, -987, -1734, -2470, -3193, -3897, -4579,
                -5236, -5862, -6456, -7012, -7529, -8003, -8432, -8813, -9144, -9423,
                -9648, -9819, -9933, -9992, -9994, -9939, -9827, -9660, -9438, -9163,
                -8835, -8458, -8032, -7561, -7046, -6492, -5901, -5276, -4622, -3941,
                -3238, -2517, -1781, -1035, -283},
        {0, 878, 1749, 2607, 3445, 4256, 5034, 5773, 6468, 7113,
                7702, 8233, 8699, 9099, 9428, 9684, 9866, 9971, 9999, 9950,
                9825, 9623, 9347, 8999, 8581, 8097, 7550, 6945, 6287, 5579,
                4829, 4042, 3223, 2379, 1517, 643, -235, -1112, -1980, -2834,
                -3665, -4467, -5236, -5964, -6645, -7276, -7850, -8364, -8813,
                -9194, -9504, -9740, -9901, -9986, -9994, -9924, -9778, -9556,
                -9261, -8893, -8458, -7956, -7394, -6774, -6102, -5383, -4622,
                -3825, -2999, -2150, -1284, -409},
        {0, 1003, 1996, 2969, 3912, 4815, 5670, 6468, 7200, 7860,
                8441, 8936, 9341, 9652, 9866, 9980, 9993, 9906, 9718,
                9433, 9053, 8581, 8022, 7383, 6669, 5888, 5048, 4156,
                3223, 2257, 1268, 267, -737, -1734, -2713, -3665, -4579,
                -5448, -6262, -7012, -7692, -8294, -8813, -9242, -9579,
                -9819, -9959, -9999, -9939, -9778, -9518, -9163, -8715,
                -8179, -7561, -6866, -6102, -5276, -4398, -3475, -2517,
                -1533, -534},
        {0, 1128, 2242, 3327, 4369, 5356, 6274, 7113, 7860, 8507,
                9046, 9469, 9771, 9949, 9999, 9922, 9718, 9391, 8943,
                8381, 7712, 6945, 6089, 5156, 4156, 3104, 2012, 894,
                -235, -1361, -2470, -3548, -4579, -5553, -6456, -7276,
                -8003, -8629, -9144, -9542, -9819, -9970, -9994, -9890,
                -9660, -9307, -8835, -8251, -7561, -6774, -5901, -4953,
                -3941, -2879, -1781, -659},
        {0, 1253, 2486, 3679, 4815, 5875, 6843, 7702, 8441, 9046,
                9509, 9822, 9980, 9981, 9825, 9514, 9053, 8449, 7712,
                6854, 5888, 4829, 3694, 2501, 1268, 16, -1237, -2470,
                -3665, -4801, -5862, -6831, -7692, -8432, -9039, -9504,
                -9819, -9979, -9982, -9827, -9518, -9059, -8458, -7723,
                -6866, -5901, -4843, -3709, -2517, -1284, -32},
        {0, 1377, 2728, 4027, 5249, 6372, 7372, 8233, 8936, 9469, 9822,
                9987, 9962, 9747, 9347, 8768, 8022, 7124, 6089, 4939, 3694,
                2379, 1019, -361, -1734, -3074, -4355, -5553, -6645, -7611,
                -8432, -9092, -9579, -9883, -9999, -9924, -9660, -9212, -8589,
                -7802, -6866, -5799, -4622, -3357, -2027, -659},
        {0, 1502, 2969, 4369, 5670, 6843, 7860, 8699, 9341, 9771, 9980,
                9962, 9718, 9255, 8581, 7712, 6669, 5475, 4156, 2743,
                1268, -235, -1734, -3193, -4579, -5862, -7012, -8003,
                -8813, -9423, -9819, -9992, -9939, -9660, -9163, -8458,
                -7561, -6492, -5276, -3941, -2517, -1035},
        {0, 1626, 3208, 4705, 6077, 7287, 8303, 9099, 9652, 9949, 9981,
                9747, 9255, 8516, 7550, 6384, 5048, 3577, 2012, 393,
                -1237, -2834, -4355, -5760, -7012, -8078, -8929, -9542,
                -9901, -9997, -9827, -9396, -8715, -7802, -6681, -5383,
                -3941, -2395, -785},
        {0, 1749, 3445, 5034, 6468, 7702, 8699, 9428, 9866, 9999, 9825,
                9347, 8581, 7550, 6287, 4829, 3223, 1517, -235, -1980,
                -3665, -5236, -6645, -7850, -8813, -9504, -9901, -9994,
                -9778, -9261, -8458, -7394, -6102, -4622, -2999, -1284},
        {0, 1873, 3679, 5356, 6843, 8087, 9046, 9684, 9980, 9922, 9514,
                8768, 7712, 6384, 4829, 3104, 1268, -612, -2470, -4241,
                -5862, -7276, -8432, -9290, -9819, -10000, -9827, -9307,
                -8458, -7309, -5901, -4285, -2517, -659},
        {0, 1996, 3912, 5670, 7200, 8441, 9341, 9866, 9993, 9718, 9053,
                8022, 6669, 5048, 3223, 1268, -737, -2713, -4579, -6262,
                -7692, -8813, -9579, -9959, -9939, -9518, -8715, -7561,
                -6102, -4398, -2517, -534},
        {0, 2119, 4142, 5976, 7540, 8760, 9583, 9971, 9906, 9391, 8449,
                7124, 5475, 3577, 1517, -612, -2713, -4691, -6456, -7927,
                -9039, -9740, -9999, -9803, -9163, -8106, -6681, -4953, -2999, -910},

                //test
                {0, 2486, 4815, 6843, 8441, 9509, 9980, 9825, 9053, 7712, 5888, 3694, 1268,
                        -1237, -3665, -5862, -7692, -9039, -9819, -9982, -9518, -8458, -6866,
                        -4843, -2517, -32
                }

};

TestMotorSPI *TestMotorSPI::S_Instance = NULL;

int32_t CreateSin(int iFrequence)
{
    int returnValue = 0;
    static int index = 0;

    int iMaxIndex = 1000 / (iFrequence);
    if(iMaxIndex % 2 == 0)
    {
        iMaxIndex = iMaxIndex / 2;
    }
    else
    {
        iMaxIndex -= 1;
        iMaxIndex = iMaxIndex / 2;
    }

    returnValue = steps_for_big_ml[iFrequence - 5][index];
    ++index;

    if(index == iMaxIndex)
    {
        index = 0;
    }

    return returnValue;
}

void TestMotorSPI::TestMotorSPICreateTask (void)
{
    xTaskCreate(Entry, "TestMotorSPI", HFOSPI_TASK_SIZE, NULL,
                HFOSPI_TASK_PRIORITY, (TaskHandle_t *) NULL);
}


TestMotorSPI* TestMotorSPI::S_GetInstance (void)
{
    //Create object if it is null
    if (S_Instance == NULL)
    {
        S_Instance = new TestMotorSPI();
    }
    //check object is null or not
    ASSERTION(S_Instance != NULL);

    return(S_Instance);
}

void TestMotorSPI::Init (void)
{
    //Initialize for SPI
    //PIN SCK
    Chip_SCU_PinMuxSet(SCK_PORT, SCK_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC1));
    //PIN MISO
    Chip_SCU_PinMuxSet(MISO_PORT, MISO_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC5));
    //PIN MOSI
    Chip_SCU_PinMuxSet(MOSI_PORT, MOSI_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC5));

    //PIN CS
    Chip_SCU_PinMuxSet(CS1_PORT, CS1_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC0));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, CS1_GPIO_PORT, CS1_GPIO_PIN);

    Chip_SCU_PinMuxSet(CS2_PORT, CS2_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC0));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, CS2_GPIO_PORT, CS2_GPIO_PIN);

    Chip_SCU_PinMuxSet(CS3_PORT, CS3_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC0));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, CS3_GPIO_PORT, CS3_GPIO_PIN);

    Chip_SCU_PinMuxSet(CS4_PORT, CS4_PIN, (SCU_PINIO_FAST |SCU_MODE_FUNC0));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, CS4_GPIO_PORT, CS4_GPIO_PIN);

    //Set PIN to HIGH - default
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS1_GPIO_PORT, CS1_GPIO_PIN,true);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS2_GPIO_PORT, CS2_GPIO_PIN,true);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS3_GPIO_PORT, CS3_GPIO_PIN,true);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS4_GPIO_PORT, CS4_GPIO_PIN,true);

    //power for motor
    Chip_SCU_PinMuxSet(0x2, 2, (SCU_PINIO_FAST |SCU_MODE_FUNC4));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 2);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT,5, 2, true);

    // initialize SSP
    Chip_SSP_Init(LPC_SSP);
    Chip_SSP_SetMaster(LPC_SSP, true);
    Chip_SSP_SetBitRate(LPC_SSP, SSP_BIT_RATE);
    Chip_SSP_SetFormat(LPC_SSP, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
    Chip_SSP_Enable(LPC_SSP);
    Chip_SSP_Int_Enable(LPC_SSP);   /* enable interrupt */


    //set up clock out put = CLKIN_MAINPLL/ 12
    //initialize function for CLK0 pin
    Chip_SCU_ClockPinMuxSet(CLK0, SCU_MODE_FUNC1);

    Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, 17);
    Chip_Clock_SetBaseClock(CLK_BASE_OUT, CLKIN_IDIVE, true, false);

}
static void motor_init(uint8_t MotorID)
{
    //initialize motor

    /*=============SPI_OUT_CONF(Add = R: 0x04 - W: 0x84): = 0x4440006B=================
    +SPI_OUT_LOW_TIME (SPI output transfer idle state duration)                   : 4
    +SPI_OUT_HIGH_TIME (SPI output clock high level duration)                     : 4
    +SPI_OUT_LOW_TIME (SPI output clock low level duration)                       : 4
    +COVER_DATA_LENGTH (Datagram length. Set to 0 for TMCx drivers)               : 0
    +POLL_BLOCK_MULT (Exponent for idle state time between two polling datagrams) : 0
    +disable_polling(No transfer of polling datagrams) = A
    +scale_vale_transfer_en(Transfer of scaling values in scaling values alter) = B
             4 (AB = 00) 2(AB = 10) 4(AB = 01) 6(AB = 11)                         : 6
    +spi_output_format(Selection of connected driver/DAC)                         : B
             A: 26x-SPI, B: 26x-S/D
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0x84,0x4440006B);

    /*=============CLK_FREQ(Add = R: 0x31 - W: 0xB1): = 0x00B71B00====================
    + CLK_FREQ(Frequence): 0x007A1200 - 8Mhz
                           0x00B71b00 - 12Mhz
                           0x00F42400 -16Mhz
                           0x01036640 -17Mhz
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xB1,0x00B71B00);

    /*==============GENERAL_CONF(Add = R: 0x80 - W: 0x00):= 0x00006026=================
    Direct-a, direct - bow
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0x80,0x00006026);

    /*================REFERENCE_CONF(Add = R: 0x90 - W: 0x01): =0x00010001 ============
    StepLength
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0x90,0x00010001);

    /*==============COVER_LOW(Add = R: 0xEC - W: 0x6C):= 0xEC,0x00000000================
    DRV_STATUS response and COVER_LOW send

    Using cover datagram to write to DRVCTRL of TMC26x  sendData   (0xEC,0x00000000)
    0x00000000: single edge steps, disable step interpolation - 256 microstep
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x00000000);

    /*==============COVER_LOW(Add = R: 0xEC - W: 0x6C):= 0xEC,0x00091935================
    DRV_STATUS response and COVER_LOW send

    Using cover datagram to write to CHOPCONF of TMC26x        (0xEC,0x00090585)
    tbl=36, standard chopper, HDEC=16, HEND=11, HSTR=1, TOFF=5, RNDTF=off
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x00091935);

    /*==============COVER_LOW(Add = R: 0xEC - W: 0x6C):= 0xEC,0x000A0000================
    DRV_STATUS response and COVER_LOW send

    Using cover datagram to write to SMARTEN of TMC26x (0xEC,0x000A0000)
    tbl=36, standard chopper, HDEC=16, HEND=11, HSTR=1, TOFF=5, RNDTF=off
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x000A0000);

    /*==============COVER_LOW(Add = R: 0xEC - W: 0x6C):= 0xEC,0x000D05xx================
    DRV_STATUS response and COVER_LOW send

    Using cover datagram to write to SGCSCONF of TMC26x (0xEC,0x000D05xx)
    Disable Smarten Register, SGT=0,CS=24
    */
    if(MotorID == 1)
    {
        // Disable Smarten Register
        TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x000D0519); // Using cover datagram to write to SGCSCONF of TMC26x 06 for small motor                   (0xEC,0x000C0018)
    }
    else
    {
        // Disable Smarten Register
        TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x000D0506); // Using cover datagram to write to SGCSCONF of TMC26x 06 for small motor                   (0xEC,0x000C0018)
    }

    /*==============COVER_LOW(Add = R: 0xEC - W: 0x6C):= 0xEC,0x000EF040==============
    DRV_STATUS response and COVER_LOW send

    Using cover datagram to write to DRVCONF of TMC26x     (0xEC,0x000EF000)
    SLPH = 3, SLPL = 3, DISS2G = off, TS2G = 0-3.2µs, SDOFF = on, VSENSE = 0
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xEC,0x000EF040); // Using cover datagram to write to DRVCONF of TMC26x                     (0xEC,0x000EF000)

    /*==============VMAX(Add = R: 0x24 - W: 0xA4):= 0xA4,0x00000000================
    Actual ramp generator position- x = 0
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA4,0x00000000);

    /*==============XACTUAL(Add = R: 0x21 - W: 0xA1):= 0xA4,0x00000000================
    Target ramp generator velocity- v = 0
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA1,0x00000000);

    /*==============VSTART(Add = R: 0x37 - W: 0xB7):= 0xB7,0x00000000================
    Start velocity of ramp generator- v = 0
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xB7,0x00000000);

    /*==============RAMPMODE(Add = R: 0x20 - W: 0xA0):= 0xA0,0x00000005================
      bit 2: Ramp_mode
              1 Positioning: XTARGET is superior objective for velocity ramp
              0 Velocitiy mode: VMAX is superior objective for velocity ramp
      bit 1 - 0: Ramp_type
              00 Hold mode: |-|
              01 Trapezoidal ramp /-\
              10 S-shaped ramp /'-'\

    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA0,0x00000000);

    /*==============VMAX(Add = R: 0x24 - W: 0xA4):= 0xA0, 0x03E80000================
      03E80000 200K
      0x01860000    100k
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA4,0x01860000); // VMAX =  [pps] 03E80000 200K / 186a0=100k

    /*==============AMAX(Add = R: 0x28 - W: 0xA8):= 0xA0, 0x00001000================
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA8,0x00001000); // AMAX

    /*==============DMAX(Add = R: 0x29 - W: 0xA9):= 0xA0, 0x00001000================
    */
    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xA9,0x00001000); // DMAX

//    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xAD,0x00000100); // bow1
//    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xAE,0x00000100); // bow2
//    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xAF,0x00000100); // bow3
//    TestMotorSPI::S_GetInstance()->sendData2(MotorID, 0xB0,0x00000100); // bow4
}

void RunAllMotorToPosition(uint32_t Pos)
{
    TestMotorSPI::S_GetInstance()->sendData2(1, 0xB7,Pos); // xtarget = Pos
    TestMotorSPI::S_GetInstance()->sendData2(2, 0xB7,Pos); // xtarget = Pos
    TestMotorSPI::S_GetInstance()->sendData2(3, 0xB7,Pos); // xtarget = Pos
    TestMotorSPI::S_GetInstance()->sendData2(4, 0xB7,Pos); // xtarget = Pos
}


void TestMotorSPI::Entry (void* pvParameters)
{
    const int FOREVER = 1;

//    int32_t Position = 0;
    static uint8_t ID = 0;

    motor_init(1);
    motor_init(2);
    motor_init(3);
    motor_init(4);


    while(FOREVER)
    {
        DEBUG_HFO("HFO==============\n");

//        Position = CreateSin(15);
        switch (ID)
        {
            case 0:
                RunAllMotorToPosition(0x00000200);
                delay_ms(25);
                break;

            case 1:
                RunAllMotorToPosition(0x00000400);
                break;

            case 2:
                RunAllMotorToPosition(0x00000600);
                break;

            case 3:
                RunAllMotorToPosition(0x00000800);
                break;

            case 4:
                RunAllMotorToPosition(0x00000A00);
                break;

            case 5:
                RunAllMotorToPosition(0x00000C00);
                break;

            case 6:
                RunAllMotorToPosition(0x00000E00);
                break;

            case 7:
                RunAllMotorToPosition(0x00001000);
                break;

            case 8:
                RunAllMotorToPosition(0x00001200);
                break;

            case 9:
                RunAllMotorToPosition(0x00001400);
                break;

            case 10:
                RunAllMotorToPosition(0x00001600);
                delay_ms(25);
                break;

            case 11:
                RunAllMotorToPosition(0x00001400);
                break;

            case 23:
                RunAllMotorToPosition(0x00001200);
                break;

            case 24:
                RunAllMotorToPosition(0x00001000);
                break;

            case 25:
                RunAllMotorToPosition(0x00000E00);
                break;

            case 26:
                RunAllMotorToPosition(0x00000C00);
                break;

            case 27:
                RunAllMotorToPosition(0x00000A00);
                break;

            case 28:
                RunAllMotorToPosition(0x00000800);
                break;

            case 29:
                RunAllMotorToPosition(0x00000600);
                break;

            case 30:
                RunAllMotorToPosition(0x00000400);
                break;

            default:
                ID = 0;
                break;
        }
        ++ID;

        vTaskDelay(2/portTICK_PERIOD_MS);
    }
}

// TMC4361 SPI DATAGRAM STRUCTURE
// MSB (transmitted first) == 40 Bit ==LSB (transmitted last)
// 39 - 32                              31                       0
// to TMC4361: RW + 7 bit address       8 bit data x 4
// from TMC4361: 8 bit SPI status       8 bit data x 4
void TestMotorSPI::sendData2 (uint8_t MotorId, uint8_t address, uint32_t datagram)
{
    //TMC4361 takes 40 bit data: 8 address and 32 data
    uint8_t i_datagram;

    switch (MotorId)
    {
        case 1:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS1_GPIO_PORT, CS1_GPIO_PIN, false);
            break;

        case 2:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS2_GPIO_PORT, CS2_GPIO_PIN, false);
            break;

        case 3:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS3_GPIO_PORT, CS3_GPIO_PIN, false);
            break;

        case 4:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS4_GPIO_PORT, CS4_GPIO_PIN, false);
            break;

        default:
            break;
    }
    delay_ms(1);

    //send address byte
    Chip_SSP_WriteFrames_Blocking(LPC_SSP, &address, 1);

    //send datagram
    i_datagram = (uint8_t)((datagram >> 24) & 0xff);
    Chip_SSP_WriteFrames_Blocking(LPC_SSP, &i_datagram, 1);
    DEBUG_HFO("%x ", i_datagram);

    i_datagram = (uint8_t)((datagram >> 16) & 0xff);
    Chip_SSP_WriteFrames_Blocking(LPC_SSP, &i_datagram, 1);
    DEBUG_HFO("%x ", i_datagram);

    i_datagram = (uint8_t)((datagram >> 8) & 0xff);
    Chip_SSP_WriteFrames_Blocking(LPC_SSP, &i_datagram, 1);
    DEBUG_HFO("%x ", i_datagram);

    i_datagram = (uint8_t)(datagram & 0xff);
    Chip_SSP_WriteFrames_Blocking(LPC_SSP, &i_datagram, 1);
    DEBUG_HFO("%x \n", i_datagram);

    switch (MotorId)
    {
        case 1:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS1_GPIO_PORT, CS1_GPIO_PIN, true);
            break;

        case 2:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS2_GPIO_PORT, CS2_GPIO_PIN, true);
            break;

        case 3:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS3_GPIO_PORT, CS3_GPIO_PIN, true);
            break;

        case 4:
            Chip_GPIO_WritePortBit(LPC_GPIO_PORT,CS4_GPIO_PORT, CS4_GPIO_PIN, true);
            break;

        default:
            break;
    }

}

TestMotorSPI::TestMotorSPI (void)
{
}