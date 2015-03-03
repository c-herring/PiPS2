/* Bit Operation Macros */
#define SET(x,y) (x|=(1<<y))	// Set bit y in x
#define CLR(x,y) (x&=(~(1<<y))) // Clear bit y in x
#define CHK(x,y) (x & (1<<y)) 	// Check if bit y in x is set
#define TOG(x,y) (x^=(1<<y)) 	// Toggle bit y in x

/* Delays */
#define CLK_DELAY  		4
#define BYTE_DELAY 		3
#define CMD_DELAY 		1
#define MAX_READ_DELAY 	10

/* Maximum number of init tries */
#define MAX_INIT_ATTEMPT 	50

/* Controller Modes - From: http://www.lynxmotion.com/images/files/ps2cmd01.txt */
#define DIGITALMODE 	0x41
#define ANALOGMODE 		0x73
#define ALLPRESSUREMODE	0x79
#define DS2NATIVEMODE	0xF3

/* Button Masks */
// 		From data bit 0 (PS2data[3])
#define BTN_SELECT 		0
#define	BTN_LEFT_JOY	1
#define	BTN_RIGHT_JOY	2
#define	BTN_START		3
#define	BTN_UP			4
#define	BTN_RIGHT		5
#define	BTN_DOWN		6
#define	BTN_LEFT		7
// 		From data bit 1 (PSdata[4])
#define	BTN_L2			0
#define	BTN_R2			1
#define	BTN_L1			2
#define	BTN_R1			3
#define	BTN_TRIANGLE	4
#define	BTN_CIRCLE		5
#define	BTN_X			6
#define	BTN_SQUARE		7

/* Byte Numbers of PSdata[] For Button Pressures */
#define	PRES_RIGHT 		10
#define	PRES_LEFT 		11
#define	PRES_UP 		12
#define	PRES_DOWN 		13
#define	PRES_TRIANGLE 	14
#define	PRES_CIRCLE 	15
#define	PRES_X		 	16
#define	PRES_SQUARE 	17
#define	PRES_L1		 	18
#define	PRES_R1		 	19
#define	PRES_L2		 	20
#define	PRES_R2		 	21

/* Controller Commands */
static unsigned char 	enterConfigMode[]={0x01,0x43,0x00,0x01,0x00};
static unsigned char 	set_mode_analog_lock[]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static unsigned char 	exitConfigMode[]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static unsigned char 	type_read[]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};
static unsigned char 	config_AllPressure[]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};

/* Class declaration */
class PIPS2 {
	public:
		unsigned char initializeController(int _commandPin, int _dataPin, int _clkPin, int _attnPin);
		int reInitializeController(char controllerMode);
		int reInitializeController();
		unsigned char transmitByte(char byte);
		void transmitCmdString(unsigned char *string, int length);
		void readPS2(void);
		unsigned char PS2data[21];
		void getChangedStates(char *outputChangedStates);
		
	private:
		char controllerMode;
		int commandPin;
		int dataPin;
		int attnPin;
		int clkPin;
		unsigned int readDelay;
		char btnLastState[2];
		char btnChangedState[2];
};