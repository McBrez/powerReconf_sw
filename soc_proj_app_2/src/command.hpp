#include <xil_types.h>

enum CommandType {
	TRIGGER_MEASUREMENT = 0x0,
	TRIGGER_RECONF = 0x1,
	INVALID = 0xFF
};

class Command {
private:
	u8 frame;
	u8 module;
	u8 config;
	CommandType command;
public:
	Command();
	static const u8 START_BYTE = 0x02;
	static const u8 END_BYTE = 0x0D;

	static Command decode(u8* buffer, u32 bufferLen);
	static void encode_agingSensorValues(u8* buffer, u8 frame, u8 module, u32 sensorValue, u32 temp);
	u8 getFrame();
	CommandType getCommand();
	u8 getModule();
	u8 getConfig();
};
