#include "command.hpp"

Command::Command() :
		frame(0xFF),
		module(0xFF),
		config(0xFF)
{}

Command Command::decode(u8* buffer, u32 bufferLen) {
	// Look for start byte.
	int pos=0;
	bool startByteFound = false;
	Command retCommand;

	for(pos; pos < bufferLen; pos++){
		if (buffer[pos] == Command::START_BYTE) {
			startByteFound = true;
			pos++;
			break;
		}
	}
	if(!startByteFound || bufferLen < pos) {
		retCommand = Command();
		retCommand.command = CommandType::INVALID;
		return retCommand;
	}

	// Get message content.
	u8 commandByte = buffer[pos];
	retCommand = Command();
	switch(commandByte) {
	case TRIGGER_MEASUREMENT: {
		retCommand.command = TRIGGER_MEASUREMENT;
		retCommand.frame = buffer[pos+1];
		retCommand.module = buffer[pos+2];
		break;
	}
	case TRIGGER_RECONF: {
		retCommand.command = TRIGGER_RECONF;
		retCommand.frame = buffer[pos+1];
		retCommand.config = buffer[pos+2];
		break;
	}
	case INVALID: {
		retCommand.command = CommandType::INVALID;
		break;
	}
	}
	return retCommand;
}

u8 Command::getFrame() {
	return this->frame;
}

CommandType Command::getCommand() {
	return this->command;
}

u8 Command::getModule() {
	return this->module;
}

u8 Command::getConfig() {
	return this->config;
}

void Command::encode_agingSensorValues(u8* buffer, u8 frame, u8 module, u32 sensorValue, u32 temp) {
	buffer[0] = 0x02;
	buffer[1] = frame;
	buffer[2] = ',';
	buffer[3] = module;
	buffer[4] = ',';
	buffer[5] = (sensorValue & 0xFF000000) >> 24;
	buffer[6] = (sensorValue & 0x00FF0000) >> 16;
	buffer[7] = (sensorValue & 0x0000FF00) >> 8;
	buffer[8] = (sensorValue & 0x000000FF);
	buffer[9] = (temp & 0xFF000000) >> 24;
	buffer[10] = (temp & 0x00FF0000) >> 16;
	buffer[11] = (temp & 0x0000FF00) >> 8;
	buffer[12] = (temp & 0x000000FF);
	buffer[13] = 0x00;
}

