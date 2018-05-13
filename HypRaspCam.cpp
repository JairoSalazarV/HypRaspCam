//11 August 2016 18:30
//27 August 2016	morning
//20 Marzo 2017: 	- Se agregó la validación de mensajes
//				 	- Lectura de imágenes y recorte de subimágenes
//9 Mayo 2017 		Se agregó la opción 12 para grabar y envíar vídeos
//16 Mayo 2017 		Se agregó el control del motor al tomar imagen Slide
//					Se agregó Puerto y USB como argumento de inicio
//27 Mayo 2017 		Se agrego la opción 4 para tomar foto y devolver status
//09 Junio 2017		Se junto lo que se tenía con la opción de square aperture
// 					y se optimizó la transmisión de datos
//10 Octubre 2017	Se actualizó github


//
// HELP
//
// (ERROR on binding: Address already in use) -> netstat -tulpn -> kill process
// Para saber el USB -> ls -l /dev/tty (tab)

#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
//#include <dir.h> 
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

//#include <fstream>
//#include <iostream>

#include <ctime>

#include "lstStructs.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define NUM_THREADS     5
#define		_OK			1
#define		_ERROR		0
#define		_FAIL		2

using namespace std;


void *sender(void *arg);
void error(const char *msg);
void obtainIP(char* host);
bool sendBigFrame( int newsockfd, std::string bigFrame );
void funcMotorDoAWalk( strReqImg *reqImg );
void recordVideo( strReqImg *reqImg );
bool sendFile( int newsockfd, std::ifstream &infile );
void funcPrintFirst(int n, int max, char *buffer);
bool getRaspImg(strReqImg *reqImg, const std::string& fileName);
unsigned char *funcCropImg(unsigned char* original, int origW, int x1, int y1, int x2, int y2);
std::string file2String( const std::string &fileName );
bool funcSaveFile(char* fileName, char* data, int fileLen, int w, int h);
std::string *genSLIDECommand(strReqImg *reqImg);
std::string *genRaspiVideoCommand(strReqImg *reqImg);
std::string *genCommand(strReqImg *reqImg, const std::string& fileName);
bool reqImgIsValid( strReqImg *reqImg );
int fileExists( const std::string& fileName );
int applyTimeLapseUsingRaspistill(strReqImg *reqImg);
//bool funcFolderExists( std::string folderPath );
bool funcPathExists( std::string fileName );
bool funcCreateFolder( std::string folderName );
bool funcDeleteFolderRecursively( std::string folderName );
bool funcClearFolder( std::string folderName );
void startTimLapse(int newsockfd, strReqImg *reqImg);
int startGenerateSlideCube(int newsockfd, strReqImg *reqImg);
uint8_t* subimage( int x1, int y1, int croppedRows, int croppedCols, uint8_t* img, int imgRows, int imgCols );
bool cropAndTransmitSlide( std::string* tmpImgName, int newsockfd, strReqImg *reqImg );
int funcGenericSendFrame( int newsockfd, u_int8_t* frame, int lenToSend );
int funcSendOneMessage( int newsockfd, void* frame, int lenToSend );
strReqSubframe* readSubframeRequested( int newsockfd, strReqSubframe* subframeReq );
int funcSendERROR( int newsockfd, u_int8_t ID );
int funcSendACK( int newsockfd, u_int8_t ID );
void funcMessage( std::string msg );
int checkIfRequestedFileExists( int sockfd, strReqFileInfo* reqFileInfo );
int sendRequestedFile( int sockfd, strReqFileInfo* reqFileInfo );
int saveBinFile_From_u_int8_T( std::string fileName, uint8_t *data, size_t len);
int deleteFileIfExists( const char* fileName );

int readFileContain( const string &fileName, string* contain );

unsigned int PORT;
std::string SERIAL_PORT;
//const unsigned int PORT  = 51717;
//const unsigned int outPORT = 51717;//Mas grande

