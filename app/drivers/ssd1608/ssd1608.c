#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "image.h"
#include "ssd1608.h"
nrf_drv_spi_t SPIM_INSTANCE = NRF_DRV_SPI_INSTANCE(0);
#define SCK_PIN       	11                       /**< SPI clock GPIO pin number. */
#define MOSI_PIN      	10                       /**< SPI Master Out Slave In GPIO pin number. */
#define MISO_PIN      	0 
#define SS0_PIN       	12                      /**< SPI Slave Select GPIO pin number. */
#define DC_PIN     		13
#define RESET_PIN       14
#define BUSY_PIN       	15

// 测试图
#define PIC_WHITE                   255  // 全白
#define PIC_BLACK                   254  // 全黑
#define PIC_Orientation             253  // 方向图
#define PIC_Source_Line             251  // Source线
#define PIC_Gate_Line               250  // Gate线
#define PIC_LEFT_BLACK_RIGHT_WHITE  249  // 左黑右白
#define PIC_UP_BLACK_DOWN_WHITE     248  // 上黑下白

// 用户自定义Demo图
#define PIC_T                         1
#define PIC_P                         2
#define PIC_Z                         3
#define PIC_ESL                       9
#define PIC_COOK                      4

const unsigned char init_data[]={ //30 bytes
    0x50,0xAA,0x55,0xAA,0x55,0xAA,0x11,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,
};

