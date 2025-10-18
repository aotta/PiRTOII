/*
//                       PiRTO II Flash MultiCART by Andrea Ottaviani 2024
//
//  Intellivision flash multicart based on Raspberry Pico board -
//
//  More info on https://github.com/aotta/ 
//
//   parts of code are directly from the A8PicoCart project by Robin Edwards 2023
//  
//   Needs to be a release NOT debug build for the cartridge emulation to work
// 
//   Edit myboard.h depending on the type of flash memory on the pico clone//
//
//   v. 1.03 2025-09-04 : Added check maxfile 
//   v. 1.05 porting to Arduino IDE - support rom until 180kb (Time Pilot)
//
*/

//#define intydebug // for debug print on intyscreen

/*
#include <stdio.h>
#include <stdlib.h>

#include "tusb.h"
*/
//#include "ff.h"

#include "hardware/gpio.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "pico/multicore.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "string.h"
#include "rom.h"
#include "dir_entry.hpp"
// include for Flash files
#include "SPI.h"
#include "SdFat_Adafruit_Fork.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
Adafruit_FlashTransport_RP2040 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatVolume fatfs;
FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;
// Check if flash is formatted
bool fs_formatted;
// Set to true when PC write to flash
bool fs_changed=false;


// Pico pin usage definitions

#define B0_PIN    0
#define B1_PIN    1
#define B2_PIN    2
#define B3_PIN    3
#define B4_PIN    4
#define B5_PIN    5
#define B6_PIN    6
#define B7_PIN    7
#define F0_PIN    8
#define F1_PIN    9
#define F2_PIN    10
#define F3_PIN    11
#define F4_PIN    12
#define F5_PIN    13
#define F6_PIN    14
#define F7_PIN    15
#define BDIR_PIN  16
#define BC2_PIN   17
#define BC1_PIN   18
#define MSYNC_PIN 19
#define RST_PIN   20
#define LED_PIN   25

// Pico pin usage masks

#define B0_PIN_MASK     0x00000001L // gpio 0
#define B1_PIN_MASK     0x00000002L
#define B2_PIN_MASK     0x00000004L
#define B3_PIN_MASK     0x00000008L
#define B4_PIN_MASK     0x00000010L
#define B5_PIN_MASK     0x00000020L
#define B6_PIN_MASK     0x00000040L
#define B7_PIN_MASK     0x00000080L

#define F0_PIN_MASK     0x00000100L
#define F1_PIN_MASK     0x00000200L
#define F2_PIN_MASK     0x00000400L
#define F3_PIN_MASK     0x00000800L
#define F4_PIN_MASK     0x00001000L
#define F5_PIN_MASK     0x00002000L
#define F6_PIN_MASK     0x00004000L
#define F7_PIN_MASK     0x00008000L  // gpio 15

#define BDIR_PIN_MASK   0x00010000L // gpio 16
#define BC2_PIN_MASK    0x00020000L // gpio 17
#define BC1_PIN_MASK    0x00040000L // gpio 18
#define MSYNC_PIN_MASK  0x00080000L // gpio 19
#define RST_PIN_MASK    0x00100000L // gpio 20
#define LED_PIN_MASK    0x02000000L // gpio 25

// Aggregate Pico pin usage masks
#define BC1e2_PIN_MASK  0x00060000L
#define BX_PIN_MASK     0x000000FFL
#define FX_PIN_MASK     0x0000FF00L
#define DATA_PIN_MASK   0x0000FFFFL
#define BUS_STATE_MASK  0x00070000L
#define ALL_GPIO_MASK  	0x021FFFFFL
#define ALWAYS_IN_MASK  (BUS_STATE_MASK)
#define ALWAYS_OUT_MASK (LED_PIN_MASK)


#define SET_DATA_MODE_OUT   gpio_set_dir_out_masked(DATA_PIN_MASK)
#define SET_DATA_MODE_IN    gpio_set_dir_in_masked(DATA_PIN_MASK)

#define resetLow()  gpio_set_dir(RST_PIN,true); gpio_put(RST_PIN,true); //  Pirto to INTV BUS ; RST Output set to 0
#define resetHigh() gpio_set_dir(RST_PIN,true); gpio_put(RST_PIN,false); // RST is INPUT; B->A, INTV BUS to TEENSY

// Inty bus values (BC1+BC2+BDIR) GPIO 18-17-16

