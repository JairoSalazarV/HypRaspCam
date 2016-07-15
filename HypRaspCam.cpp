//Hola
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <streambuf>

#include <ctime>
#include <raspicam/raspicam.h>

#include "lstStructs.h"

#define NUM_THREADS     5

void *sender(void *arg);
void error(const char *msg);
void obtainIP(char* host);
bool sendBigFrame( int newsockfd, std::string bigFrame );
bool sendFile( int newsockfd, std::ifstream &infile );
void funcPrintFirst(int n, int max, char *buffer);
bool getRaspImg(strReqImg *reqImg, const std::string& fileName);
bool openAndSetCamera( strReqImg *reqImg );
unsigned char *funcCropImg(unsigned char* original, int origW, int x1, int y1, int x2, int y2);
std::string file2String( const std::string &fileName );
bool funcSaveFile(char* fileName, char* data, int fileLen, int w, int h);
std::string *genCommand(strReqImg *reqImg, const std::string& fileName);
bool fileExists( const std::string& fileName );

raspicam::RASPICAM_EXPOSURE getExposureFromString ( std::string str );

raspicam::RASPICAM_AWB getAwbFromString ( std::string str ) ;


const unsigned int PORT  = 51717;
//const unsigned int outPORT = 51717;//Mas grande


raspicam::RaspiCam Camera;

