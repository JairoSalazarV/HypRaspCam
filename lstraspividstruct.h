#ifndef LSTRASPIVIDSTRUCT_H
#define LSTRASPIVIDSTRUCT_H

    #include <stdint.h>

    /*
    raspivid Camera App v1.3.12

    Display camera output to display, and optionally saves an H264 capture at requested bitrate


    usage: raspivid [options]

    Image parameter commands

    -?, --help	: This help information
    -w, --width	: Set image width <size>. Default 1920
    -h, --height	: Set image height <size>. Default 1080
    -b, --bitrate	: Set bitrate. Use bits per second (e.g. 10MBits/s would be -b 10000000)
    -o, --output	: Output filename <filename> (to write to stdout, use '-o -')
    -v, --verbose	: Output verbose information during run
    -t, --timeout	: Time (in ms) to capture for. If not specified, set to 5s. Zero to disable
    -d, --demo	: Run a demo mode (cycle through range of camera options, no capture)
    -fps, --framerate	: Specify the frames per second to record
    -e, --penc	: Display preview image *after* encoding (shows compression artifacts)
    -g, --intra	: Specify the intra refresh period (key frame rate/GoP size). Zero to produce an initial I-frame and then just P-frames.
    -pf, --profile	: Specify H264 profile to use for encoding
    -td, --timed	: Cycle between capture and pause. -cycle on,off where on is record time and off is pause time in ms
    -s, --signal	: Cycle between capture and pause on Signal
    -k, --keypress	: Cycle between capture and pause on ENTER
    -i, --initial	: Initial state. Use 'record' or 'pause'. Default 'record'
    -qp, --qp	: Quantisation parameter. Use approximately 10-40. Default 0 (off)
    -ih, --inline	: Insert inline headers (SPS, PPS) to stream
    -sg, --segment	: Segment output file in to multiple files at specified interval <ms>
    -wr, --wrap	: In segment mode, wrap any numbered filename back to 1 when reach number
    -sn, --start	: In segment mode, start with specified segment number
    -sp, --split	: In wait mode, create new output file for each start event
    -c, --circular	: Run encoded data through circular buffer until triggered then save
    -x, --vectors	: Output filename <filename> for inline motion vectors
    -cs, --camselect	: Select camera <number>. Default 0
    -set, --settings	: Retrieve camera settings and write to stdout
    -md, --mode	: Force sensor mode. 0=auto. See docs for other modes available
    -if, --irefresh	: Set intra refresh type
    -fl, --flush	: Flush buffers in order to decrease latency
    -pts, --save-pts	: Save Timestamps to file for mkvmerge
    -cd, --codec	: Specify the codec to use - H264 (default) or MJPEG


    H264 Profile options :
    baseline,main,high


    H264 Intra refresh options :
    cyclic,adaptive,both,cyclicrows

    Preview parameter commands

    -p, --preview	: Preview window settings <'x,y,w,h'>
    -f, --fullscreen	: Fullscreen preview mode
    -op, --opacity	: Preview window opacity (0-255)
    -n, --nopreview	: Do not display a preview window

    Image parameter commands

    -sh, --sharpness	: Set image sharpness (-100 to 100)
    -co, --contrast	: Set image contrast (-100 to 100)
    -br, --brightness	: Set image brightness (0 to 100)
    -sa, --saturation	: Set image saturation (-100 to 100)
    -ISO, --ISO	: Set capture ISO
    -vs, --vstab	: Turn on video stabilisation
    -ev, --ev	: Set EV compensation - steps of 1/6 stop
    -ex, --exposure	: Set exposure mode (see Notes)
    -awb, --awb	: Set AWB mode (see Notes)
    -ifx, --imxfx	: Set image effect (see Notes)
    -cfx, --colfx	: Set colour effect (U:V)
    -mm, --metering	: Set metering mode (see Notes)
    -rot, --rotation	: Set image rotation (0-359)
    -hf, --hflip	: Set horizontal flip
    -vf, --vflip	: Set vertical flip
    -roi, --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
    -ss, --shutter	: Set shutter speed in microseconds
    -awbg, --awbgains	: Set AWB gains - AWB mode must be off
    -drc, --drc	: Set DRC Level
    -st, --stats	: Force recomputation of statistics on stills capture pass
    -a, --annotate	: Enable/Set annotate flags or text
    -3d, --stereo	: Select stereoscopic mode
    -dec, --decimate	: Half width/height of stereo image
    -3dswap, --3dswap	: Swap camera order for stereoscopic
    -ae, --annotateex	: Set extra annotation parameters (text size, text colour(hex YUV), bg colour(hex YUV))


    Notes

    Exposure mode options :
    off,auto,night,nightpreview,backlight,spotlight,sports,snow,beach,verylong,fixedfps,antishake,fireworks

    AWB mode options :
    off,auto,sun,cloud,shade,tungsten,fluorescent,incandescent,flash,horizon

    Image Effect mode options :
    none,negative,solarise,sketch,denoise,emboss,oilpaint,hatch,gpen,pastel,watercolour,film,blur,saturation,colourswap,washedout,posterise,colourpoint,colourbalance,cartoon

    Metering Mode options :
    average,spot,backlit,matrix

    Dynamic Range Compression (DRC) options :
    off,low,med,high

    */


    #define     _H246_PROFILE_baseline                  1
    #define     _H246_PROFILE_main                      2
    #define     _H246_PROFILE_high                      3

    #define     _H264_IntraRefreshOptions_cyclic        1
    #define     _H264_IntraRefreshOptions_adaptive      2
    #define     _H264_IntraRefreshOptions_both          3
    #define     _H264_IntraRefreshOptions_cyclicrows    4

    #define     _PreviewParameterCommands_p             1
    #define     _PreviewParameterCommands_f             2
    #define     _PreviewParameterCommands_op            3
    #define     _PreviewParameterCommands_n             4

    #define     _EXPOSURE_off                           1
    #define     _EXPOSURE_auto                          2
    #define     _EXPOSURE_night                         3
    #define     _EXPOSURE_nightpreview                  4
    #define     _EXPOSURE_backlight                     5
    #define     _EXPOSURE_spotlight                     6
    #define     _EXPOSURE_sports                        7
    #define     _EXPOSURE_snow                          8
    #define     _EXPOSURE_beach                         9
    #define     _EXPOSURE_verylong                      10
    #define     _EXPOSURE_fixedfps                      11
    #define     _EXPOSURE_antishake                     12
    #define     _EXPOSURE_fireworks                     13

    #define     _AWB_off                                1
    #define     _AWB_auto                               2
    #define     _AWB_sun                                3
    #define     _AWB_cloud                              4
    #define     _AWB_shade                              5
    #define     _AWB_tungsten                           6
    #define     _AWB_fluorescent                        7
    #define     _AWB_incandescent                       8
    #define     _AWB_flash                              9
    #define     _AWB_horizon                            10

    #define     _IMAGE_EFFECT_none                      1
    #define     _IMAGE_EFFECT_negative                  2
    #define     _IMAGE_EFFECT_solarise                  3
    #define     _IMAGE_EFFECT_sketch                    4
    #define     _IMAGE_EFFECT_denoise                   5
    #define     _IMAGE_EFFECT_emboss                    6
    #define     _IMAGE_EFFECT_oilpaint                  7
    #define     _IMAGE_EFFECT_hatch                     8
    #define     _IMAGE_EFFECT_gpen                      9
    #define     _IMAGE_EFFECT_pastel                    10
    #define     _IMAGE_EFFECT_watercolour               11
    #define     _IMAGE_EFFECT_film                      12
    #define     _IMAGE_EFFECT_blur                      13
    #define     _IMAGE_EFFECT_saturation                14
    #define     _IMAGE_EFFECT_colourswap                15
    #define     _IMAGE_EFFECT_washedout                 16
    #define     _IMAGE_EFFECT_posterise                 17
    #define     _IMAGE_EFFECT_colourpoint               18
    #define     _IMAGE_EFFECT_colourbalance             19
    #define     _IMAGE_EFFECT_cartoon                   20

    #define     _MATERING_average                       1
    #define     _MATERING_spot                          2
    #define     _MATERING_backlit                       3
    #define     _MATERING_matrix                        4

    #define     _DINAMIC_RANGE_COMPRESSION_off          1
    #define     _DINAMIC_RANGE_COMPRESSION_low          2
    #define     _DINAMIC_RANGE_COMPRESSION_med          3
    #define     _DINAMIC_RANGE_COMPRESSION_high         4



    typedef struct structRaspivid
    {
        char o[30];     // Fileout name
        int16_t w;      // --width	: Set image width <size>. Default 1920
        int16_t h;      // --height	: Set image height <size>. Default 1080
        int     b;      // --bitrate	: Set bitrate. Use bits per second (e.g. 10MBits/s would be -b 10000000)
        int16_t v;      // --verbose	: Output verbose information during run
        int16_t t;      // --timeout	: Time (in ms) to capture for. If not specified, set to 5s. Zero to disable
        int16_t d;      // --demo	: Run a demo mode (cycle through range of camera options, no capture)
        int16_t fps;    // --framerate	: Specify the frames per second to record
        int16_t e;      // --penc	: Display preview image *after* encoding (shows compression artifacts)
        int16_t g;      // --intra	: Specify the intra refresh period (key frame rate/GoP size). Zero to produce an initial I-frame and then just P-frames.
        int16_t pf;     // --profile	: Specify H264 profile to use for encoding
        int16_t td;     // [1,4] --timed	: Cycle between capture and pause. -cycle on,off where on is record time and off is pause time in ms
        int16_t s;      // --signal	: Cycle between capture and pause on Signal
        int16_t k;      // --keypress	: Cycle between capture and pause on ENTER
        char i[6];      // --initial	: Initial state. Use 'record' or 'pause'. Default 'record'
        int16_t qp;     // --qp	: Quantisation parameter. Use approximately 10-40. Default 0 (off)
        int16_t ih;     // --inline	: Insert inline headers (SPS, PPS) to stream
        int16_t sg;     // --segment	: Segment output file in to multiple files at specified interval <ms>
        int16_t wr;     // --wrap	: In segment mode, wrap any numbered filename back to 1 when reach number
        int16_t sn;     // --start	: In segment mode, start with specified segment number
        int16_t sp;     // --split	: In wait mode, create new output file for each start event
        int16_t c;      // --circular	: Run encoded data through circular buffer until triggered then save
        int16_t x;      // --vectors	: Output filename <filename> for inline motion vectors
        int16_t cs;     // --camselect	: Select camera <number>. Default 0
        int16_t set;    // --settings	: Retrieve camera settings and write to stdout
        int16_t md;     // --mode	: Force sensor mode. 0=auto. See docs for other modes available
        int16_t _if;    // --irefresh	: Set intra refresh type
        int16_t fl;     // --flush	: Flush buffers in order to decrease latency
        int16_t pts;    // --save-pts	: Save Timestamps to file for mkvmerge
        char cd[5];     // --codec	: Specify the codec to use - H264 (default) or MJPEG

        //IMAGE PARAMETERS

        int16_t sh;         // --sharpness	: Set image sharpness (-100 to 100)
        int16_t co;         // --contrast	: Set image contrast (-100 to 100)
        int16_t br;         // --brightness	: Set image brightness (0 to 100)
        int16_t sa;         // --saturation	: Set image saturation (-100 to 100)
        int16_t ISO;        // --ISO	: Set capture ISO
        int16_t vs;         // --vstab	: Turn on video stabilisation
        int16_t ev;         // --ev	: Set EV compensation - steps of 1/6 stop
        int16_t ex;         // --exposure	: Set exposure mode (see Notes)
        int16_t awb;        // --awb	: Set AWB mode (see Notes)
        int16_t ifx;        // --imxfx	: Set image effect (see Notes)
        int16_t cfx;        // --colfx	: Set colour effect (U:V)
        int16_t mm;         // --metering	: Set metering mode (see Notes)
        int16_t rot;        // --rotation	: Set image rotation (0-359)
        int16_t hf;         // --hflip	: Set horizontal flip
        int16_t vf;         // --vflip	: Set vertical flip
        float roiX;         // --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
        float roiY;         // --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
        float roiW;         // --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
        float roiD;         // --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
        int16_t ss;         // --shutter	: Set shutter speed in microseconds
        int16_t awbg;       // --awbgains	: Set AWB gains - AWB mode must be off
        int16_t drc;        // --drc	: Set DRC Level
        int16_t st;         // --stats	: Force recomputation of statistics on stills capture pass
        int16_t a;          // --annotate	: Enable/Set annotate flags or text
        int16_t _3d;        // --stereo	: Select stereoscopic mode
        int16_t dec;        // --decimate	: Half width/height of stereo image
        int16_t _3dswap;    // --3dswap	: Swap camera order for stereoscopic
        int16_t ae;         // --annotateex	: Set extra annotation parameters (text size, text colour(hex YUV), bg colour(hex YUV))

    }structRaspivid;



#endif // LSTRASPIVIDSTRUCT_H