#define BUS_NACT  0b000  //0
#define BUS_BAR   0b001  //1
#define BUS_IAB   0b010  //2
#define BUS_DWS   0b011  //3
#define BUS_ADAR  0b100  //4
#define BUS_DW    0b101  //5
#define BUS_DTB   0b110  //6
#define BUS_INTAK 0b111  //7


unsigned char busLookup[8];

char RBLo,RBHi;
#define BINLENGTH  1024*96//65536L
#define RAMSIZE  0x2100
uint16_t ROM[BINLENGTH];

uint16_t RAM[RAMSIZE];
#define maxHacks 32
uint16_t HACK[maxHacks];
uint16_t HACK_CODE[maxHacks];

char curPath[256];
char path[256];
unsigned char files[256*64] = {0};
unsigned char nomefiles[32*25] = {0};
int fileda=0,filea=0;
volatile char cmd=0;
char errorBuf[40];
bool cmd_executing=false;

unsigned int parallelBus2;
  
unsigned int romLen;
unsigned int ramfrom = 0;
unsigned int ramto =   0;
unsigned int mapfrom[80];
unsigned int mapto[80];
unsigned int maprom[80];
int mapdelta[80];
unsigned int mapsize[80];
unsigned int addrto[80];
unsigned int RAMused = 0;
volatile bool RAM8 = false;
unsigned int tipo[80];  // 0-rom / 1-page / 2-ram
unsigned int page[80];  // page number

char substr[100];

int slot;
int hacks;

int base=0x17f;

uint32_t selectedfile_size;    // BIN file size
char longfilename[60];         // long file name (trunked) 
char mapfilename[60];          // map cfg file name
char testfilename[60];         // for check if directory
char rewindfilename[60];       // for rewind browsing
char rewindprev[60];       // for rewind browsing
int lenfilename; 
char tiposcelta[9];
bool pressed=false;



// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in rom_table to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
 
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // sync with flash
  flash.syncBlocks();

  // clear file system's cache to force refresh
  fatfs.cacheClear();

  fs_changed = true;

}


////////////////////////////////////////////////////////////////////////////////////
//                     RESET
////////////////////////////////////////////////////////////////////////////////////
void resetCart() {
  gpio_init(MSYNC_PIN);
  gpio_set_dir(MSYNC_PIN,false);
  gpio_set_pulls(MSYNC_PIN,false,true);
  gpio_put(LED_PIN,false);
  resetHigh();
   sleep_ms(30);  // was 20 for Model II; 30 works for both
   resetLow();
  //sleep_ms(3);  // was 2 for Model II; 3 works for both
 
  //while ((gpio_get(MSYNC_PIN)==1)) ;
    
  sleep_ms(1);  // was 1 for Model II; 
 resetHigh();
 gpio_put(LED_PIN,true);
   
}


/*
 Theory of Operation
 -------------------
 Inty sends command to mcu on cart by writing to 50000 (CMD), 50001 (parameter) and menu (50002-50641) 
 Inty must be running from RAM when it sends a command, since the mcu on the cart will
 go away at that point. Inty polls 50001 until it reads $1.
*/
#pragma GCC push_options
#pragma GCC optimize ("O3")