int main(int argc, char *argv[])
{
	int aux;
	
	//
	//Set variables received from aguments
	//
	if( argc < 2 || argc > 3 )
	{	
		printf("Usage 1: ./HypRaspCam <Port>\n");
		printf("Usage 2: ./HypRaspCam <Port> </dev/ttyUSBX>\n");
		printf("Example: ./HypRaspCam 51717 /dev/ttyUSB0\n");
		fflush(stdout);
		return -1;
	}	
	
	//PORT
	std::string tmpPort;
	tmpPort = "";
	tmpPort.append(argv[1]);
	std::istringstream(tmpPort) >> PORT;	
	
	if( argc == 3 )
	{
		//ttyUSBX
		SERIAL_PORT = "";
		SERIAL_PORT.append(argv[2]);
	}
	
	
	
	//pthread_t threadReceiver, threadSender;
	//int rcReceiver, rcSender;
	//int i = 0;

	//Define variables
	strReqImg *reqImg 			= (strReqImg*)malloc(sizeof(strReqImg));
	strReqFileInfo* reqFileInfo = (strReqFileInfo*)malloc(sizeof(strReqFileInfo));
	//strReqImg *reqImgSqu 	= (strReqImg*)malloc(sizeof(strReqImg));

	//Obtain the IP address
	char* host = (char*)malloc(NI_MAXHOST * sizeof(char));
	obtainIP(host);

	//Obtains the camera's name
	char camName[] = "IRis\0";
	FILE* pipe;
	std::string result;

	//Buffer
	char bufferComm[streamLen];
	//frameStruct *tmpFrame 		          		= (frameStruct*)malloc(sizeof(frameStruct));
	//frameStruct *frame2Send 		          	= (frameStruct*)malloc(sizeof(frameStruct));
	frameStruct *frameReceived 	          		= (frameStruct*)malloc(sizeof(frameStruct));
	structRaspistillCommand* raspistillCommand 	= (structRaspistillCommand*)malloc(sizeof(structRaspistillCommand));
	//structRaspcamSettings *raspcamSettings  	= (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));
	structCamSelected *camSelected          	= (structCamSelected*)malloc(sizeof(structCamSelected));
	//structSubimage *strSubimage       			= (structSubimage*)malloc(sizeof(structSubimage));
	
	//unsigned int tmpFrameLen, headerLen;
	unsigned int headerLen;
	headerLen = sizeof(frameHeader);
	std::string auxFileName;


	camSelected->On = false;


	//unsigned int tmpTamArch;
	//float tmpNumMsgs;

	//unsigned int fileLen;
	//const unsigned int dataLen = 2592 * 1944 * 3;
	//unsigned char *data = new unsigned char[ dataLen ];
	std::ifstream infile;

	//int sockfd, newsockfd, portno;
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[frameBodyLen];

	struct sockaddr_in serv_addr, cli_addr;
	//int n, aux, status;
	int n;
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
  
	if( argc == 3 )
	{
		//
		//SERIAL PORT INITIALIZE
		//
		// sudo chmod o+rw /dev/ttyUSBX 						// Permissions to the Serial port
		// stty -F /dev/ttyUSBX speed 9600 cs8 -cstopb -parenb	// Set speed
		// tail -f /dev/ttyUSBX &								// Allow arduino to receive messages
		// echo 111111111111111111 > /dev/ttyUSB0				// Message example	
		printf("Initializing Serila Port\n");
		std::string tmpCommand;

		// Permissions to the Serial port
		tmpCommand = "sudo chmod o+rw ";
		tmpCommand.append(SERIAL_PORT);
		//printf("Serial Port: %s\n", tmpCommand.c_str());
		pipe = popen(tmpCommand.c_str(), "r");

		// Set speed
		tmpCommand = "stty -F ";
		tmpCommand.append(SERIAL_PORT);
		tmpCommand.append(" speed 9600 cs8 -cstopb -parenb");
		//printf("Serial Port: %s\n", tmpCommand.c_str());
		pipe = popen(tmpCommand.c_str(), "r");

		//Allow arduino to receive messages
		tmpCommand = "tail -f ";
		tmpCommand.append(SERIAL_PORT);
		tmpCommand.append(" &");
		//printf("Serial Port: %s\n", tmpCommand.c_str());
		pipe = popen(tmpCommand.c_str(), "r");


		fflush(stdout);
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
  
  
  
  
  
  
  
  
  
  
  
  

  
	//WELCOME
	printf("Welcome!!!\n\n\n");
	fflush(stdout);
  
  

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
    printf("\nidMessage(%i) n(%i)",frameReceived->header.idMsg,n);
    //printf("Message(%c%c)\n",frameReceived->msg[0],frameReceived->msg[1]);
    printf("\n");
    switch( frameReceived->header.idMsg ){
	  //
      //Sending cam settings
      //
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
      //
      //Execute command and send result
      //
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
        result = (result.size()>1)?result:"  ";
        //printf("\n<%s>\n", result.c_str());
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
      //
      //Execute command and omit result (only send ack)
      //
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
	//
	//  1) Delete last image
	//	2) Excecute command
	//	3) Send ACK; 0:error | 1:done
	//
    case 4:
		
		//Extracts Command Structure		
		memset( raspistillCommand, '\0', sizeof(structRaspistillCommand)  );		
		memcpy( raspistillCommand, frameReceived, sizeof(structRaspistillCommand) );
    
		//Delete last image
		deleteFileIfExists(raspistillCommand->fileName);
				
		//Show message received    
		printf( "Raspistill -> %s\n",raspistillCommand->raspiCommand );
		
        //Execute raspistill
		FILE* pipe;
		pipe = popen(raspistillCommand->raspiCommand, "r");
		pclose(pipe);
		printf("Command executed\n");
		
		//Verify if it was created the snapshot
		//..
		if( fileExists( raspistillCommand->fileName ) )
			buffer[1] = 1;
		else
			buffer[1] = 0;
        
		//Send ACK		
		write(newsockfd,&buffer,2);
    
		break;

	//
	//  1) Obtiene commando
	//	2) Envía ACK
	//	3) Ejecuta commando
	//
    case 5:
		
		//Semd ACK
		buffer[1] = 1;	
		write(newsockfd,&buffer,2);
		
		//Delay if required
		if( frameReceived->header.trigeredTime > 0 )
		{
			printf("Dalaying time (%d seconds)\n",frameReceived->header.trigeredTime);
			fflush(stdout);
			sleep(frameReceived->header.trigeredTime);
		}
		
		//Obtain command
		printf("Applying command -> %s\n",frameReceived->msg);

        //Execute command
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
        
        //Notify after finish
        printf("Command Execueted\n");
        

		break;

	//
    //Send image from camera
    //
    case 6:
      break;
	
	//
    //Obtain and execute command to aquire image using raspistill
    //..
    case 7:
		//Prepare memory
		memset( reqImg, '\0', sizeof(strReqImg)  );		
		memcpy( reqImg, frameReceived, sizeof(strReqImg) );
		memset(buffer,'\0',3);
		//Send ACK with camera status
		if( reqImgIsValid( reqImg ) )
			buffer[1] = 1;
		else
			buffer[1] = 0;	
		write(newsockfd,&buffer,2);
		
		if( buffer[1] == 0 )break;
		else{ 
			//Get image
			//..
			printf("Making the snapshot by applying raspistill\n");
			std::string fileName = "./tmpSnapshots/tmpImg.RGB888";
			if( getRaspImg(reqImg,fileName) ){				
				printf("Snapshot [OK]\n");
			}else{
			  printf("Snapshot[Fail]\n");
			  break;
			}
			
			//Cut image if necessary
			

		}
    	break;

	case 8:
	/*
		//Prepare memory		
		memset( reqImg, '\0', sizeof(strReqImg) );
		memcpy( reqImg, frameReceived, sizeof(strReqImg) );
		printf("Square: x(%d) y(%d) w(%d) h(%d)\n",reqImg->sqApSett.rectX,reqImg->sqApSett.rectY,reqImg->sqApSett.rectW,reqImg->sqApSett.rectH);

		//Send ACK with camera status
		buffer[1] = 1;
		write(newsockfd,&buffer,2);
		if( buffer[1] == 0 )break;
		else{

			//Get square-aperture image
			//..
			printf("Making the square-aperture snapshot by applying raspistill\n");
			std::string fileNameSqu = "./tmpSnapshots/tmpImgSqu.RGB888";
			reqImg->squApert = true;
			reqImg->needCut	 = true;
			if( getRaspImg(reqImg,fileNameSqu) ){
			  printf("Snapshot square-aperture [OK]\n");
			}else{
			  printf("Snapshot square-aperture [Fail]\n");
			  break;
			}		
		}*/
    	break;

	//
    //Require slide-cube from scratch
    //..
    case 9:
		//Order message
		memset( reqImg, '\0', sizeof(strReqImg)  );		
		memcpy( reqImg, frameReceived, sizeof(strReqImg) );
		
		//Verify valid message
		if( reqImgIsValid( reqImg ) )
		{
			//Send ACK with camera status OK
			buffer[1] = 1;
			write(newsockfd,&buffer,2);
			
			//Start time lapse process
			startTimLapse( newsockfd, reqImg );
		}
		else
		{
			//Send ACK with camera status ERROR
			buffer[1] = 0;
			write(newsockfd,&buffer,2);
		}
    	break;
	//
    //Response if file exists
    //..
    case 10:
		//Order message		
		aux = sizeof(reqFileInfo);
		memset( reqFileInfo, '\0', aux  );		
		memcpy( reqFileInfo, frameReceived, n );
		checkIfRequestedFileExists( newsockfd, reqFileInfo );
    	break;
    	
    //
    //Send file, assume that file exists and user check this using  
    //command 10.
    //..
    case 11:
		//Order message
		aux = sizeof(reqFileInfo);
		memset( reqFileInfo, '\0', aux );		
		memcpy( reqFileInfo, frameReceived, n );
		n = sendRequestedFile( newsockfd, reqFileInfo );
    	break;
    	
    //
    //1) Send ACK
    //2) Delete tmpVideo if exist
    //3) Start to record a new video
    //
    case 12:
		//Ordering message
		memset( reqImg, '\0', sizeof(strReqImg)  );		
		memcpy( reqImg, frameReceived, sizeof(strReqImg) );
		//Send ACK with camera status OK
		buffer[1] = 1;
		write(newsockfd,&buffer,2);
		//Delete tmpVideo if exist
		funcClearFolder( "tmpSnapVideos" );
		
		//Motor walk
		pid_t pid;
		if( argc == 3 )
		{
			if ( (pid=fork()) == 0 )
			{//Parallel
				funcMotorDoAWalk( reqImg );
			}
			else
			{
				//Start to record a new video
				recordVideo( reqImg );
			}
		}
		else
		{
			//Start to record a new video
			recordVideo( reqImg );
		}
		
			
		
		break;

    //Unrecognized instruction
    default:
      n = write(newsockfd,"Default",8);
      if (n < 0)
      {
        error("ERROR writing to socket");
      }
      holdon = 0;
      break;

    }

    
  }
  
  delete[] reqImg;
  delete[] reqFileInfo;

  close(newsockfd);
  close(sockfd);
  printf("It finishes...\n");
  return 0;
}