int main(int argc, char *argv[])
{
  pthread_t threadReceiver, threadSender;
  int rcReceiver, rcSender;
  int i = 0;
 
  //Define variables
  strReqImg *reqImg = (strReqImg*)malloc(sizeof(strReqImg));
  
  //Obtain the IP address
  char* host = (char*)malloc(NI_MAXHOST * sizeof(char));
  obtainIP(host);

  //Obtains the camera's name
  char camName[] = "BabyFace\0";
  FILE* pipe;
  std::string result;


  //Buffer
  char bufferComm[streamLen];
  frameStruct *tmpFrame 		              = (frameStruct*)malloc(sizeof(frameStruct));
  frameStruct *frame2Send 		            = (frameStruct*)malloc(sizeof(frameStruct));
  frameStruct *frameReceived 	            = (frameStruct*)malloc(sizeof(frameStruct));
  structRaspcamSettings *raspcamSettings  = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));
  structCamSelected *camSelected          = (structCamSelected*)malloc(sizeof(structCamSelected));
  unsigned int tmpFrameLen, headerLen;
  headerLen = sizeof(frameHeader);
  std::string auxFileName;
  

  camSelected->On = false;


  unsigned int tmpTamArch;
  float tmpNumMsgs;

  unsigned int fileLen;
  const unsigned int dataLen = 2592 * 1944 * 3;
  //unsigned char *data = new unsigned char[ dataLen ];
  std::ifstream infile;

  //int sockfd, newsockfd, portno;
  int sockfd, newsockfd;
  socklen_t clilen;
  char buffer[frameBodyLen];
  
  struct sockaddr_in serv_addr, cli_addr;
  int n, aux;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    error("ERROR opening socket");
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  //portno = inPORT;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }
  
  
  
  
  
  
  
  
  
  
  
  /*
  printf("Executing raspistill\n");
  std::string fileName = "./tmpSnapshots/test.RGB888";
  if( getRaspImg(reqImg,fileName) ){
	  printf("Success\n");
	  std::string tmpImg = file2String(fileName);
	  printf("size: %d\n",tmpImg.size());
  }else{
	  printf("Fail\n");
  }
  return 1;
  */
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  

  unsigned int holdon = 1;
  while( holdon ){
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd	 = accept(sockfd, 
               (struct sockaddr *) &cli_addr, 
               &clilen);
    if (newsockfd < 0){
      error("ERROR on accept");
    }
    bzero(buffer,frameBodyLen);
    n = read(newsockfd,buffer,frameBodyLen-1);
    if (n < 0){
      error("ERROR reading from socket");
    }

    //Ordering frame received
    aux = n-headerLen;       
    memcpy(frameReceived,buffer,n);

    //Extract the message and execute instruction identified
    printf("idMessage(%i) n(%i)\n",frameReceived->header.idMsg,n);
    //printf("Message(%c%c)\n",frameReceived->msg[0],frameReceived->msg[1]);
    switch( frameReceived->header.idMsg ){
      //Sending cam settings
      case 1:
        printf("Hand-shaking\n");
        //Send ACK
        camSettings tmpCamSettings;
        tmpCamSettings.idMsg = (char)1;
        memcpy(&tmpCamSettings.IP,host,15);        
        memcpy(&tmpCamSettings.Alias,camName,20);
        n = write(newsockfd,&tmpCamSettings,sizeof(camSettings));
        if (n < 0){
          error("ERROR writing to socket");
        }
        //printf("n(%i)\n", n);
        break;
      //Execute command and send result
      case 2:
        printf("Applying command -> %s\n",frameReceived->msg);

        //Get command result
        result = "";//idMsg to send
        pipe = popen(frameReceived->msg, "r");
        try {
          while (!feof(pipe)) {
            if (fgets(bufferComm, frameBodyLen, pipe) != NULL){
              result.append( bufferComm );
            }
          }
        } catch (...) {
          pclose(pipe);
          throw;
        }
        pclose(pipe);


        //Send message
        printf("\n<%s>\n", result.c_str());
        if( sendBigFrame( newsockfd, result ) ){
          printf("Command result sent\n");
        }else{
          printf("Command result NOT sent\n");
        }
        

        /*
        //Prepare message
        frame2Send->header.idMsg        = (char)2;  
        frame2Send->header.consecutive  = 1;        
        frame2Send->header.numTotMsg    = 1;        
        frame2Send->header.bodyLen      = result.size();
        bzero(frame2Send->msg,frameBodyLen);
        std::copy(result.begin(), result.end(), frame2Send->msg);
        tmpFrameLen = sizeof(frameHeader)+frame2Send->header.bodyLen;
        n = write(newsockfd,frame2Send,tmpFrameLen);        
        if (n < 0){
          error("ERROR writing to socket");
        }
        */
        break;
      //Execute command and omit result (only done ack)
      case 3:

        //Aply message
        printf("Applying command -> %s\n",frameReceived->msg);
        pipe = popen(frameReceived->msg, "r");
        result.assign("done");

        //Send result
        printf("\n<%s>\n", result.c_str());
        if( sendBigFrame( newsockfd, result ) ){
          printf("Command DONE sent\n");
        }else{
          printf("Command DONE NOT sent\n");
        }

        //Prepare message
        /*
        frame2Send->header.idMsg        = (char)2;  
  	    frame2Send->header.consecutive  = 1;        
  	    frame2Send->header.numTotMsg    = 1;        
  	    frame2Send->header.bodyLen      = result.size();
  	    bzero(frame2Send->msg,frameBodyLen);
  	    std::copy(result.begin(), result.end(), frame2Send->msg);
        tmpFrameLen = sizeof(frameHeader)+frame2Send->header.bodyLen;
        n = write(newsockfd,frame2Send,tmpFrameLen);        
        if (n < 0){
          error("ERROR writing to socket");
        }
        */
        break;

    case 4:
    	printf("Image requested\n");
      infile.open("yo.jpg", std::ifstream::binary);
      if( sendFile( newsockfd, infile ) ){
        printf("File sent\n");
      }else{
        printf("File NOT sent\n");
      }
      infile.close();
     	break;
    
    //Camera operations      
    case 5:
      printf("[Camera instruction: %i] ",buffer[1]);
      char camInst[2];
      switch( buffer[1] ){
        case 1:
          printf("camInst: Turn on\n");

          //Get raspCamSettings
          bzero( raspcamSettings, sizeof(structRaspcamSettings) );
          memcpy( raspcamSettings, &buffer[2], sizeof(structRaspcamSettings) );

          //Try to turn on the camera
          //Camera.setWidth( raspcamSettings->width );
          //Camera.setHeight( raspcamSettings->height );
          //printf("Opening Camera W(%i) H(%i)...\n",raspcamSettings->width,raspcamSettings->height);
          if(camSelected->On){
            printf("Camera is actually turn on W(%i) H(%i)\n",Camera.getWidth(),Camera.getHeight());
            camInst[1] = 1; //Turned on successful
          }else{
            if ( Camera.open() ) {
              printf("Sleeping for 3 secs until camera stabilizes\n");
              sleep(3);
              camInst[1] = 1; //Turned on successful
              camSelected->On = true;
              printf("Camera turned on\n");
            }else{
              printf("Error opening camera\n");
              camInst[1] = 0; //Turned on error
              camSelected->On = false;
            }
          }          
          n = write(newsockfd,&camInst,2);          

          /*
          printf("W(%i) H(%i)\n", raspcamSettings->width, raspcamSettings->height );
          printf("AWB(%s) Exposure(%s)\n", raspcamSettings->AWB, raspcamSettings->Exposure );
          printf("Bri(%i) Sharp(%i)\n", raspcamSettings->Brightness, raspcamSettings->Sharpness );
          printf("Contrast(%i) Saturation(%i)\n", raspcamSettings->Contrast, raspcamSettings->Saturation );
          printf("ShutterSpeed(%i) ISO(%i)\n", raspcamSettings->ShutterSpeed, raspcamSettings->ISO );
          printf("ExposureCompensation(%i) Format(%i)\n", raspcamSettings->ExposureCompensation, raspcamSettings->Format );
          printf("Red(%i) Green(%i)\n", raspcamSettings->Red, raspcamSettings->Green );
          */
          break;
        case 2:
          printf("camInst: Turn off\n");
          Camera.release();
          camSelected->On = false;          
          camInst[0] = 0;
          camInst[1] = 1;
          n = write(newsockfd,&camInst,2);
          break;
        case 3:
          printf("camInst: Reset parameters\n");
          //Get raspCamSettings
          bzero( raspcamSettings, sizeof(structRaspcamSettings) );
          memcpy( raspcamSettings, &buffer[2], sizeof(structRaspcamSettings) );
          //Try to turn on the camera
          //Camera.setWidth( raspcamSettings->width );
          //Camera.setHeight( raspcamSettings->height );
          //Send new imageSize
          camInst[0] = 1;
          camInst[1] = 1;
          n = write(newsockfd,&camInst,2);
          printf("Reseted camera parameters...\n");
          break;
        default:
          printf("camInst: Default\n");
          camInst[0] = 0;
          camInst[1] = 0;
          n = write(newsockfd,&camInst,2);
          break;
      }
      break;

    //Send image from camera
    case 6:
      printf("Image requested from camera\n");
      //Sending image
      if( camSelected->On ){//Camera is turn on
        //Get image        
        Camera.grab();
        fileLen = Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB );
        unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];        
        Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_IGNORE );//get camera image
        if( buffer[1] == 2 ){
          
          printf("Saving image in HypCam\n");

          //Take file name
          char tmpFileName[16+17];
          snprintf(tmpFileName, 16+17, "tmpSnapshots/%lu.ppm", time(NULL));

          //Save file
          std::ofstream outFile ( tmpFileName, std::ios::binary );
          outFile<<"P6\n"<<Camera.getWidth()<<" "<<Camera.getHeight()<<" 255\n";
          outFile.write((char*)data,fileLen);
          outFile.close();
          camInst[0] = 1;
          camInst[1] = 1;
          n = write(newsockfd,&camInst,2);
        }else{
          //Send image as frame
          std::string tmpImg( data, data+fileLen );
          delete data;
          printf("Sending image\n");
          if( sendBigFrame( newsockfd, tmpImg ) ){
            printf("Photogram sent\n");
          }else{
            printf("ERROR sending photo\n");
          }
        }
      }else{
        //Camera is turn off            
        printf("ERROR: Camera is off\n");
        fileLen = 0;
        write(newsockfd,&fileLen,sizeof(unsigned int));
        break;
      }
 
      break;

    //Require image from scratch
    //..
    case 7:
	//Prepare memory
	memset( reqImg, '\0', sizeof(strReqImg)  );		
	memcpy( reqImg, frameReceived, sizeof(strReqImg) );

	//Send ACK with camera status
	//buffer[1] = ( openAndSetCamera( reqImg  ) )?1:0;
	buffer[1] = 1;
	write(newsockfd,&buffer,2);
	if( buffer[1] == 0 )break;
	else{ 
		//Get image
		//..
		//Camera.grab();
		//int fileLen = Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB );
		//unsigned char *data=new unsigned char[ fileLen ];
		//Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_IGNORE );//get camera image		
		//char tmpFileNameNot[] = "./tmpSnapshots/notCropped.ppm";
		//funcSaveFile(tmpFileNameNot,(char*)data, fileLen, reqImg->imgCols, reqImg->imgRows);
		
		printf("Making the snapshot by applying raspistill\n");
		std::string fileName = "./tmpSnapshots/tmpImg.RGB888";
		if( getRaspImg(reqImg,fileName) ){
		  printf("Snapshot [OK]\n");
		}else{
		  printf("Snapshot[Fail]\n");
		  break;
		}

		
		
		//Send image as frame
		//..
		//std::ifstream fileRead(fileName.c_str());
		//std::string tmpImg(
		//					std::istreambuf_iterator<char>(fileRead),
		//					std::istreambuf_iterator<char>()
		//				  );
		std::string tmpImg = file2String(fileName);
		printf("size: %d\n",tmpImg.size());
		
		
		
		//std::string tmpImg(data,data+fileLen);//Begin -to- End		
		//printf("Sending image: \n", tmpImg.size());
		if( sendBigFrame( newsockfd, tmpImg ) ){
			printf("Photogram sent\n");
		}else{
			printf("ERROR sending photo\n");
		}		

		//Turn of the camera and release memo
		//..
		//delete[] data;
		//Camera.release();
	}
	

    	break;


    //Unrecognized instruction
    default:
      n = write(newsockfd,"Default",8);
      if (n < 0){
        error("ERROR writing to socket");
      }
      holdon = 0;
      break;

    }

    
  }

  close(newsockfd);
  close(sockfd);
  printf("It finishes...\n");
  return 0;
}

