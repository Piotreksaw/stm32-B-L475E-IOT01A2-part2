#include "ms_ism43362-m3g-l44.h"

#include "main.h"
//#include "stm32l4xx_hal.h"
//#include "stm32l4xx_hal_spi.h"

//#define WIFI_CS_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET)
//#define WIFI_CS_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET)
//#define WIFI_RST_LOW()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET)
//#define WIFI_RST_HIGH() HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET)
//#define WIFI_IS_READY() HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1) // PE1 is Data Ready


#define WIFI_CS_LOW()   HAL_GPIO_WritePin(ISM43362_SPI3_CSN_GPIO_Port, ISM43362_SPI3_CSN_Pin, GPIO_PIN_RESET)
#define WIFI_CS_HIGH()  HAL_GPIO_WritePin(ISM43362_SPI3_CSN_GPIO_Port, ISM43362_SPI3_CSN_Pin, GPIO_PIN_SET)
#define WIFI_RST_LOW()  HAL_GPIO_WritePin(ISM43362_RST_GPIO_Port, ISM43362_RST_Pin, GPIO_PIN_RESET)
#define WIFI_RST_HIGH() HAL_GPIO_WritePin(ISM43362_RST_GPIO_Port, ISM43362_RST_Pin, GPIO_PIN_SET)
#define WIFI_IS_READY() HAL_GPIO_ReadPin(ISM43362_DRDY_EXTI1_GPIO_Port, ISM43362_DRDY_EXTI1_Pin)

extern SPI_HandleTypeDef hspi3;

/* Send/Receive a 16-bit word */
uint16_t SPI_Exchange_16bit(uint16_t data)
{
    uint16_t rx_data = 0;

    // Note: Since we configured SPI to 16-bit, HAL expects uint16_t pointers
    // Ensure SPI3 is in 16-bit mode in hspi3.Init.DataSize
    HAL_SPI_TransmitReceive(&hspi3, (uint8_t*)&data, (uint8_t*)&rx_data, 1, 100);

    return rx_data;
}

void WIFI_SendCommand(const char *cmd)
{
    uint16_t temp_word;
    int len = strlen(cmd);

    /* Wait for Module to be ready to listen (DATARDY check is strictly for Read,
       but we ensure CS is high for a moment before starting) */
    WIFI_CS_HIGH();
    HAL_Delay(1);
    WIFI_CS_LOW();
    HAL_Delay(1); // Tiny delay for setup

    for (int i = 0; i < len; i += 2)
    {
        // Pack two chars into one 16-bit word
        uint8_t char1 = cmd[i];
        uint8_t char2 = (i + 1 < len) ? cmd[i + 1] : 0x0A; // Pad with \n or 0 if odd

        // Combine: Low Byte first, High Byte second
        temp_word = (char2 << 8) | char1;

        SPI_Exchange_16bit(temp_word);
    }

    WIFI_CS_HIGH();
}

void WIFI_ReadResponse(void)
{
    uint16_t rx_word;
    uint8_t c1, c2;

    printf("Waiting for Data Ready...\n");
    // Simple timeout loop
    uint32_t timeout = HAL_GetTick() + 2000;
    while (!WIFI_IS_READY() && HAL_GetTick() < timeout);

    if (WIFI_IS_READY())
    {
        WIFI_CS_LOW();
        printf("Response: ");

        // Read while Data Ready is High
        while (WIFI_IS_READY())
        {
            // Send Dummy 0xFFFF or 0x0000 to clock in data
            rx_word = SPI_Exchange_16bit(0x0A0A);

            // Unpack
            c1 = rx_word & 0xFF;
            c2 = (rx_word >> 8) & 0xFF;

            // Filter out non-printable if needed, or just print
            if(c1) printf("%c", c1);
            if(c2) printf("%c", c2);

            HAL_Delay(1); // Slight throttle
        }
        printf("\n");
        WIFI_CS_HIGH();
    }
    else
    {
        printf("Timeout: No response.\n");
    }
}

void Custom_WIFI_Test(void)
{
    printf("Starting WiFi Low Level Test...\n");

    /* 1. Hardware Reset Sequence */
    WIFI_CS_HIGH();
    WIFI_RST_HIGH();
    HAL_Delay(50);

    // Pulse Reset Low
    WIFI_RST_LOW();
    HAL_Delay(100);
    WIFI_RST_HIGH();

    printf("Module Reset. Waiting for Boot...\n");

    // Wait for the initial "Ready" prompt (Cursor >)
    // The module raises PE1 when it has the boot message ready.
    HAL_Delay(1000);
    WIFI_ReadResponse(); // Should print "Inventek... >"

    /* 2. Send "Get ID" Command */
    // "I?" requests the Model info. "\r" is the terminator.
    printf("Sending I? command...\n");
    WIFI_SendCommand("I?\r");

    // Short delay to allow processing
    HAL_Delay(50);

    /* 3. Read Result */
    WIFI_ReadResponse();
}