void funcMotorDoAWalk( strReqImg *reqImg  )
{
	int16_t timeMs, degreeEnd, degreeInit;
	degreeInit 	= reqImg->motorWalk.degreeIni;
	timeMs 		= reqImg->motorWalk.durationMs;
	degreeEnd 	= reqImg->motorWalk.degreeEnd;
	
	//Execute raspistill
	FILE* pipe;	
	int i;
	std::string tmpCommand;
	
	//Go to zero
	tmpCommand = "echo 2 > ";
	tmpCommand.append(SERIAL_PORT.c_str());
	for(i=0; i<360; i++)
	{		
		pipe = popen(tmpCommand.c_str(), "r");
		//usleep(10*1000);
	}
	
	//Go to ini
	tmpCommand = "echo 1 > ";
	tmpCommand.append(SERIAL_PORT.c_str());
	for(i=0; i<degreeInit; i++)
	{
		pipe = popen(tmpCommand.c_str(), "r");
		//printf("Going to Ini from zero\n");
		usleep(10*1000);
	}
	fflush(stdout);
	
	//Wait to stabilize the camera
	usleep(reqImg->motorWalk.stabilizingMs*1000);
	
	//Do a walk
	tmpCommand = "echo 1 > ";
	tmpCommand.append(SERIAL_PORT.c_str());
	int delayMs = ceil( (float)timeMs/(float)(degreeEnd-degreeInit));
	for(i=degreeInit; i<=degreeEnd; i++)
	{
		//printf("Walking\n");
		pipe = popen(tmpCommand.c_str(), "r");
		usleep(delayMs*1000);
	}
	fflush(stdout);
	
	//Wait at the end of the walk
	usleep(2000*1000);
	
	//Go to zero
	tmpCommand = "echo 2 > ";
	tmpCommand.append(SERIAL_PORT.c_str());
	for(i=1; i<=360; i++)
	{		
		//printf("Returnning to zero\n");
		pipe = popen(tmpCommand.c_str(), "r");
		//usleep(10*1000);
	}
	fflush(stdout);
	
	pclose(pipe);
	
}

void recordVideo( strReqImg *reqImg )
{
	//Concatenate raspisVid command
	//raspivid -o video.h264 -t 1000
	//..
	std::string *raspiVIDEOCommand = genRaspiVideoCommand( reqImg );
	printf("RaspiVIDEOCommand: %s\n",raspiVIDEOCommand->c_str());
	//Prepare command as required
	char *tmpComm = new char[raspiVIDEOCommand->size()+1];
	std::copy(raspiVIDEOCommand->begin(), raspiVIDEOCommand->end(),tmpComm);
	tmpComm[raspiVIDEOCommand->size()] = '\0';
	
	//Execute raspistill
	FILE* pipe;
	pipe = popen(raspiVIDEOCommand->c_str(), "r");
	pclose(pipe);
	
}

int funcSendFile( int sockfd, strReqFileInfo* reqFileInfo )
{
	std::string fileRequested = file2String( reqFileInfo->fileName );
	
	
	return 1;	
}

int checkIfRequestedFileExists( int sockfd, strReqFileInfo* reqFileInfo )
{	
	//Check if file exists and send ACK or ERROR


	//Check if file exists
	std::string internFileName;
	internFileName.assign( reqFileInfo->fileName );
	
	//Notify to user
	printf( "Client asked for: %s\n", internFileName.c_str() );
	fflush(stdout);
	
	if( fileExists( internFileName ) )
	{
		funcMessage("File exists");
		funcSendACK( sockfd, 101 );
	}
	else
	{
		funcMessage("File DOES NOT exists");
		funcSendERROR( sockfd, 101 );
	}
	
	return -1;
}

int sendRequestedFile( int sockfd, strReqFileInfo* reqFileInfo )
{	
	//It assumes that file exists
	// Recomendation: Client must to use checkIfRequestedFileExists
	//				  invoque this function
	//Send the fileName requested
	
	
	//Send image as frame
	//..
	int n;
	
	//Read file contain
	std::string tmpFile = file2String(reqFileInfo->fileName);
	
	//Send file len
	int fileLen = tmpFile.size();
	printf("fileLen: %d\n",fileLen);
	fflush(stdout);
	n = write( sockfd, &fileLen, sizeof(int)+1 );
	
	//Send file
	n = write( sockfd, tmpFile.c_str(), fileLen+1 );
	printf("Bytes inyectados: %d\n",n);
	
	
	return n;

}

