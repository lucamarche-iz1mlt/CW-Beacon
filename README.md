# CW-Beacon
Create a simple cw beacon with adf4351 and Arduino, it's possible generate frequency fron 35Mhz to 4.4Ghz.

The software, to communicate with ADF4351, uses a software SPI that allows you to use all the pins on the Arduino board.
In the source file it is possible to set all the parameters necessary to configure the transmission of the message

    //////////////////////////////////SETTING///////////////////////////////////

    #define CW_TEXT  "VVV IZ1MLT/B JN44LN"  //Send CW text

    unsigned int speed_wpm    = 12;             //WPM
    unsigned int tone_on      = 10000;          //ms TX after beacon
    unsigned int tone_off     = 2000;           //ms pause before send beacon
    unsigned int time_pause   = 500;            //ms pause after beacon text

    // Freq:  1296,410MHz 
    // RFstep: 10.000kHz
    // Clock in: 10 MHz Ref.
    long int r0       = 0x00409408;
    long int r1       = 0x08001f41;
    long int r2       = 0x18004e42;
    long int r3       = 0x000004B3;
    long int r4       = 0x0015003c;
    long int r5       = 0x00580005;

To configure the frequency it is necessary to enter the values ​​of the registers by hand using the analog device software which you can find at the following link.

https://ez.analog.com/rf/f/q-a/75978/adf4350-and-adf4351-evaluation-board-control-software

![ADF435x](https://github.com/lucamarche-iz1mlt/CW-Beacon/blob/master/img/ADF435xSW.png)
