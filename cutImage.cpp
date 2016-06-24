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
bool openAndSetCamera( strReqImg *reqImg );

raspicam::RASPICAM_EXPOSURE getExposureFromString ( std::string str );

raspicam::RASPICAM_AWB getAwbFromString ( std::string str ) ;


const unsigned int PORT  = 51717;
//const unsigned int outPORT = 51717;//Mas grande


raspicam::RaspiCam Camera;

int main(int argc, char *argv[])
{

	int W,H,x1,y1,x2,y2;
	if(false){
		W = 640;
		H = 480;
		x1 = 195;
		y1 = 128;
		x2 = 475;
		y2 = 403;
	}else{
		W = 2592;
		H = 1944;
		x1 = 317;
		y1 = 239;
		x2 = 357;
		y2 = 280;
	}

	

	//Get image
	Camera.setWidth( W );
	Camera.setHeight( H );
	Camera.open();
	sleep(1);
        Camera.grab();
        unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];        
        Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_IGNORE );//get camera image

	//Crop image
	int lastW = x2-x1;
	int lastH = y2-y1;
	//printf("Last(%d,%d)",lastW,lastH);
	unsigned char *lastImg = new unsigned char[(lastW*lastH*3)];
	int r, aux;	
	for( r=0; r<lastH; r++ ){
		aux = ((y1-1+r)*W*3)+(x1*3);
		memcpy( &lastImg[r*lastW*3], &data[aux],(lastW*3));
	}

	//save
	std::ofstream outFile ( "croped.ppm",std::ios::binary );
    	outFile<<"P6\n"<<lastW<<" "<<lastH <<" 255\n";
    	outFile.write ( ( char* ) lastImg, lastW*lastH*3  );
	
	
	std::ofstream outFile2 ( "notCroped.ppm",std::ios::binary );
    	outFile2<<"P6\n"<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    	outFile2.write ( ( char* ) data, Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB ) );
	
	Camera.release();

}

bool openAndSetCamera( strReqImg *reqImg ){
	//Set camera
	Camera.setWidth( reqImg->imgCols );
	Camera.setHeight( reqImg->imgRows );

	//Turn on camera pre-setted
	if ( Camera.open() ) {
		sleep(reqImg->stabSec);
	}else{
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
    //printf("numMsgs %i\n",aux);
    for(i=1; i<=aux; i++){
      //Send part of the file
      bzero(buffer,frameBodyLen);
      memcpy( buffer, &bigFrame[(i-1)*frameBodyLen], frameBodyLen );
      //infile.read(buffer,frameBodyLen);


      //funcPrintFirst(3,frameBodyLen,buffer);


      //printf("Writing\n");
      n = write(newsockfd,buffer,frameBodyLen);
      if (n < 0){
        error("ERROR writing to socket");
      }
      //printf("File part sent(%i)\n",i);

      //Get next part requesst
      //printf("reading\n");
      n = read(newsockfd, buffer, frameBodyLen-1);
      if (n < 0){
        error("ERROR reading from socket");
      }

      //printf("File part Requested(%i)\n",i);
    }
    //Send last part of the file
    if( aux==0 ){

        bzero(buffer,frameBodyLen);
        memcpy( buffer, &bigFrame[0], fileLen );
        //printf("writing\n");
        n = write(newsockfd,buffer,fileLen);
        if (n < 0){
          error("ERROR writing to socket");
        }

    }else{
      aux = fileLen - (aux*frameBodyLen);
      if( aux > 0 ){          
        bzero(buffer,frameBodyLen);
        memcpy( buffer, &bigFrame[i*frameBodyLen], aux );
        //infile.read(buffer,aux);
        //printf("writing\n");
        n = write(newsockfd,buffer,aux);
        if (n < 0){
          error("ERROR writing to socket");
        }
        //printf("Last part sent i(%i) aux(%i) \n",i,aux);
      }else{
        //printf("Last part does not exist aux(%i) \n",aux);
      }
    }
    //printf("Image sent\n");

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