int applyTimeLapseUsingRaspistill(strReqImg *reqImg)
{
	//
	// RETURN: number of imagery generated
	//
	
	//Prepare folder container
	funcClearFolder( "tmpTimeLapse" );
	
	//Generate console command
	std::string *timeLapseCommand = genSLIDECommand(reqImg);
	printf("Comm: %s\n",timeLapseCommand->c_str());
			
	//Execute command generated
	FILE* pipe;
	pipe = popen(timeLapseCommand->c_str(), "r");
	pclose(pipe);
	
	//Count imagery generated
	int i, numImgs, expectedNumImgs;
	std::ostringstream intToStr; 
	std::string tmpFileName;
	numImgs 		= 0;
	expectedNumImgs = ceil(
                            (float)(reqImg->slide.degreeEnd - reqImg->slide.degreeIni) /
                            (float)reqImg->slide.degreeJump
                         );
	for( i=1; i<=expectedNumImgs; i++ )
	{
		intToStr.str("");
		intToStr << i;		
		tmpFileName = "./tmpTimeLapse/" + intToStr.str() + ".png";
		if( funcPathExists( tmpFileName ) == true )
		{
			numImgs++;
		}
	}
	printf("Aquired imagery: %d of %d\n",numImgs,expectedNumImgs);
	return numImgs;
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
	char *tmpComm = new char[raspistillCommand->size()+1];
	std::copy(raspistillCommand->begin(), raspistillCommand->end(),tmpComm);
	tmpComm[raspistillCommand->size()] = '\0';
	
	//Execute raspistill
	//..
	//Remove file if exists	
	if(fileExists(fileName)){
		//remove(fileName.c_str());	
	}
	//Execute raspistill
	FILE* pipe;
	pipe = popen(raspistillCommand->c_str(), "r");
	pclose(pipe);
	
	//Verify if it was created the snapshot
	//..
	if(!fileExists(fileName)){
		return false;
	}
	
	//Crop image if neccesary
	//..
	if( reqImg->needCut )
	{
		float x,y,w,h;	
		//Normalization
		if(reqImg->squApert)
		{
			//x = (float)reqImg->sqApSett.rectX / (float)_BIG_WIDTH;
			//y = (float)reqImg->sqApSett.rectY / (float)_BIG_HEIGHT;
			//w = (float)reqImg->sqApSett.rectW / (float)_BIG_WIDTH;
			//d = (float)reqImg->sqApSett.rectH / (float)_BIG_HEIGHT;			
			//printf("Square: x(%.4f) y(%.4f) w(%.4f) h(%.4f)\n",x,y,w,d);
			//printf("Square: x(%d) y(%d) w(%d) h(%d)\n",reqImg->sqApSett.rectX,reqImg->sqApSett.rectY,reqImg->sqApSett.rectW,reqImg->sqApSett.rectH);
			x = reqImg->sqApSett.rectX;
			y = reqImg->sqApSett.rectY;
			w = reqImg->sqApSett.rectW;
			h = reqImg->sqApSett.rectH;
		}
		else
		{			
			//x = (float)reqImg->diffArea.rectX / (float)_BIG_WIDTH;
			//y = (float)reqImg->diffArea.rectY / (float)_BIG_HEIGHT;
			//w = (float)reqImg->diffArea.rectW / (float)_BIG_WIDTH;
			//d = (float)reqImg->diffArea.rectH / (float)_BIG_HEIGHT;
			//printf("Diff: x(%.4f) y(%.4f) w(%.4f) h(%.4f)\n",x,y,w,d);
			x = reqImg->diffArea.rectX;
			y = reqImg->diffArea.rectY;
			w = reqImg->diffArea.rectW;
			h = reqImg->diffArea.rectH;
		}
		
		//Save image
		int imgCols, imgRows, bpp;
		uint8_t* tmpImg 		= stbi_load(_PATH_IMG_GEN_USING_RASP, &imgCols, &imgRows, &bpp, 3);	
		uint8_t* croppedSlide 	= subimage( x, y, h, w, tmpImg, imgRows, imgCols );
		if( saveBinFile_From_u_int8_T( _PATH_IMG_GEN_USING_RASP, croppedSlide, (w*h*3) ) )
			std::cout << "File cropped successfully" << std::endl;
		else
			std::cout << "ERROR cropping image" << std::endl;
			
		
		
	}
	
	
	return true;
		
}

bool reqImgIsValid( strReqImg *reqImg )
{
	//Validates image required size
	if( reqImg->imgCols <= 0 )
	{
		printf("Width <= 0\n");
		return false;
	}	
	if( reqImg->imgRows <= 0 )
	{
		printf("Height <= 0\n");
		return false;
	}	
	
	//Validates area of interes
	if( reqImg->needCut == true )
	{
		printf("Cut required\n");
		
		if( reqImg->sqApSett.rectW <= 0 )
		{
			printf( "ROI W <= 0\n" );
			return false;
		}
		if( reqImg->sqApSett.rectH <= 0 )
		{
			printf( "ROI H <= 0\n" );
			return false;
		}
		if( reqImg->sqApSett.rectW < 0 )
		{
			printf( "ROI X < 0\n" );
			return false;
		}
		if( reqImg->sqApSett.rectW < 0 )
		{
			printf( "ROI Y < 0\n" );
			return false;
		}
	}
	
	//
	if( reqImg->isSlide == true )
	{
		if( reqImg->slide.x1 < 0 ){printf( "x1 < 0\n" );return false;}
		if( reqImg->slide.y1 < 0 ){printf( "y1 < 0\n" );return false;}
		if( reqImg->slide.rows1 < 1 ){printf( "rows1 < 1\n" );return false;}
		if( reqImg->slide.cols1 < 1 ){printf( "cols1 < 1\n" );return false;}
		
		if( reqImg->slide.x2 < 0 ){printf( "x2 < 0\n" );return false;}
		if( reqImg->slide.y2 < 0 ){printf( "y2 < 0\n" );return false;}
		if( reqImg->slide.rows2 < 1 ){printf( "rows2 < 1\n" );return false;}
		if( reqImg->slide.cols2 < 1 ){printf( "cols2 < 1\n" );return false;}
		
		if( reqImg->slide.speed < 100 ){printf( "speed < 100\n" );return false;}
		if( reqImg->slide.degreeEnd < 0 ){printf( "degreeEnd < 0\n" );return false;}
		if( reqImg->slide.degreeEnd > 360 ){printf( "degreeEnd > 360\n" );return false;}
		if( reqImg->slide.degreeJump < 1 ){printf( "degreeJump < 1\n" );return false;}
		if( reqImg->slide.degreeJump > 360 ){printf( "degreeJump >360\n" );return false;}
	}
		
	
	
	
	
	return true;
}