void __time_critical_func(setup1()) {   //HandleBUS()

  unsigned int lastBusState, busState1;
  unsigned int busSt1, busSt2;
  unsigned int parallelBus;
  unsigned int dataOut, dataInd;
  unsigned int dataWrite=0;
  unsigned char busBit;
  bool deviceAddress = false; 
  unsigned int curPage=0;
  unsigned int checpage=0;
  int exitBUS;

  //multicore_lockout_victim_init();	

  sleep_ms(480);

  busState1 = BUS_NACT;
  lastBusState = BUS_NACT;
  
  dataOut=0;

    gpio_set_dir_in_masked(ALWAYS_IN_MASK);
    gpio_set_dir_out_masked(ALWAYS_OUT_MASK);
 
    // Initial conditions
    SET_DATA_MODE_IN;
   
while(1) {
    // Wait for the bus state to change
  
    do {
    } while (!((gpio_get_all() ^ lastBusState) & BUS_STATE_MASK));
    // We detected a change, but reread the bus state to make sure that all three pins have settled
     lastBusState = gpio_get_all();

    busState1 = ((lastBusState & BUS_STATE_MASK)>> BDIR_PIN); //if gpio9    
   
    busBit = busLookup[busState1];
    // Avoiding switch statements here because timing is critical and needs to be deterministic
    if (!busBit)
    {
      // -----------------------
      // DTB
      // -----------------------
      // DTB needs to come first since its timing is critical.  The CP-1600 expects data to be
      // placed on the bus early in the bus cycle (i.e. we need to get data on the bus quickly!)
	    if (deviceAddress)
      {
        // The data was prefetched during BAR/ADAR.  There isn't nearly enough time to fetch it here.
        // We can just output it.
        SET_DATA_MODE_OUT;
        gpio_put_masked(DATA_PIN_MASK,dataOut);
        asm inline ("nop;nop;nop;nop;");

       /// while ((gpio_get_all() & BC1_PIN_MASK)); // wait while bc1 & bc2 are high... it's enough test BC1
        while(((gpio_get_all() & BC1e2_PIN_MASK)>>BC2_PIN)==3);
        //asm inline (delWR); //150ns
        
              
       SET_DATA_MODE_IN;
      }
     }
    else
    {
      busBit >>= 1;
      if (!busBit)
      {
        // -----------------------
        // BAR, ADAR
        // -----------------------
	   if (busState1==BUS_ADAR) {
	     if (deviceAddress)  
      		{
        // The data was prefetched during BAR/ADAR.  There isn't nearly enough time to fetch it here.
        // We can just output it.
       	SET_DATA_MODE_OUT;
        gpio_put_masked(DATA_PIN_MASK,dataOut);
        
        while((gpio_get_all() & BC1_PIN_MASK)>>BC1_PIN); //wait BC1 go down 
        //asm inline (delWR); //150ns

         SET_DATA_MODE_IN;
        
      		}
	   }
      /// ELSE is BAR   
        // Prefetch data here because there won't be enough time to get it during DTB.
        // However, we can't take forever because of all the time we had to wait for
        // the address to appear on the bus.
        SET_DATA_MODE_IN;
       // We have to wait until the address is stable on the bus
        // waiting bus is stable 66 nop at 200mhz is ok/85 at 240
    
       while(((parallelBus=gpio_get_all()) & BDIR_PIN_MASK));  // wait DIR go low for finish BAR cycle 
       //asm inline (delRD); //150ns
    
       parallelBus = gpio_get_all()& 0xFFFF; 

       deviceAddress = false;
         
       // Load data for DTB here to save time
           for (int8_t i=0; i <= slot; i++) {
            if ((parallelBus - maprom[i]) <= mapsize[i]) {
              if (tipo[i]==0) {
                dataOut=ROM[(parallelBus - mapdelta[i])];
                deviceAddress = true;
                break;
              }
              if (tipo[i]==1) {
                if (page[i]==curPage) {
                  dataOut=ROM[(parallelBus - mapdelta[i])];
                  deviceAddress = true;
                  break;
                }
                if ((parallelBus & 0xfff)==0xfff) {
                  checpage=1;
                   deviceAddress = true;
                  break;
                }
              }
              if (tipo[i]==2) {
                dataOut=RAM[parallelBus - ramfrom];
                deviceAddress = true;
                break;
              } 
            }    
          }
  
        if (hacks>0) {
          for (int i=0; i<maxHacks;i++) {
            if (parallelBus==HACK[i]) {
              dataOut=HACK_CODE[i];
              deviceAddress = true;
	          }
            break;
          }
        } 
      }
      else
      {
         busBit >>= 1;
        if (!busBit)
        {
          // -----------------------
          // DWS
          // -----------------------
       
          if (deviceAddress) {

            SET_DATA_MODE_IN;
           
            dataWrite = gpio_get_all() & 0xFFFF; 
        
          if (RAMused == 1) {
              if (RAM8) 
                RAM[parallelBus-ramfrom]=dataWrite & 0xFF;
              else 
                RAM[parallelBus-ramfrom]=dataWrite;
            } 
           if ((checpage == 1) && (((dataWrite >> 4) & 0xff) == 0xA5)) {
              curPage=dataWrite & 0xf;
              checpage=0;
           }
           
        }
        else
        {
         // -----------------------
         // NACT, IAB, DW, INTAK
         // -----------------------
         // reconnect to bus
           parallelBus2=parallelBus;
           SET_DATA_MODE_IN;
        }
        
        }
      } 
    }    
  }
} 

#pragma GCC pop_options


////////////////////////////////////////////////////////////////////////////////////
//                     Error(N)
////////////////////////////////////////////////////////////////////////////////////
void error(int numblink){
  Serial.print("Error ");
  Serial.println(numblink);
  while(1){
    gpio_set_dir(25,GPIO_OUT);
    
    for(int i=0;i<numblink;i++) {
      gpio_put(25,true);
      sleep_ms(600);
      gpio_put(25,false);
      sleep_ms(500);
      Serial.print("Error ");
      Serial.println(numblink);
    }
  sleep_ms(2000);
  }
}