#define TX_RX_MSG_LENGTH         32
uint8_t * gImage_ESL = (uint8_t *)0x30000;
//static uint8_t m_tx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master TX buffer. */
static uint8_t m_rx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master RX buffer. */
static void spi_init()
{
    uint32_t err_code = NRF_SUCCESS;

    // Configure SPI master.
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.sck_pin  = SCK_PIN;
    //spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
    spi_config.mosi_pin = MOSI_PIN;
    spi_config.ss_pin   = SS0_PIN;
    spi_config.irq_priority   = APP_IRQ_PRIORITY_LOW;

    err_code = nrf_drv_spi_init(&SPIM_INSTANCE, &spi_config,NULL);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    //spi_master_evt_handler_reg(SPIM_INSTANCE, spi_master_event_handler);
}
void spi_uninit()
{
    nrf_drv_spi_uninit(&SPIM_INSTANCE);
}
void WriteCommand(uint8_t command)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_clear(DC_PIN);
    nrf_drv_spi_transfer(&SPIM_INSTANCE, &command,1,m_rx_data_spi, 1);
    //nrf_delay_us(100);
    nrf_gpio_pin_set(DC_PIN);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void WriteLongCommand(uint8_t *command,uint8_t length)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_clear(DC_PIN);
    nrf_drv_spi_transfer(&SPIM_INSTANCE, command,length,m_rx_data_spi, length);
    //nrf_delay_us(100);
    nrf_gpio_pin_set(DC_PIN);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void WriteData(uint8_t data)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_set(DC_PIN);
    nrf_drv_spi_transfer(&SPIM_INSTANCE, &data,1,m_rx_data_spi, 1);
    //nrf_delay_us(100);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void WriteLongData(uint8_t *data,uint8_t length)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_set(DC_PIN);
    nrf_drv_spi_transfer(&SPIM_INSTANCE, data,1,m_rx_data_spi, length);
    //nrf_delay_us(100);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void ssd1608_init()
{
    spi_init();
    nrf_gpio_cfg_input(BUSY_PIN,NRF_GPIO_PIN_NOPULL); 
    nrf_gpio_cfg_output(DC_PIN); 
    nrf_gpio_cfg_output(RESET_PIN);
    nrf_gpio_pin_clear(RESET_PIN);
    nrf_delay_ms(10);
    nrf_gpio_pin_set(RESET_PIN);
    nrf_delay_ms(10);
    WriteCommand(0x01); // Gate Setting
    WriteData(0x27); // A[8:0]=0x0127(Set Mux for 295（ 0x0127） +1=296)
    WriteData(0x01); // A[8:0]=0x0127(Set Mux for 295（ 0x0127） +1=296)
    WriteData(0x00); // B[2]:GD=0[POR](G0 is the 1st gate output channel)
    //B[1]:SM=0[POR](left and right gate interlaced)
    //B[0]:TB=0[POR](scan from G0 to G319)
    WriteCommand(0x03); // Gate Driving voltage
    WriteData(0xEA); // A[7:4]:VGH=1110(+22V)[POR] A[3:0]:VGL=1010(-
    //20V)[POR]
    WriteCommand(0x04); // Source Driving voltage
    WriteData(0x0A); // A[3:0]:VSH/VSL=1010(15V)[POR]
    WriteCommand(0x3A); // number of dummy line period set dummy line for 50Hz
    //frame freq
    WriteData(0x00); // A[6:0]=0(Number of dummy line period in term of TGate)

    WriteCommand(0x3B); // Gate line width set gate line for 50Hz frame freq
    WriteData(0x09); // A[3:0]=1001(68us) Line width in us
    //68us*(296+0)=20128us=20.128ms
    WriteCommand(0x3C); // board
    // WriteData(0x30); // GS0-->GS0
    // WriteData(0x31); // GS0-->GS1
    // WriteData(0x32); // GS1-->GS0
    WriteData(0x33); // GS1-->GS1 开机第一次刷新 Border 从白到白 first
    // refresh from start ,border from white to white
    // WriteData(0x43); // VBD-->VSS
    // WriteData(0x53); // VBD-->VSH
    // WriteData(0x63); // VBD-->VSL
    // WriteData(0x73); // VBD-->HiZ
    WriteCommand(0x11); // data enter mode
    WriteData(0x01); // 01 –Y decrement, X increment,
    WriteCommand(0x44); // set RAM x address start/end
    WriteData(0x00); // RAM x address start at 00h;
    WriteData(0x0f); // RAM x address end at 0fh(15+1)*8->128
    WriteCommand(0x45); // set RAM y address start/end
    WriteData(0x27); // RAM y address start at 0127h;
    WriteData(0x01); // RAM y address start at 0127h;
    WriteData(0x00); // RAM y address end at 00h;
    WriteData(0x00); // 高位地址=0
    WriteCommand(0x2C); // vcom
    WriteData(0x5A); //VCOM=-4.2V+5A*0.02=-2.4V (Vcom=-4.2V+[2C 的
    //值]*0.02V)
    // WriteData(0x55); //-2.5V
    // WriteData(0x6E); //-2V
    // WriteData(0x87); //-1.5V
    // WriteData(0xA0); //VCOM=-4.2V+A0*0.02=-1V (Vcom=-4.2V+[2C 的
    //值]*0.02V)
    // WriteData(0xB9); //-0.5V
    WRITE_LUT();
    WriteCommand(0x21); // Option for Display Update
    WriteData(0x83); // A[7]=1(Enable bypass) A[4]=0 全黑(value will be used
    //as for bypass)
    display_image(PIC_ESL); // 全黑到全白清屏，这样可防止开机出现花屏的问题 ful
    WriteCommand(0x21); // Option for Display Update
    WriteData(0x03); // 后面刷新恢复正常的前后 2 幅图比较 rear refresh get
    WriteCommand(0x3C); // board
    WriteData(0x73); // VBD-->HiZ 后面刷新时 Border 都是高阻
    spi_uninit();
}
void WRITE_LUT()
{
    unsigned char i;
    WriteCommand(0x32); // write LUT register
    for(i=0;i<30;i++) // write LUT register
        WriteData(init_data[i]);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xx   图片显示函数    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void display_image(unsigned char num)
{
    unsigned int row, col;
    unsigned int pcnt;

    WriteCommand(0x4E);   // set RAM x address count to 0;
    WriteData(0x00);
    WriteCommand(0x4F);   // set RAM y address count to 296;
    WriteData(0x27);
    WriteData(0x01);

    WriteCommand(0x24);

    pcnt = 0;                     // 复位或保存提示字节序号
    for(col=0; col<296; col++)    // 总共296 GATE
    {
        for(row=0; row<16; row++)   // 总共128 SOURCE，每个像素1bit,即 128/8=16 字节
        {
            switch (num)
            {
                case PIC_WHITE:
                    WriteData(0xff);
                    break;  

                case PIC_BLACK:
                    WriteData(0x00);
                    break;  

                case PIC_Orientation:
                    WriteData(gImage_Orientation[pcnt]);
                    break;  

                case PIC_Gate_Line:
                    if(col%2)
                        WriteData(0xff);  // 奇数Gate行
                    else
                        WriteData(0x00);  // 偶数Gate行
                    break;  

                case PIC_LEFT_BLACK_RIGHT_WHITE:
                    if(col>148)
                        WriteData(0xff);
                    else
                        WriteData(0x00);
                    break;

                case PIC_UP_BLACK_DOWN_WHITE:
                    if(row>8)
                        WriteData(0xff);
                    else
                        WriteData(0x00);
                    break;  

                case PIC_COOK:
                    WriteData(gImage_COOK[pcnt]);
                    break;  

                case PIC_Z:
                    WriteData(gImage_Z[pcnt]);
                    break;  

                case PIC_T:
                    WriteData(gImage_T[pcnt]);
                    break;  

                case PIC_P:
                    WriteData(gImage_P[pcnt]);
                    break;  
                case PIC_ESL:
                    WriteData(gImage_ESL[pcnt]);
                    break;  

                default:
                    break;
            }
            pcnt++;
        }
    }

    WriteCommand(0x22);
    WriteData(0xC7);    // (Enable Clock Signal, Enable CP) (Display update,Disable CP,Disable Clock Signal)
    //    WriteData(0xF7);    // (Enable Clock Signal, Enable CP, Load Temperature value, Load LUT) (Display update,Disable CP,Disable Clock Signal)
    WriteCommand(0x20);
    nrf_delay_ms(1);
    READBUSY();
    /*  
    //For ET update flow
    WriteCommand(0x1A);
    WriteData(0x23);    //Last Temperature Value of WS
    WriteData(0x00);
    WriteCommand(0x22);
    WriteData(0xD7);    //Required 0x1A to define temperature value, then Load LUT from OTP, Display update
    WriteCommand(0x20);
    DELAY_mS(1);
    READBUSY();
    */
}
void READBUSY()
{
    while(1)
    {
        __NOP();
        if((nrf_gpio_pin_read(BUSY_PIN)==0))
            break;
    }      
}