std::string file2String( const std::string &fileName ){
	std::ifstream t(fileName.c_str());
	std::string str;
	t.seekg(0,std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0,std::ios::beg);
	str.assign(
					(std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>()
			  );
	return str;
}

bool funcSaveFile(char* fileName, char* data, int fileLen, int w, int h){
	std::ofstream outFile( fileName,std::ios::binary );
	outFile<<"P6\n"<<w<<" "<<h<<" 255\n";
	outFile.write ( data, fileLen  );
	return true;
}

unsigned char *funcCropImg(unsigned char* original, int origW, int x1, int y1, int x2, int y2){
	//Crop image
	int lastW = x2-x1;
	int lastH = y2-y1;
	unsigned char *lastImg = new unsigned char[(lastW*lastH*3)];
	int r, aux;
	for( r=0; r<lastH; r++ ){
		aux = ((y1-1+r)*origW*3)+(x1*3);
		memcpy( &lastImg[r*lastW*3], &original[aux],(lastW*3));
	}

	//save
	//..
	printf("lastW(%d) lastH(%d)\n",lastW,lastH);
	char tmpFileName[] = "./tmpSnapshots/crop.ppm";
	funcSaveFile(tmpFileName,(char*)lastImg, (lastW*lastH*3), lastW, lastH);
	return lastImg;
}


bool getRaspImg(strReqImg *reqImg, const std::string &fileName){
	
	//Concatenate raspistill command
	//raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888 -t 8000 -ss 1000000 -roi x,y,w,d 
	//..	
	std::string *raspistillCommand = genCommand(reqImg, fileName);
	printf("Comm: %s\n",raspistillCommand->c_str());
	//Prepare command as required
	//char *tmpComm = new char[raspistillCommand->size()+1];
	//std::copy(raspistillCommand->begin(), raspistillCommand->end(),tmpComm);
	//tmpComm[raspistillCommand->size()] = '\0';
	
	//Execute raspistill
	//..
	//Remove file if exists	
	if(fileExists(fileName)){
		remove(fileName.c_str());	
	}
	//Execute raspistill
	FILE* pipe;
	//pipe = popen(tmpComm, "r");
	pipe = popen(raspistillCommand->c_str(), "r");
	pclose(pipe);
	//delete[] tmpComm;
	
	//Verify if it was created the snapshot
	//..
	if(!fileExists(fileName)){
		return false;
	}
	
	
	return true;
		
}

std::string *genCommand(strReqImg *reqImg, const std::string& fileName){
	
	//Initialize command
	//..
	std::string *tmpCommand = new std::string("raspistill -o ");
	std::ostringstream ss;
	tmpCommand->append(fileName);
	tmpCommand->append(" -n -q 100 -gc");
	//Colour balance?
	if(reqImg->raspSett.ColorBalance){
		tmpCommand->append(" -ifx colourbalance");
	}
	//Denoise?
	if(reqImg->raspSett.Denoise){
		tmpCommand->append(" -ifx denoise");
	}
	//Width
	ss.str("");
	ss<<reqImg->imgCols;
	tmpCommand->append(" -w " + ss.str());
	//Height
	ss.str("");
	ss<<reqImg->imgRows;
	tmpCommand->append(" -h " + ss.str());
	//Shuter speed
	ss.str("");
	ss<<reqImg->raspSett.ShutterSpeed;
	tmpCommand->append(" -ss " + ss.str());
	//Trigering timer
	ss.str("");
	ss<<(reqImg->raspSett.TriggerTime*1000);
	tmpCommand->append(" -t " + ss.str());
	
	//Crop image if neccesary
	//..
	if( reqImg->needCut ){
		float x,y,w,d;	
		//Normalization
		x = (float)reqImg->sqApSett.rectX / (float)_BIG_WIDTH;
		y = (float)reqImg->sqApSett.rectY / (float)_BIG_HEIGHT;
		w = (float)reqImg->imgCols / (float)_BIG_WIDTH;
		d = (float)reqImg->imgRows / (float)_BIG_HEIGHT;
		//Add region of interes to the raspistill command
		tmpCommand->append(" -roi ");		
		ss.str("");
		ss<<x;
		tmpCommand->append(ss.str()+",");
		ss.str("");
		ss<<y;
		tmpCommand->append(ss.str()+",");
		ss.str("");
		ss<<w;
		tmpCommand->append(ss.str()+","	);
		ss.str("");
		ss<<d;
		tmpCommand->append(ss.str());
	}
	
	//AWB
	if(strcmp((char*)reqImg->raspSett.AWB, "none")!=0){
		std::string sAWB((char*)reqImg->raspSett.AWB, sizeof(reqImg->raspSett.AWB));
		tmpCommand->append(" -awb ");
		tmpCommand->append(sAWB.c_str());
		//printf("Entro a AWB: %s\n",sAWB.c_str());
	}
	
	//Exposure
	if(strcmp((char*)reqImg->raspSett.Exposure, "none")!=0){
		std::string sExposure((char*)reqImg->raspSett.Exposure, sizeof(reqImg->raspSett.Exposure));
		tmpCommand->append(" -ex ");
		tmpCommand->append(sExposure.c_str());
		//printf("Entro a Exp: %s\n",sExposure.c_str());
	}
	
	return tmpCommand;
}

bool fileExists( const std::string& fileName ){
	struct stat buffer;
	return( stat(fileName.c_str(), &buffer) == 0 );
}


bool openAndSetCamera( strReqImg *reqImg ){
	//Set camera
	Camera.setWidth( reqImg->imgCols );
	Camera.setHeight( reqImg->imgRows );
	Camera.setBrightness ( reqImg->raspSett.Brightness );
	Camera.setSharpness ( reqImg->raspSett.Sharpness );
	Camera.setContrast ( reqImg->raspSett.Contrast );
	Camera.setSaturation ( reqImg->raspSett.Saturation );
	Camera.setShutterSpeed(reqImg->raspSett.ShutterSpeed );
	Camera.setISO (reqImg->raspSett.ISO );
	Camera.setVideoStabilization ( true );
	Camera.setExposureCompensation ( reqImg->raspSett.ExposureCompensation);
	//Camera.setFormat(reqImg->raspSett.Format);
	Camera.setAWB_RB(reqImg->raspSett.Red, reqImg->raspSett.Green );

	std::string tmpExposure;
	std::string tmpAWB;
	tmpExposure.assign( (char*)reqImg->raspSett.Exposure );
	tmpAWB.assign( (char*)reqImg->raspSett.AWB );
	Camera.setExposure ( getExposureFromString (tmpExposure ) );
	Camera.setAWB( getAwbFromString ( tmpAWB ) );
	


	//Turn on camera pre-setted
	if ( Camera.open() ) {
		sleep(reqImg->stabSec);
	}else{
		printf("ERROR openning camera\n");
		return false;
	}   
	return true;
}

bool sendBigFrame( int newsockfd, std::string bigFrame ){
  //printf("Inside sendBigFrame\n");

  //get file properties
  std::string result;
  frameStruct *frame2Send = (frameStruct *)malloc(sizeof(frameStruct));
  //infile.open("yo.jpg", std::ifstream::binary);
  //infile.seekg (0,infile.end);
  unsigned int fileLen;
  char buffer[frameBodyLen];
  int i, n, aux;

  //printf("Len sent\n");
  fileLen = bigFrame.size();
  //infile.seekg (0);
  if( fileLen < 1 ){
    //Prepare message
    result.assign("Error reading file");
    frame2Send->header.idMsg        = (char)3;  
    frame2Send->header.consecutive  = 1;
    frame2Send->header.bodyLen      = result.size();
    bzero(frame2Send->msg,frameBodyLen);        
    std::copy(result.begin(), result.end(), frame2Send->msg);
    unsigned int tmpFrameLen = sizeof(frameHeader)+frame2Send->header.bodyLen;
    n = write(newsockfd,frame2Send,tmpFrameLen);        
    if (n < 0){
      error("ERROR writing to socket");
    }
  }else{

    //Send Len
    //printf("fileLen: %i\n", fileLen);
    n = write(newsockfd,&fileLen,sizeof(unsigned int));
    if (n < 0){
      error("ERROR writing to socket");      
    }
    //printf("Len sent\n");

    //Get file request
    //printf("Reading\n");
    n = read(newsockfd, buffer, frameBodyLen-1);
    if (n < 0){
      error("ERROR reading from socket");
    }
    //printf("File requested\n");

    //Send file
    aux = floor( (float)fileLen / (float)frameBodyLen );
    aux = ((aux*frameBodyLen)<fileLen)?aux+1:aux;
    printf("numMsgs: %i - frameBodyLen: \n",aux,frameBodyLen);
    for(i=1; i<=aux; i++){
      //Send part of the file
      bzero(buffer,frameBodyLen);
      memcpy( buffer, &bigFrame[(i-1)*frameBodyLen], frameBodyLen );
      n = write(newsockfd,buffer,frameBodyLen);
      if (n < 0){
        error("ERROR writing to socket");
      }
      //Get next part requesst
      n = read(newsockfd, buffer, frameBodyLen-1);
      if (n < 0){
        error("ERROR reading from socket");
      }
    }
  }
  //infile.close();

  //printf("bigFrame finish\n");
  return true;
}


bool sendFile( int newsockfd, std::ifstream &infile ){
  //printf("Image requested inside\n");

  //get file properties
  std::string result;
  frameStruct *frame2Send = (frameStruct *)malloc(sizeof(frameStruct));
  //infile.open("yo.jpg", std::ifstream::binary);
  infile.seekg (0,infile.end);
  unsigned int fileLen;
  char buffer[frameBodyLen];
  int i, n, aux;

  //printf("Len sent\n");
  fileLen = infile.tellg();
  infile.seekg (0);
  if( fileLen < 1 ){
    //Prepare message
    result.assign("Error reading file");
    frame2Send->header.idMsg        = (char)3;  
    frame2Send->header.consecutive  = 1;
    frame2Send->header.bodyLen      = result.size();
    bzero(frame2Send->msg,frameBodyLen);        
    std::copy(result.begin(), result.end(), frame2Send->msg);
    unsigned int tmpFrameLen = sizeof(frameHeader)+frame2Send->header.bodyLen;
    n = write(newsockfd,frame2Send,tmpFrameLen);        
    if (n < 0){
      error("ERROR writing to socket");
    }
  }else{

    //Send Len
    n = write(newsockfd,&fileLen,sizeof(unsigned int));
    if (n < 0){
      error("ERROR writing to socket");
    }
    //printf("Len sent\n");

    //Get file request
    n = read(newsockfd, buffer, frameBodyLen-1);
    if (n < 0){
      error("ERROR reading from socket");
    }
    //printf("File requested\n");

    //Send file
    aux = floor( (float)fileLen / (float)frameBodyLen );
    for(i=1; i<=aux; i++){
      //Send part of the file
      bzero(buffer,frameBodyLen);
      infile.read(buffer,frameBodyLen);

      //funcPrintFirst(3,frameBodyLen,buffer);



      n = write(newsockfd,buffer,frameBodyLen);
      if (n < 0){
        error("ERROR writing to socket");
      }
      //printf("File part sent(%i)\n",i);

      //Get next part requesst
      n = read(newsockfd, buffer, frameBodyLen-1);
      if (n < 0){
        error("ERROR reading from socket");
      }

      //printf("File part Requested(%i)\n",i);
    }
    //Send last part of the file
    aux = fileLen - (aux*frameBodyLen);
    if( aux > 0 ){          
      bzero(buffer,frameBodyLen);
      infile.read(buffer,aux);
      n = write(newsockfd,buffer,aux);
      if (n < 0){
        error("ERROR writing to socket");
      }
      //printf("Last part sent i(%i) aux(%i) \n",i,aux);
    }else{
      //printf("Last part does not exist aux(%i) \n",aux);
    }
    //printf("Image sent\n");

  }
  //infile.close();

  return true;
}









void funcPrintFirst(int n, int max, char *buffer){
  int i;
  printf("First: ");
  for(i=0;i<n;i++){
    printf(" %i ", (int)buffer[i]);
  }
  printf("\nLast: ");
  for(i=max-1;i>=max-n;i--){
    printf(" %i ", (int)buffer[i]);
  }
  printf("\n\n");
}

void *sender(void *arg){
  printf("Sender\n");
  return NULL;
}
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void obtainIP(char* host)
{
    FILE *f;
    char line[100] , *p , *c;
     
    f = fopen("/proc/net/route" , "r");
     
    while(fgets(line , 100 , f))
    {
        p = strtok(line , " \t");
        c = strtok(NULL , " \t");
         
        if(p!=NULL && c!=NULL)
        {
            if(strcmp(c , "00000000") == 0)
            {
                //printf("Default interface is : %s \n" , p);
                break;
            }
        }
    }
     
    //which family do we require , AF_INET or AF_INET6
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
    int family , s;
    //char host[NI_MAXHOST];
    
 
    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
 
    //Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }
 
        family = ifa->ifa_addr->sa_family;
 
        if(strcmp( ifa->ifa_name , p) == 0)
        {
            if (family == fm) 
            {
                s = getnameinfo( ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6) , host , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
                 
                if (s != 0) 
                {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }
                 
                //printf("address: %s", host);
            }
            printf("\n");
        }
    }
 
    freeifaddrs(ifaddr);

}