/////////////////////////////////
// print on mfile on inty
////////////////////////////////
void printInty(char *temp) {
  for (int i=0;i<120;i++) RAM[0x17f+i]=0;
  for (int i=0;i<60;i++) {
    RAM[0x17f+i*2]=temp[i];
    if (temp[i]==0) {
      RAM[0x17f+i*2]='*';
      break;
    }
  }
  sleep_ms(1000);
} 
int read_directory(char *path) {
  //  if (path[0]==0) strcpy(path,'/');
    Serial.print("Read_directory: ");
    Serial.println(path);
    int ret = 0;
    num_dir_entries = 0;
    DIR_ENTRY *dst = (DIR_ENTRY *)&files[0];

    FatFile dir;
    if (dir.open(path, O_RDONLY)) {
    //if (dir.open("/", O_RDONLY)) {
        FatFile file;
        while (num_dir_entries < 64 && file.openNext(&dir, O_RDONLY)) {
            char filename[256];
            file.getName(filename, sizeof(filename));
            
            if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                file.close();
                continue;
            }
            
            if (file.isHidden() || file.isSystem()) {
                file.close();
                continue;
            }
            
            dst->isDir = file.isDir();
            if (!dst->isDir && !is_valid_file(filename)) {
                file.close();
                continue;
            }
            
            // Usa lo stesso nome per entrambi i campi
            strncpy(dst->long_filename, filename, 31);
            dst->long_filename[31] = 0;
            strncpy(dst->filename, filename, 12);
            dst->filename[12] = 0;
            
            dst->full_path[0] = 0;
            dst++;
            num_dir_entries++;
            
            file.close();
        }
        dir.close();
        
        qsort((DIR_ENTRY *)&files[0], num_dir_entries, sizeof(DIR_ENTRY), entry_compare);
        ret = 1;
    }
    else {
        Serial.println("Can't read directory");
    }
    
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////* load file in  ROM */////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void load_file(char *filename) {
	
  int car_file = 0;
  int  size = 0;
	Serial.print("load file: ");
  Serial.println(filename);
  //printInty("loadfile");
  //printInty(filename);

 
	if (!(file.open(filename))) {
		Serial.print("Can't open file ");
    Serial.println(filename);
    error(2);
	}

 Serial.println("Start load");
 size=0;
 while ((file.available()) && (size<(BINLENGTH-1))) {
  
    RBHi = file.read();
    RBLo = file.read();
   
    ROM[size]= RBLo | (RBHi << 8);
	  size++;
   
  }
  if (size==(BINLENGTH-1)) Serial.println("Max size exceeded");
closefile:
  romLen=size;
  RAM[base+202]=romLen;
  Serial.print("Len: ");Serial.println(romLen);
 	file.close();
}
// Funzione per leggere una riga dal file