std::string *genSLIDECommand(strReqImg *reqImg)
{
	//
	//Initialize command
	//..
	std::string *tmpCommand = new std::string("raspistill -o ./tmpTimeLapse/%d.png"); //5000 -tl 200");
	std::ostringstream numberToString;
	tmpCommand->append(" -n -q 100 -gc");
	
	//Add lapse (speed) and duration
	numberToString.str("");
	int duration = 	 ceil( (float)( reqImg->slide.degreeEnd - reqImg->slide.degreeIni ) / 
					   (float)reqImg->slide.degreeJump ) * reqImg->slide.speed;
	numberToString << " -t " << duration << " -tl " << reqImg->slide.speed;
	tmpCommand->append( numberToString.str() );
	
	//Width
	numberToString.str("");
	numberToString<<reqImg->imgCols;
	tmpCommand->append(" -w " + numberToString.str());
	
	//Height
	numberToString.str("");
	numberToString<<reqImg->imgRows;
	tmpCommand->append(" -h " + numberToString.str());

	//Colour balance?
	if(reqImg->raspSett.ColorBalance){
		tmpCommand->append(" -ifx colourbalance");
	}
	
	//Denoise?
	if(reqImg->raspSett.Denoise){
		tmpCommand->append(" -ifx denoise");
	}	
	
	//Diffraction Shuter speed
	int shutSpeed = reqImg->raspSett.ShutterSpeed;
	if(
		(!reqImg->squApert && shutSpeed>0) ||	//Whe is by parts
		(reqImg->fullFrame  && shutSpeed>0)	//Whn is unique and shutter speed has been setted
	)
	{
		numberToString.str("");
		numberToString<<shutSpeed;
		tmpCommand->append(" -ss " + numberToString.str());
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
	
	//ISO
	if( reqImg->raspSett.ISO > 0 ){
		numberToString.str("");
		numberToString<<reqImg->raspSett.ISO;
		tmpCommand->append(" -ISO " + numberToString.str());
	}
	
	
	
	return tmpCommand;
}

std::string *genRaspiVideoCommand(strReqImg *reqImg)
{
	//Initialize command
	//..
	//std::string *tmpCommand = new std::string("raspivid -w 1296 -h 972 -o ");
	std::string *tmpCommand = new std::string("raspivid -o ");		
	tmpCommand->append( reqImg->video.o );		
	
	std::ostringstream auxIntToString;
	
	//Add seconds
	if( reqImg->video.t > 0 )
	{
		auxIntToString.str("");
		auxIntToString<< (1000*reqImg->video.t);
		tmpCommand->append(" -t " + auxIntToString.str());
	}
	else
	{
		tmpCommand->append(" -t 3000");
	}
	
	//Width
	if( reqImg->video.w > 0 )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.w;
		tmpCommand->append(" -w " + auxIntToString.str());		
	}
	
	//Height
	if( reqImg->video.h > 0 )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.h;
		tmpCommand->append(" -h " + auxIntToString.str());		
	}

	//-fps, --framerate	: Specify the frames per second to record
	if( reqImg->video.fps > 0 )
	{
		//Mode indicates video size
		auxIntToString.str("");
		auxIntToString << reqImg->video.fps;
		tmpCommand->append(" -fps " + auxIntToString.str());		
	}
	
    //-b, --bitrate	: Set bitrate. Use bits per second (e.g. 10MBits/s would be -b 10000000)
    if( reqImg->video.b > 0 )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.b;
		tmpCommand->append(" -fps " + auxIntToString.str());	
	}
    
    //-v, --verbose	: Output verbose information during run
    if( reqImg->video.v > 0 )
		tmpCommand->append(" -v " );
		
    //-d, --demo	: Run a demo mode (cycle through range of camera options, no capture)
    if( reqImg->video.d > 0 )
		tmpCommand->append(" -d " );
    
    //-e, --penc	: Display preview image *after* encoding (shows compression artifacts)
    if( reqImg->video.e > 0 )
		tmpCommand->append(" -e " );
		
    //-g, --intra	: Specify the intra refresh period (key frame rate/GoP size). Zero to produce an initial I-frame and then just P-frames.
    //-pf, --profile	: Specify H264 profile to use for encoding
    //-td, --timed	: Cycle between capture and pause. -cycle on,off where on is record time and off is pause time in ms
    //-s, --signal	: Cycle between capture and pause on Signal
    //-k, --keypress	: Cycle between capture and pause on ENTER
    //-i, --initial	: Initial state. Use 'record' or 'pause'. Default 'record'
    //-qp, --qp	: Quantisation parameter. Use approximately 10-40. Default 0 (off)
    //-ih, --inline	: Insert inline headers (SPS, PPS) to stream
    //-sg, --segment	: Segment output file in to multiple files at specified interval <ms>
    //-wr, --wrap	: In segment mode, wrap any numbered filename back to 1 when reach number
    //-sn, --start	: In segment mode, start with specified segment number
    //-sp, --split	: In wait mode, create new output file for each start event
    //-c, --circular	: Run encoded data through circular buffer until triggered then save
    //-x, --vectors	: Output filename <filename> for inline motion vectors
    //-cs, --camselect	: Select camera <number>. Default 0
        
    //-set, --settings	: Retrieve camera settings and write to stdout
    if( reqImg->video.set > 0 )
		tmpCommand->append(" -set " );
		
    //-md, --mode	: Force sensor mode. 0=auto. See docs for other modes available
    //Mode
	if( reqImg->video.md > 0 )
	{
		//Mode indicates video size
		auxIntToString.str("");
		auxIntToString << reqImg->video.md;
		tmpCommand->append(" --mode " + auxIntToString.str());		
	}
	
    //-if, --irefresh	: Set intra refresh type
    if( reqImg->video._if > 0 )
		tmpCommand->append(" -if " );
    
    //-fl, --flush	: Flush buffers in order to decrease latency
    if( reqImg->video.fl > 0 )
		tmpCommand->append(" -fl " );
		
    //-pts, --save-pts	: Save Timestamps to file for mkvmerge
    if( reqImg->video.pts > 0 )
		tmpCommand->append(" -pts " );
		
    //-cd, --codec	: Specify the codec to use - H264 (default) or MJPEG
	if( strcmp(reqImg->video.cd, "MJPEG") == 0 )
		tmpCommand->append(" -cd MJPEG " );
		
		
	//-sh, --sharpness	: Set image sharpness (-100 to 100)
	if( reqImg->video.sh != '\0' )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.sh;
		tmpCommand->append(" -sh " + auxIntToString.str());		
	}
	
    //-co, --contrast	: Set image contrast (-100 to 100)
    if( reqImg->video.co != '\0' )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.co;
		tmpCommand->append(" -co " + auxIntToString.str());		
	}
	
    //-br, --brightness	: Set image brightness (0 to 100)
    if( reqImg->video.br != '\0' )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.br;
		tmpCommand->append(" -br " + auxIntToString.str());		
	}
	
    //-sa, --saturation	: Set image saturation (-100 to 100)
    if( reqImg->video.sa != '\0' )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.sa;
		tmpCommand->append(" -sa " + auxIntToString.str());		
	}
	
    //-ISO, --ISO	: Set capture ISO
    if( reqImg->video.ISO != '\0' )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.ISO;
		tmpCommand->append(" -ISO " + auxIntToString.str());		
	}
	
    //-vs, --vstab	: Turn on video stabilisation
    if( reqImg->video.vs > 0 )
		tmpCommand->append(" -vs ");		

    //-ev, --ev	: Set EV compensation - steps of 1/6 stop
    //-ex, --exposure	: Set exposure mode (see Notes)
    if( reqImg->video.ex > 0 )
	{
		auxIntToString.str("");
		switch( reqImg->video.ex )
		{
			/*
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
			*/			
			case _EXPOSURE_off:
				tmpCommand->append(" -ex off " );
				break;
			case _EXPOSURE_auto:
				tmpCommand->append(" -ex auto " );
				break;
			case _EXPOSURE_night:
				tmpCommand->append(" -ex night " );
				break;
			case _EXPOSURE_nightpreview:
				tmpCommand->append(" -ex nightpreview " );
				break;
			case _EXPOSURE_backlight:
				tmpCommand->append(" -ex backlight " );
				break;
			case _EXPOSURE_spotlight:
				tmpCommand->append(" -ex spotlight " );
				break;
			case _EXPOSURE_sports:
				tmpCommand->append(" -ex sports " );
				break;
			case _EXPOSURE_snow:
				tmpCommand->append(" -ex snow " );
				break;
			case _EXPOSURE_verylong:
				tmpCommand->append(" -ex verylong " );
				break;
			case _EXPOSURE_fixedfps:
				tmpCommand->append(" -ex fixedfps " );
			case _EXPOSURE_antishake:
				tmpCommand->append(" -ex antishake " );
				break;
			case _EXPOSURE_fireworks:
				tmpCommand->append(" -ex fireworks " );
				break;
		}
	}
    
    //-awb, --awb	: Set AWB mode (see Notes)
    if( reqImg->video.awb > 0 )
	{
		auxIntToString.str("");
		switch( reqImg->video.awb )
		{
			/*
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
			*/		
			case _AWB_off:
				tmpCommand->append(" -awb off " );
				break;
			case _AWB_auto:
				tmpCommand->append(" -awb auto " );
				break;
			case _AWB_sun:
				tmpCommand->append(" -awb sun " );
				break;
			case _AWB_cloud:
				tmpCommand->append(" -awb cloud " );
				break;
			case _AWB_shade:
				tmpCommand->append(" -awb shade " );
				break;
			case _AWB_tungsten:
				tmpCommand->append(" -awb tungsten " );
				break;
			case _AWB_fluorescent:
				tmpCommand->append(" -awb fluorescent " );
				break;
			case _AWB_incandescent:
				tmpCommand->append(" -awb incandescent " );
				break;
			case _AWB_flash:
				tmpCommand->append(" -awb flash " );
				break;
			case _AWB_horizon:
				tmpCommand->append(" -awb horizon " );
				break;
		}
	}
    
    //-ifx, --imxfx	: Set image effect (see Notes)
    //-cfx, --colfx	: Set colour effect (U:V)
    //-mm, --metering	: Set metering mode (see Notes)
    //-rot, --rotation	: Set image rotation (0-359)
    //-hf, --hflip	: Set horizontal flip
    //-vf, --vflip	: Set vertical flip
    //-roi, --roi	: Set region of interest (x,y,w,d as normalised coordinates [0.0-1.0])
    
    //-ss, --shutter	: Set shutter speed in microseconds
    if( reqImg->video.ss > 0 )
	{
		auxIntToString.str("");
		auxIntToString << reqImg->video.ss;
		tmpCommand->append(" -ss " + auxIntToString.str());		
	}
    
    //-awbg, --awbgains	: Set AWB gains - AWB mode must be off
    //-drc, --drc	: Set DRC Level
    //-st, --stats	: Force recomputation of statistics on stills capture pass
    //-a, --annotate	: Enable/Set annotate flags or text
    
    //-3d, --stereo	: Select stereoscopic mode
    if( reqImg->video._3d == 1 )
		tmpCommand->append(" -3d ");
		
    //-dec, --decimate	: Half width/height of stereo image
    if( reqImg->video.dec == 1 )
		tmpCommand->append(" -dec ");
    
    //-3dswap, --3dswap	: Swap camera order for stereoscopic
    if( reqImg->video._3dswap == 1 )
		tmpCommand->append(" -3dswap ");
		
    //-ae, --annotateex	: Set extra annotation parameters (text size, text colour(hex YUV), bg colour(hex YUV))
	
	
	
	return tmpCommand;	
}

