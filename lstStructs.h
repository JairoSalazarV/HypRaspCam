#ifndef LSTSTRUCTS_H
#define LSTSTRUCTS_H

    const unsigned int frameBodyLen = 1024;
    #define FRAME_COMM_LEN 1024;

    typedef struct frameHeader{
        unsigned char idMsg;					// Id instruction
        unsigned int consecutive;	// Consecutive
        unsigned int numTotMsg;		// Total number of message to send
        unsigned int bodyLen;		// Message lenght
    }frameHeader;

    typedef struct frameStruct{
      frameHeader header;
      char msg[frameBodyLen];				// Usable message
    }frameStruct;

    typedef struct camSettings{
      char idMsg;
      char IP[15];
      char Alias[20];
    }camSettings;

    typedef struct structRaspcamSettings{
        //int             width;                  // 1280 to 2592
        //int             height;                 // 960 to 1944
        unsigned char   AWB[20];                // OFF,AUTO,SUNLIGHT,CLOUDY,TUNGSTEN,FLUORESCENT,INCANDESCENT,FLASH,HORIZON
        unsigned char   Exposure[20];           // OFF,AUTO,NIGHT,NIGHTPREVIEW,BACKLIGHT,SPOTLIGHT,SPORTS,SNOW,BEACH,VERYLONG,FIXEDFPS,ANTISHAKE,FIREWORKS
        u_int8_t        Brightness;             // 0 to 100
        int             Sharpness;              // -100 to 100
        int             Contrast;               // -100 to 100
        int             Saturation;             // -100 to 100
        int             ShutterSpeed;           // microsecs (max 330000)
        int             ISO;                    // 100 to 800
        int             ExposureCompensation;   // -10 to 10
        u_int8_t        Format;                 // 1->raspicam::RASPICAM_FORMAT_GRAY | 2->raspicam::RASPICAM_FORMAT_YUV420
        u_int8_t        Red;                    // 0 to 8 set the value for the RED component of white balance
        u_int8_t        Green;                  // 0 to 8 set the value for the GREEN component of white balance
    }structRaspcamSettings;

    typedef struct squareAperture{
        int width;
        int height;
        int x1;//Left-Top = 1,1
        int y1;//Left-Top = 1,1
        int x2;//Left-Top = 1,1
        int y2;//Left-Top = 1,1
    }squareAperture;

    typedef struct strReqImg{
        unsigned char idMsg;
        unsigned char stabSec;
	int imgCols;
        int imgRows;
        bool needCut;
        structRaspcamSettings raspSett;
        squareAperture sqApSett;
    }strReqImg;

    typedef struct structSettings{
        bool setted;
        float version;
        unsigned int inputPort;
        unsigned int outputPort;
        unsigned int tcpPort;
    }structSettings;

    typedef struct structCamSelected{
        bool isConnected;
        bool On;
        bool stream;
        unsigned int tcpPort;
        unsigned char IP[15];
    }structCamSelected;

    typedef struct fileInfo{
        unsigned int fileLen;
        char *rawFile;
    }fileInfo;

    const unsigned int streamLen = sizeof(frameStruct);

#endif //LSTSTRUCTS_H