/////////////////////////////
bool leggiRiga(FatFile &f, char* buf, int max) {
    int i = 0;
    while (i < max-1 && f.available()) {
        char c = f.read();
        if (c == '\n') break;
        if (c == '\r') {
            if (f.available() && f.peek() == '\n') f.read();
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i > 0 || f.available();
}
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////* load .cfg file */////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void load_cfg(char *filename) {
	int car_file = 0;
	int size = 0;
  unsigned char byteread=0;
  char riga[80];
  char tmp[80];
  int linepos;     

  memset(tmp,0,sizeof(tmp));
  int j=40;
  int dot=0;
  while ((filename[j]!='.')&&(j>0)) {
    dot=j;
    j--;
  }
  for (int i=0;i<j;i++) tmp[i]=filename[i];
  tmp[j]=0;
  
   strcat(tmp,".cfg");
  
   memcpy(filename,tmp,sizeof(tmp));
   

	if (!(file.open(filename))) {
		Serial.println("cfg not found");
    //printInty("using 0.cfg");
    strcpy(filename,"/0.cfg");  
    if (!(file.open(filename))) {
		  Serial.println("Can't find 0.cfg");
      error(3);
    }
  } else {
   Serial.println("cfg found");
  }
 
	unsigned char *dst = &byteread;
	int bytes_to_read = 1;
	// read the file to SRAM
  #ifdef debug
  int slot1=0;  // poi TOGLIERE DOPO IL DEBUG -----------------------
  #else
    slot=0;  // poi rimettere dopo il debug!!! ------------------------------
  #endif

  hacks=0;
  #ifndef debug
  RAMused=0; // poi rimettere dopo il debug!!!  --------------------------------
  #endif

  while (file.available()) {
    memset(riga,0,sizeof(riga));
    leggiRiga(file, riga, 79);
    //printInty(riga);
    if (riga[0]>=32) {
      //printInty("riga0>32");
      memset(tmp,0,sizeof(tmp));
      memcpy(tmp,riga,9);
  #ifdef debug
      if (slot1==0) {
  #else
      if (slot==0) {
  #endif      
        if (!(strcmp(tmp,"[mapping]"))) {
          memset(riga,0,sizeof(riga));
          leggiRiga(file, riga, 79);
          //printInty(riga);
         } else {
          Serial.println("[mapping] not found");
           error(4); // 3 error [mapping] section not found
        }
      }
      if (!(strcmp(tmp,"[memattr]"))) {
        memset(riga,0,sizeof(riga));
        leggiRiga(file, riga, 79);
        memset(tmp,0,sizeof(tmp)); 
        linepos=strcspn(riga,"-"); 
        memcpy(tmp,riga+1,linepos-1);
        ramfrom=strtoul(tmp,NULL,16);
        mapfrom[slot]=ramfrom;
        memset(tmp,0,sizeof(tmp));
        memcpy(tmp,riga+(linepos+3),5);
        ramto=strtoul(tmp,NULL,16);
        mapto[slot]=ramto; 
        maprom[slot]=ramfrom;
        addrto[slot]=maprom[slot]+(mapto[slot]-mapfrom[slot]);
        memset(tmp,0,sizeof(tmp));
        memcpy(tmp, riga + 20, 1);
        RAM8=false;
        if ((!strcmp(tmp,"8"))) {
          RAM8=true; // RAM8
          Serial.println("RAM 8");
        } else {
          Serial.println("RAM 16");
        }
        tipo[slot]=2; // RAM
        RAMused=1;
        mapdelta[slot]=maprom[slot] - mapfrom[slot]; //poi rimettere  --------------------------------
        mapsize[slot]=mapto[slot] - mapfrom[slot];  //poi rimettere  --------------------------------
        //printInty("slot++");
        slot++;
      } else {   
        memset(tmp,0,sizeof(tmp));
        memcpy(tmp,riga,1);
        if (!strcmp(tmp,"p")) {
          // [MACRO]
          memset(tmp,0,sizeof(tmp));
          memcpy(tmp,riga+2,4);
          HACK[hacks]=strtoul(tmp,NULL,16);
          memset(tmp,0,sizeof(tmp));
          memcpy(tmp,riga+7,4);
          HACK_CODE[hacks]=strtoul(tmp,NULL,16);
          hacks++;
        } else {
          //mapping
          linepos=strcspn(riga,"-");
          if ((linepos>=0) && (riga[linepos]=='-')) {
            //printInty("line>0");
            memset(tmp,0,sizeof(tmp));
            memcpy(tmp,riga+1,linepos-1);
            mapfrom[slot]=strtoul(tmp,NULL,16);  // poi rimettere --------------------------------
            if (linepos==6) {
              memset(tmp,0,sizeof(tmp));
              memcpy(tmp,riga+(linepos+3),4);
              #ifndef debug
              mapto[slot]=strtoul(tmp,NULL,16);  // poi rimettere --------------------------------
              #endif
              memset(tmp,0,sizeof(tmp));
              memcpy(tmp,riga+(linepos+11),4);  
              #ifndef debug
              maprom[slot]=strtoul(tmp,NULL,16);  // poi rimettere --------------------------------
              #endif
            } else {
              memset(tmp,0,sizeof(tmp));
              memcpy(tmp,riga+(linepos+3),5);
              #ifndef debug
              mapto[slot]=strtoul(tmp,NULL,16); // poi rimettere --------------------------------
              #endif
              memset(tmp,0,sizeof(tmp));
              memcpy(tmp,riga+(linepos+12),5);  
              #ifndef debug     
              maprom[slot]=strtoul(tmp,NULL,16);  // poi rimettere ---------------------------
              #endif
              }   
            #ifndef debug
            addrto[slot]=maprom[slot]+(mapto[slot]-mapfrom[slot]); // poi rimettere ------------------
            #endif
            linepos=strcspn(riga,"P");
            if ((linepos>0)&&(riga[linepos]!=0)) {
              #ifndef debug
              tipo[slot]=1;   //poi rimettere -------------------------------- 
              #endif
              memset(tmp,0,sizeof(tmp));
              memcpy(tmp,riga+(linepos+5),2);
              #ifndef debug
              page[slot]=strtoul(tmp,NULL,16); //poi rimettere  --------------------------------
              #endif

            } else {
              #ifndef debug
              tipo[slot]=0;  // poi rimettere -------------------------
              #endif
            }
            #ifndef debug
            slot++;
            #else
            slot1++;
            #endif
            //printInty("slot++");
          } 
        }
      }
      mapdelta[slot-1]=maprom[slot-1] - mapfrom[slot-1]; //poi rimettere  --------------------------------
      mapsize[slot-1]=mapto[slot-1] - mapfrom[slot-1];  //poi rimettere  --------------------------------
    }
    
    Serial.print(mapfrom[slot-1],HEX);Serial.print("-");
    Serial.print(mapto[slot-1],HEX);Serial.print("-");
    Serial.print(maprom[slot-1],HEX);Serial.print("-");
    Serial.print(mapdelta[slot-1],HEX);Serial.print("-");
    Serial.print(mapsize[slot-1],HEX);Serial.print("-");
    Serial.println(page[slot-1],HEX); 
    
  }
    slot=slot-1;

closefile:
	file.close();
  

}


////////////////////////////////////////////////////////////////////////////////////
//                     filelist
////////////////////////////////////////////////////////////////////////////////////

void filelist(DIR_ENTRY* en,int da, int a)
{
  char longfilename[32];
  char tmp[32];

  int base=0x17f;
  for(int i=0;i<20*20;i++) RAM[base+i*2]=0;
    for(int n = 0;n<(a-da);n++) {
		memset(longfilename,0,32);
	
	 	if (en[n+da].isDir) {
			//strcpy(longfilename,"DIR->");
			RAM[0x1000+n]=1;
			strcat(longfilename, en[n+da].long_filename);
	 	} else {
			RAM[0x1000+n]=0;
			strcpy(longfilename, en[n+da].long_filename);
      /// rimuovo il .bin
      memset(tmp,0,sizeof(tmp));
      int j=32;
      int dot=0;
      while ((longfilename[j]!='.')&&(j>0)) {
      dot=j;
      j--;
      }
      for (int i=0;i<j;i++) tmp[i]=longfilename[i];
      tmp[j]=0;      
      memcpy(longfilename,tmp,sizeof(tmp));
	 	}
  
	 	for(int i=0;i<20;i++) {
      		RAM[base+i*2+(n*40)]=longfilename[i];
	  		if ((RAM[base+i*2+(n*40)])<=20) RAM[base+i*2+(n*40)]=32;
     	}
		strcpy((char*)&nomefiles[40*n], longfilename);
	}
	RAM[0x1030]=da;RAM[0x1031]=a;RAM[0x1032]=num_dir_entries;
  }



////////////////////////////////////////////////////////////////////////////////////
//                     IntyMenu
////////////////////////////////////////////////////////////////////////////////////
void IntyMenu(int tipo) { // 1=start,2=next page, 3=prev page, 4=dir up
  int numfile=0;
  int maxfile=0;
  int ret=0;
  int rootpos[255];
  int lastpos;
  	
  switch (tipo) {
    case 1:
    /////////////////// TIPO 1 /////////////////// 
      ret = read_directory(curPath);
		  if (!(ret)) {
        curPath[0]='/';
        curPath[1]=0;
        read_directory(curPath);
        Serial.println("Dir missed, return root");
        //error(1);
      }
		  maxfile=10;
		  fileda=0;
		  if (maxfile>num_dir_entries) maxfile=num_dir_entries;
		  filea=fileda+maxfile;
		  filelist((DIR_ENTRY *)&files[0],fileda,filea);
		  //sleep_ms(1400);
      break;
    case 2:
    /////////////////// TIPO 2 /////////////////// 
   	  if (filea<num_dir_entries) {
        maxfile=10;
		    if ((filea+maxfile)>num_dir_entries) maxfile=num_dir_entries-filea;
   	    fileda=filea;
		    filea=fileda+maxfile;
    	  filelist((DIR_ENTRY *)&files[0],fileda,filea); 
      }
      break;
    case 3:    
    /////////////////// TIPO 3 /////////////////// 
   	  if (fileda>=10) {
  	    fileda=fileda-10;
		    filea=fileda+10;
		    filelist((DIR_ENTRY *)&files[0],fileda,filea);	
	    }
    break;
  }
   

}
////////////////////////////////////////////////////////////////////////////////////
//                     Directory Up
////////////////////////////////////////////////////////////////////////////////////
void DirUp() {
	int len = strlen(curPath);
	if (len>0) {
		while (len && curPath[--len] != '/');
		curPath[len] = 0;
	}
  if (len==0) curPath[0]='/';
}
////////////////////////////////////////////////////////////////////////////////////
//                     LOAD Game
////////////////////////////////////////////////////////////////////////////////////
#pragma GCC push_options
#pragma GCC optimize ("O0")
void LoadGame(){ 
  int numfile=0;
  int numErr=0;
  char longfilename[32];
  char firstbyte=0x0;

   numfile=RAM[0x899]+fileda-1;
   //numfile=3;  //poivia
   
   DIR_ENTRY *entry = (DIR_ENTRY *)&files[0];
   strcpy(longfilename,entry[numfile].long_filename);
  
  if (entry[numfile].isDir)
	{	// directory
    strcat(curPath, "/");
		strcat(curPath, entry[numfile].filename);
		IntyMenu(1);
	} else {
		memset(path,0,sizeof(path));
		strcat(path,curPath);
		strcat(path, "/");
		strcat(path,longfilename);
  
    char savepath[256];
    memcpy(savepath,path,sizeof(path)); // preserve path
  	load_cfg(path);
    
    
    load_file(savepath);  // load rom in files[]
    
    
  gpio_put(LED_PIN,false);
   	
    sleep_ms(200);
    resetCart(); // inizia con il gioco!
    sleep_ms(200);
    resetCart(); // inizia con il gioco!
    memset(RAM,0,sizeof(RAM));
    Serial.println("Game loop");
    vreg_set_voltage(VREG_VOLTAGE_1_25);
    set_sys_clock_khz(380000, true);
    delay(400);
    while(1) {
        gpio_put(LED_PIN,true);
        sleep_ms(2000);
        gpio_put(LED_PIN,false);
        sleep_ms(2000);
        Serial.print(".");
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////
//                     SETUP
////////////////////////////////////////////////////////////////////////////////////

void setup() {
  gpio_init_mask(ALL_GPIO_MASK);
  pinMode(MSYNC_PIN,INPUT_PULLDOWN);
  pinMode(RST_PIN,OUTPUT);

  bool carton=false;
 // reset interval in ms
   int t = 100;

   while (gpio_get(MSYNC_PIN) == 0 && to_ms_since_boot(get_absolute_time()) < 2000) {   // wait for Inty powerup
      if (to_ms_since_boot(get_absolute_time()) > t) {
         t += 100;
         gpio_put(RST_PIN, false);
         sleep_ms(5);
         gpio_put(RST_PIN, true);
      }
   }

   // check why loop is ended...
   if (gpio_get(MSYNC_PIN) == 1)
      carton=true;

  Serial.begin(115200);
  while ((!Serial)&&(to_ms_since_boot(get_absolute_time())) < 200);   // wait for native usb

  //else {
  flash.begin();

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("PiRTOII", "External Flash", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size() / 512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

  usb_msc.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
  
  // Init file system on the flash
  
  fs_formatted = fatfs.begin(&flash);

  if (!fs_formatted)
  {
    Serial.println("Failed to init files system, flash may not be formatted");
  }

  //} // end else, part to do if pc connected
  
  if (!carton) {
    fs_changed = true; // to print contents initially  
    Serial.println("Connected to PC");
    Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");
    while(1);
  } else {
    usb_msc.setUnitReady(false);
    Serial.println("connected to Intellivision");
    Serial.println("--------------------------");
  }

}

////////////////////////////////////////////////////////////////////////////////////
//                     MAIN LOOP
////////////////////////////////////////////////////////////////////////////////////    
  
void loop1()
{
     
}


////////////////////////////////////////////////////////////////////////////////////
//                     Inty Cart Main
////////////////////////////////////////////////////////////////////////////////////

void loop()
{
    uint32_t pins;
    uint32_t addr;
    uint32_t dataOut=0;
    uint16_t dataWrite=0;
 
  Serial.println("Loop");

	// overclocking isn't necessary for most functions - but XEGS carts weren't working without it
	// I guess we might as well have it on all the time.
  vreg_set_voltage(VREG_VOLTAGE_1_10);
  set_sys_clock_khz(270000, true);
 
  //multicore_launch_core1(core1_main);

  // Initialize the bus state variables

  busLookup[BUS_NACT]  = 4; // 100
  busLookup[BUS_BAR]   = 1; // 001
  busLookup[BUS_IAB]   = 4; // 100
  busLookup[BUS_DWS]   = 2; // 010   // test without dws handling
  busLookup[BUS_ADAR]  = 1; // 001
  busLookup[BUS_DW]    = 4; // 100
  busLookup[BUS_DTB]   = 0; // 000
  busLookup[BUS_INTAK] = 4; // 100


  gpio_init_mask(ALWAYS_OUT_MASK);
  gpio_init_mask(DATA_PIN_MASK);
  gpio_init_mask(BUS_STATE_MASK);
  gpio_set_dir_in_masked(ALWAYS_IN_MASK);
  gpio_set_dir_out_masked(ALWAYS_OUT_MASK);
  gpio_init(LED_PIN);
  gpio_put(LED_PIN,true); 
  gpio_init(RST_PIN);
  
  gpio_set_dir(MSYNC_PIN,GPIO_IN);
  gpio_pull_down(MSYNC_PIN);
  
  sleep_ms(800);
#ifdef intidebug
  RAM[0x163]=2; //1 for debug, 2 for debug with looping
#endif

  resetHigh(); 
  sleep_ms(30);
  resetLow();
  //while (gpio_get(MSYNC_PIN)==1); // wait for Inty powerup
  printf("Inty Pow-ON");
  
  gpio_put(LED_PIN,true);
  memset(ROM,0,BINLENGTH);
  
  for (int i=0;i<(sizeof(_acpirtoII)/2);i++) {
   ROM[i]=_acpirtoII[(i*2)+1] | (_acpirtoII[i*2] << 8);
  }
  memset(RAM,0,sizeof(RAM));
  
 for (int i=0; i<maxHacks; i++) {
  HACK[i]=0;
  HACK_CODE[i]=0;
 }

  slot=1; // 2 slots per splash

  //  [mapping]
  //$0000 - $0dFF = $5000
  mapfrom[0]=0x0;
  mapto[0]=0xdff;
  maprom[0]=0x5000;
  tipo[0]=0;
  page[0]=0;
  addrto[0]=0x5dff;
  mapdelta[0]=maprom[0] - mapfrom[0];
  mapsize[0]=mapto[0] - mapfrom[0];
    
 //[memattr]
 //$8000 - $9FFF = RAM 16
  RAMused=1;
  ramfrom=0x8000;
  mapfrom[1]=0x8000;
  mapto[1]=0x9fff;
  maprom[1]=0x8000;
  tipo[1]=2;
  page[1]=0;
  addrto[1]=0x9fff;
  mapdelta[1]=maprom[1] - mapfrom[1];
  mapsize[1]=mapto[1] - mapfrom[1];
  
  //sleep_ms(200);
  //resetCart();
  sleep_ms(200);
  resetCart();
  sleep_ms(1200);
  // Initial conditions 
  memset(curPath,0,sizeof(curPath));
  curPath[0]='/';
  Serial.print("Curpath:");
  Serial.println(curPath);
  sleep_ms(800);
	
  RAM[0x889]=0;
  sleep_ms(800);
	RAM[0x119]=1;
   	 
  RAM[0x119]=123;
   gpio_put(LED_PIN,true);
  
  
  IntyMenu(1);
  
  while (1) {
     cmd_executing=false;
     cmd=RAM[0x889];
     RAM[0x119]=0;
 
     if ((cmd>0)&&!(cmd_executing)) {
      switch (cmd) {
      case 1:  // read file list
        cmd_executing=true;
        RAM[0x889]=0;
    	  IntyMenu(1);
	 	    RAM[0x119]=1;
      	sleep_ms(900);
        Serial.print("new path:");Serial.println(curPath);
      	break;
      case 2:  // run file list
        cmd_executing=true;
    	  RAM[0x889]=0;
	 	    LoadGame();
		    RAM[0x119]=1;
      	sleep_ms(1400);
      	break;
      case 3:  // next page
	      cmd_executing=true;
        RAM[0x889]=0;
        IntyMenu(2);
		    RAM[0x119]=1;
        Serial.println("next page");
        sleep_ms(900);
      	break;
      case 4:  // prev page
        cmd_executing=true;
        RAM[0x889]=0; 
     	  IntyMenu(3);
		    RAM[0x119]=1;
        Serial.println("prev page");
        sleep_ms(900);
	 	    break;
	    case 5:  // up dir
        cmd_executing=true;
        RAM[0x889]=0; 
     	  DirUp();
		    IntyMenu(1);
		    RAM[0x119]=1;
        sleep_ms(900);
        Serial.print("Dirup->new path:");Serial.println(curPath);
	 	    break;
    }     
   }
  }
}
 	
#pragma GCC pop_options