std::string *genCommand(strReqImg *reqImg, const std::string& fileName)
{
	
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
	
	//Square Shuter speed
	int shutSpeed = reqImg->raspSett.SquareShutterSpeed;
	if( (reqImg->squApert && shutSpeed>0))
	{		
		ss.str("");
		ss<<shutSpeed;
		tmpCommand->append(" -ss " + ss.str());
	}
	
	//Diffraction Shuter speed
	shutSpeed = reqImg->raspSett.ShutterSpeed;
	if(
		(!reqImg->squApert && shutSpeed>0) ||	//Whe is by parts
		(reqImg->fullFrame  && shutSpeed>0)	//Whe is unique and shutter speed has been setted
	)
	{
		ss.str("");
		ss<<shutSpeed;
		tmpCommand->append(" -ss " + ss.str());
	}
	
	//Trigering timer
	if( reqImg->raspSett.TriggerTime > 0 )
	{
		ss.str("");
		ss<<(reqImg->raspSett.TriggerTime*1000);
		tmpCommand->append(" -t " + ss.str());
	}
	else
	{
		ss.str("");
		ss<<250;//Milliseconds by default
		tmpCommand->append(" -t " + ss.str());
	}	
	
	//Width
	ss.str("");
	ss<<reqImg->imgCols;
	tmpCommand->append(" -w " + ss.str());
	
	//Height
	ss.str("");
	ss<<reqImg->imgRows;
	tmpCommand->append(" -h " + ss.str());
	
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
	
	//ISO
	if( reqImg->raspSett.ISO > 0 ){
		ss.str("");
		ss<<reqImg->raspSett.ISO;
		tmpCommand->append(" -ISO " + ss.str());
	}
	
	
	return tmpCommand;
}

