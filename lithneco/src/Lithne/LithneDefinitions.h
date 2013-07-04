#ifndef LithneDefinitions_h
#define LithneDefinitions_h

/* NETWORK DEFINITIONS */
#define BROADCAST 255			//The standard ID for a broadcast message
#define COORDINATOR 0			//The standard ID for message to the coordinator
#define UNKNOWN_ADD16 0xFFFE	//Defines an unknown 16-bit address
#define UNKNOWN_16B 0xFFFE		//Defines an unknown 16-bit address
#define UNKNOWN_ADD64 0xFFFE	//Defines the LSB of an unknown 64-bit address
#define UNKNOWN_64B 0xFFFE		//Defines the LSB of an unknown 64-bit address
#define UNKNOWN_NODE_ID 254		//Defines an unknown node ID
#define UNKNOWN_NODE 0xFE		//Defines an unknown node
#define UNKNOWN_PAN_ID 0xFFFE	//Defines an unknown PAN ID
#define UNKNOWN_STATUS 0xFFFE	//Defines an unknown status
#define COORDINATOR_ADDR 0x0	//The standard LSB of the 64-bit coordinator
#define BROADCAST_ADDR 0xFFFF	//The standard LSB of the 64-bit broadcast

/*			NETWORK MESSAGES			*/
/*	Provides a specific type of error. Some error types are
	defined by the Lithne library. Own errors can be defined.
	uint16_t: error type.
*/
// #define ERROR_MESSAGE 255

/*	Introduce self to the network includes two arguments
	uint16_t: 16-bit address of the node
	boolean : true if the sender would like to receive a return 
			  message from the recipient.
*/
#define HELLO_WORLD 101
#define NO_REPLY 0
#define REPLY 1

/* 		LIGHT RELATED DEFINITIONS		 */
/*	The function SWITCH (1) can have three arguments
	OFF, ON or TOGGLE	*/
#define SWITCH 1
#define OFF 0
#define TURN_OFF 0
#define ON 1
#define TURN_ON 1
#define TOGGLE 2
/*	The function INTENSITY (2) is used to manipulate
	the intensity of the complete object. This needs
	to be handled in your own function table.	*/
#define INTENSITY 2
#define INTENSITY_TO 2
#define SET_INTENSITY 2
/*	The function CCT (3) is used to manipulate the
	colour temperature of the light object	*/
#define CCT 3
#define CCT_TO 3
#define SET_CCT 3
/*	The function RGB (4) is used to manipulate
	the colour of the light object and requires
	three arguments: R (uint8_t), G (uint8_t), 
	B (uint8_t).	*/
#define RGB 4
#define RGB_TO 4
#define SET_RGB 4
/*	The function HSB (5) is used to manipulate
	the colour of the light object and requires
	three arguments: H(ue) (uint8_t), S(aturation)
	(uint8_t), B(rightness) (uint8_t).	*/
#define HSB 5
#define HSB_TO 5
#define SET_HSB 5
/*	The function LIGHT_PARAMETERS (6) is used
	to manipulate all light parameters at once.	*/
#define LIGHT_PARAMETERS_TO 6
#define SET_LIGHT_PARAMETERS 6
#define LIGHT_PARAMETERS 6
/*	The function DEMO (10) is used to start demo
	behaviour of the light object	*/
#define DEMO 10

/*			SCOPE DEFINITIONS			*/
#define MAX_SCOPES	10
/*	Use this if you do not wish to define a scope	*/
#define NO_SCOPE 1
/*	To create a new group use one of the following
	definitions.	*/
#define ADD_GROUP 20
#define SET_GROUP 20
#define ADD_TO_GROUP 20
/*	To remove the node from a group.	*/
#define REMOVE_GROUP 21
#define REMOVE_FROM_GROUP 21
/*	To change or rename a group	*/
#define RENAME_GROUP 22
#define CHANGE_GROUP 22
/*	To retrieve the value of a group	*/
#define GET_GROUP 23


#define HELLO_NODE 102		//Return the favour
#define REQUEST_INFO 110	//Request the most basic info of the node
#define SUBMIT_INFO 111		//Submit the most basic info of the node
#define REQUEST_BOOL 112	//Request for a boolean
#define SUBMIT_BOOL 113		//The response of requesting a boolean
#define REQUEST_VAR_8 114	//Request for an 8-bit variable
#define	SUBMIT_VAR_8 115	//The response of requesting an 8-bit variable
#define REQUEST_VAR_16 116	//Request for an 16-bit variable
#define SUBMIT_VAR_16 117	//The response of requesting a 16-bit variable
#define GET_FIRMWARE 118	//Requests the firmware information from the node
#define FIRMWARE_INFO 119	//Reports the info regarding the firmware

/*	ERROR DEFINITIONS	*/

/* META DEFINITIONS */
#define STD_DURATION 1000
#define SWITCH_DURATION 100
#define DEBUG 255
//#define FIRMWARE 102

#endif