raspicam::RASPICAM_EXPOSURE getExposureFromString ( std::string str ) {
  if ( str=="OFF" ) return raspicam::RASPICAM_EXPOSURE_OFF;
  if ( str=="AUTO" ) return raspicam::RASPICAM_EXPOSURE_AUTO;
  if ( str=="NIGHT" ) return raspicam::RASPICAM_EXPOSURE_NIGHT;
  if ( str=="NIGHTPREVIEW" ) return raspicam::RASPICAM_EXPOSURE_NIGHTPREVIEW;
  if ( str=="BACKLIGHT" ) return raspicam::RASPICAM_EXPOSURE_BACKLIGHT;
  if ( str=="SPOTLIGHT" ) return raspicam::RASPICAM_EXPOSURE_SPOTLIGHT;
  if ( str=="SPORTS" ) return raspicam::RASPICAM_EXPOSURE_SPORTS;
  if ( str=="SNOW" ) return raspicam::RASPICAM_EXPOSURE_SNOW;
  if ( str=="BEACH" ) return raspicam::RASPICAM_EXPOSURE_BEACH;
  if ( str=="VERYLONG" ) return raspicam::RASPICAM_EXPOSURE_VERYLONG;
  if ( str=="FIXEDFPS" ) return raspicam::RASPICAM_EXPOSURE_FIXEDFPS;
  if ( str=="ANTISHAKE" ) return raspicam::RASPICAM_EXPOSURE_ANTISHAKE;
  if ( str=="FIREWORKS" ) return raspicam::RASPICAM_EXPOSURE_FIREWORKS;
  return raspicam::RASPICAM_EXPOSURE_AUTO;
}

raspicam::RASPICAM_AWB getAwbFromString ( std::string str ) {
  if ( str=="OFF" ) return raspicam::RASPICAM_AWB_OFF;
  if ( str=="AUTO" ) return raspicam::RASPICAM_AWB_AUTO;
  if ( str=="SUNLIGHT" ) return raspicam::RASPICAM_AWB_SUNLIGHT;
  if ( str=="CLOUDY" ) return raspicam::RASPICAM_AWB_CLOUDY;
  if ( str=="SHADE" ) return raspicam::RASPICAM_AWB_SHADE;
  if ( str=="TUNGSTEN" ) return raspicam::RASPICAM_AWB_TUNGSTEN;
  if ( str=="FLUORESCENT" ) return raspicam::RASPICAM_AWB_FLUORESCENT;
  if ( str=="INCANDESCENT" ) return raspicam::RASPICAM_AWB_INCANDESCENT;
  if ( str=="FLASH" ) return raspicam::RASPICAM_AWB_FLASH;
  if ( str=="HORIZON" ) return raspicam::RASPICAM_AWB_HORIZON;
  return raspicam::RASPICAM_AWB_AUTO;
}