int fileExists( const std::string& fileName ){
	struct stat buffer;
	return( stat(fileName.c_str(), &buffer) == 0 );
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
    printf("fileLen: %i\n", fileLen);
    n = write(newsockfd,&fileLen,sizeof(unsigned int));
    if (n < 0){
      error("ERROR writing to socket");      
    }
    printf("Len sent\n");

    //Get file request
    printf("Reading\n");
    n = read(newsockfd, buffer, frameBodyLen-1);
    if (n < 0){
      error("ERROR reading from socket");
    }
    //printf("File requested\n");

    //Send file
    aux = ceil( (float)fileLen / (float)frameBodyLen );
    //aux = ((aux*frameBodyLen)<fileLen)?aux+1:aux;
    printf("numMsgs: %i - frameBodyLen: %d \n",aux,frameBodyLen);
    for(i=1; i<=aux; i++){
		//printf("Msg: %d\n",i);
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

bool funcPathExists( std::string fileName )
{
	FILE* fileAsked = fopen( fileName.c_str(), "r" );
	if( fileAsked )
	{
		fclose( fileAsked );
		return true;
	}	
	return false;
}

bool funcCreateFolder( std::string folderName )
{
	if( funcPathExists( folderName ) == true )
	{
		printf( "Cannot create folder (%s), folder exits\n", folderName.c_str() );		
		return false;
	}
	else
	{
		std::string *tmpCommand = new std::string("mkdir -m777 ");
		tmpCommand->append( folderName );		
		system( tmpCommand->c_str() );
	}
	return true;
}

bool funcDeleteFolderRecursively( std::string folderName )
{
	if( funcPathExists( folderName ) == false )
	{
		printf( "Cannot delete folder (%s), folder does not exits\n", folderName.c_str() );		
		return false;
	}
	else
	{
		std::string *tmpCommand = new std::string("rm -r ");
		tmpCommand->append( folderName );		
		system( tmpCommand->c_str() );
	}
	return true;
}

bool funcClearFolder( std::string folderName )
{
	if( funcPathExists( folderName ) == true )
		if( funcDeleteFolderRecursively( folderName ) == false )
			return false;
	return funcCreateFolder( folderName );	
}

void startTimLapse(int newsockfd, strReqImg *reqImg)
{
	int n;
	strNumSlideImgs strNumImgs;
	char buffer[frameBodyLen];
	
	//Wait for instruction to start time lapse
	n = read(newsockfd, buffer, frameBodyLen-1);
	if (n < 0)
	{
		error("Sock: Before start time lapse");
	}

	//Start time lapse and send ACK with the number of images generated	
	strNumImgs.idMsg 	= buffer[0];
	strNumImgs.numImgs	= applyTimeLapseUsingRaspistill( reqImg );
	n = sizeof(strNumSlideImgs);
	memcpy(buffer,&strNumImgs,n);
	write(newsockfd,&buffer,n+1);
	
	//Receive instruction acording to the number of images generated
	//1:Enought images and create the cube 0:finish with error
	n = read(newsockfd, buffer, frameBodyLen-1);
	if (n < 0)
	{
		error("Sock: Before start time lapse");
	}
	if( buffer[0] == '1' )
	{
		std::cout << "Data-cube generatios process started" << std::endl;
		startGenerateSlideCube( newsockfd, reqImg);
	}
	else
	{
		std::cout << "Time lapse process finished" << std::endl;
	}
}

int startGenerateSlideCube(int newsockfd, strReqImg *reqImg)
{
	//std::cout << "startGenerateSlideCube" << std::endl;
	
	//int i, n, expectedN;
	int i, expectedN;
	std::string* tmpImgName = new std::string();
	std::ostringstream ss;
	//char buffer[frameBodyLen];
	//strNumSlideImgs strNumImgs;
	
	expectedN = ceil(
						(float)(reqImg->slide.degreeEnd - reqImg->slide.degreeIni) /
						(float)reqImg->slide.degreeJump
					 );
	//expectedN = 1;
	for( i=1; i<=expectedN; i++ )
	{
		//Define next image
		ss.str("");
		ss << i;
		tmpImgName->assign("");
		tmpImgName->append("./tmpTimeLapse/");
		tmpImgName->append(ss.str());
		tmpImgName->append(".png");
		
		//Send image if exists
		if( funcPathExists( tmpImgName->c_str() ) == true )
		{
			std::cout << "Cropping and transmitting image " << i << "..." << std::endl;
			write(newsockfd,&i,sizeof(int));			
			cropAndTransmitSlide( tmpImgName, newsockfd, reqImg );
		}	
	}
	
	std::cout << "Notify that finished" << std::endl;
	i = -1;
	write(newsockfd,&i,sizeof(int));
	
	std::cout << "Slide-Cube sent" << std::endl;
	fflush(stdout);
	//delete[] tmpImgName;


	return 1;
}


bool cropAndTransmitSlide( std::string* tmpImgName, int newsockfd, strReqImg *reqImg )
{
	//std::cout << "cropAndTransmitSlide" << std::endl;
	//
	//Crop slide and diffraction areas and put all together into a single array
	//(cropImgTrans)
	//
	int imgCols, imgRows, bpp;
	int x1, y1, rows1, cols1;
	int x2, y2, rows2, cols2;
	
	//Received parameters
	x1 		= reqImg->slide.x1;			x2 		= reqImg->slide.x2;
	y1 		= reqImg->slide.y1;			y2 		= reqImg->slide.y2;
	rows1 	= reqImg->slide.rows1;		rows2 	= reqImg->slide.rows2;
	cols1 	= reqImg->slide.cols1;		cols2 	= reqImg->slide.cols2;
	
	//Crop imagery
	
	uint8_t* tmpImg 		= stbi_load(tmpImgName->c_str(), &imgCols, &imgRows, &bpp, 3);	
	uint8_t* croppedSlide 	= subimage( x1, y1, rows1, cols1, tmpImg, imgRows, imgCols );
	uint8_t* croppedDiff 	= subimage( x2, y2, rows2, cols2, tmpImg, imgRows, imgCols );
	
	//uint8_t* croppedSlide 	= stbi_load("croppedSlide.png", &cols1, &rows1, &bpp, 3);
	//uint8_t* croppedDiff 	= stbi_load("croppedDiff.png", &cols2, &rows2, &bpp, 3);
	

	//printf( "img: %s (R:%d,C:%d), Uno(x1:%d,y1:%d,rows1:%d,cols1:%d)\n",tmpImgName->c_str(),imgRows,imgCols,x1,y1,rows1,cols1 );
	//printf( "img: %s (R:%d,C:%d), Uno(x1:%d,y1:%d,rows1:%d,cols1:%d)\n",tmpImgName->c_str(),imgRows,imgCols,x2,y2,rows2,cols2 );
	
	/*
	int cropR = 600;
	int cropC = 1500;
	int width, height;
	uint8_t* rgb_image = stbi_load(tmpImgName->c_str(), &width, &height, &bpp, 3);	
	uint8_t* croppedImg = subimage( 200, 700, cropR, cropC, rgb_image, height, width );
	stbi_write_png("exampleCropped.png", cropC, cropR, 3, croppedImg, cropC*3);
	*/

	//Merge cropped imagery into a single array
	int slideLen 			= (rows1*cols1*3);
	int diffLen				= (rows2*cols2*3);
	int totalLen			= slideLen + diffLen;
	uint8_t* cropImgTrans 	= (uint8_t*)malloc(totalLen);
	memcpy( &cropImgTrans[0], croppedSlide, slideLen );
	memcpy( &cropImgTrans[slideLen], croppedDiff, diffLen );
	delete[] croppedSlide;
	delete[] croppedDiff;
	
	//printf("slideLen(%d), diffLen(%d), totalLen(%d)\n",slideLen,diffLen,totalLen);
	//fflush(stdout);
	
	//Save image
	//stbi_write_png("./tmpTimeLapse/croppedSlide.png", cols1, rows1, 3, croppedSlide, cols1*3);
	//stbi_write_png("./tmpTimeLapse/croppedDiff.png", cols2, rows2, 3, croppedDiff, cols2*3);
	//stbi_write_png("./tmpTimeLapse/allTogether.png", (cols1+cols2), rows1, 3, cropImgTrans, (cols1+cols2)*3);
	
	//
	//Transmit image
	//
	//funcGenericSendFrame( newsockfd, cropImgTrans, totalLen );
	funcSendOneMessage( newsockfd, cropImgTrans, totalLen );
	
	//delete[] tmpImg;	
	delete[] cropImgTrans;	
	
	//funcMessage("termina: cropAndTransmitSlide");
	
	return true;
}

uint8_t* subimage( int x1, int y1, int croppedRows, int croppedCols, uint8_t* img, int imgRows, int imgCols )
{ 
	
	uint8_t* croppedImg = (uint8_t*)malloc(croppedRows*croppedCols*3);
	
	if( 
		croppedRows < 1 || croppedCols < 1 || 
		x1 > imgCols 	|| x1 < 0 || 
		y1 > imgRows 	|| y1 < 0
	)
	{
		printf("Crop coordinates are out of bounds \n");
		if( croppedRows < 1 )
			funcMessage( "croppedRows < 1" );
		if( croppedCols < 1 )
			funcMessage( "croppedCols < 1" );
		if( x1 > imgCols )
			funcMessage( "x1 > imgCols" );
		if( x1 < 0 )
			funcMessage( "x1 < 0" );
		if( y1 > imgRows )
			funcMessage( "y1 > imgRows" );
		if( y1 < 0 )
			funcMessage( "y1 < 0" );
		
		
		return croppedImg;
	}

	int beforeX1	= (x1>0)?x1:0;
	//int afterX2 	= ((x1+croppedCols)<imgCols)?imgCols-(x1+croppedCols):0;
	int beforeY1	= (y1>0)?y1-1:0;
	//int step		= beforeX1 + afterX2;
	
	//printf( "img(%d x %d) | cropped(%d x %d) beforeX1: %d, afterX2: %d, step: %d\n", imgRows, imgCols, croppedRows, croppedCols,  beforeX1, afterX2, step );
	
	int r;
	uint8_t *ptr, *cropPtr;
	
	cropPtr = &croppedImg[0];
	ptr		= &img[0];
	ptr	   += ((beforeY1 * imgCols) + beforeX1)*3;

	for( r=0; r<croppedRows; r++ )
	{
		memcpy( cropPtr, ptr, croppedCols*3 );
		ptr 	+= imgCols*3;
		cropPtr	+= croppedCols*3;
	}
	
	return croppedImg;

}


int funcGenericSendFrame( int newsockfd, u_int8_t* frame, int lenToSend )
{
	//std::cout << "funcGenericSendFrame" << std::endl;
	
	//Send frameLen and number of messages to send
	//int numMsgs = floor( (float)lenToSend / (float)frameBodyLen );
	
	//u_int8_t* internFrame = (u_int8_t*)frame;
	
	//funcSendOneMessage( newsockfd, (void*)&lenToSend, sizeof(int) );

	//funcSendOneMessage( newsockfd, (void*)&numMsgs, sizeof(int) );
	
	funcSendOneMessage( newsockfd, frame, lenToSend );


	/*
	//Send frame
	int i, framePos;
	i 		 	= 0;
	framePos 	= 0;
	for( i=0; i<numMsgs; i++ )
	{
		funcSendOneMessage( newsockfd, &frame[framePos], frameBodyLen );
		framePos += frameBodyLen;
	}
	//Send last message if necessary
	int remainder = lenToSend%frameBodyLen;
	if( remainder > 0 )
	{
		printf("Remainder: %d framePos:%d\n",remainder,framePos);
		fflush(stdout);
		funcSendOneMessage( newsockfd, &frame[framePos], remainder );
		framePos += remainder;
	}
	
	
	funcMessage("\nEnd for");
	*/
	
	
	
	

	return 1;
}

void funcMessage( std::string msg )
{
	std::cout << msg << std::endl;
	fflush(stdout);
}

strReqSubframe* readSubframeRequested( int newsockfd, strReqSubframe* subframeReq )
{
	subframeReq->posIni = 0;
	subframeReq->len	= 0;
	int structSize		= sizeof(strReqSubframe)+1;
	
	//Prepare buffer
	int n;
	char buffer[structSize];
	memset(buffer,'\0',structSize);
	
	//Wait for request
	n = read(newsockfd, buffer, structSize);
	if( n < 0 )
		return subframeReq;//0's filled
	//std::cout << "Requiring parameters, n: " << n << std::endl;
	
	//Save parameters acquired
	memcpy(subframeReq,buffer,structSize-1);
	
	return subframeReq;
}

int funcSendOneMessage( int newsockfd, void* frame, int lenToSend )
{
	int n;
	
	//Send message
	n = write(newsockfd,frame,lenToSend+1);
	
	//If message sent complete
	if( n == lenToSend+1 )
		return n;
	
	//Return error sending message
	return 0;
}

int funcSendACK( int newsockfd, u_int8_t ID )
{
	//funcMessage("funcSendACK");
	
	int n;
	u_int8_t buffer[2];
	buffer[0] = 1;
	buffer[1] = ID;
	
	//printf("buffer[0](%d) buffer[1](%d)\n",buffer[0],buffer[1]);
	
	//Send ACK message
	n = write(newsockfd,&buffer[0],3);
	
	//If message sent complete
	if( n == 3 )
	{
		//printf("n(%d)\n",n);
		return 1;
	}
	
	//Return error sending message
	return 0;
}

int funcSendERROR( int newsockfd, u_int8_t ID )
{
	//funcMessage("funcSendERROR");
	
	int n;
	u_int8_t buffer[2];
	buffer[0] = 0;
	buffer[1] = ID;
	
	//Send ACK message
	n = write(newsockfd,&buffer[0],3);
	
	//If message sent complete
	if( n == 3 )
	{
		printf("n(%d)\n",n);
		return 1;
	}
	
	//Return error sending message
	return 0;
}

int saveBinFile_From_u_int8_T( std::string fileName, uint8_t *data, size_t len)
{
    std::ofstream fp;
    fp.open( fileName.c_str(), std::ios::out | std::ios::binary );
    fp.write((char*)data, len);
    fp.close();
    return 1;
}

int deleteFileIfExists( const char* fileName )
{
	return unlink(fileName);
}

int readFileContain( const string &fileName, string* contain )
{ 
	std::ifstream myfile ( fileName.c_str() );
	if( myfile.is_open() )
	{
		if( !getline( myfile, *contain ) )
		{
			return _FAIL;
		}
		myfile.close();
	}
	else
	{
		return _ERROR;
	}
	return _OK;
}
