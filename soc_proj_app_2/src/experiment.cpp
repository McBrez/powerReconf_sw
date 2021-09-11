#include "experiment.hpp"

#include <xil_io.h>

using namespace experiment;

Experiment::Experiment(UINTPTR baseAddress, u16 frameCount):
	baseAddress(baseAddress),
	frameCount(frameCount)
{}

void Experiment::writeOperand(u32 operand, u8 frame, u8 index) {

	this->write(operand, WRITE_OP, frame, index);
}

void Experiment::write(u32 value, u16 operation, u8 frame, u8 index) {

	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x0);
	Xil_Out32(
			this->baseAddress + COMMAND_REGISTER_OFFSET,
			this->composeOperation(frame, index, operation));
	Xil_Out32(
			this->baseAddress + DATA_REGISTER_OFFSET,
			value);
	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x1);
}

u32 Experiment::composeOperation(u8 frame, u8 index, u16 operation) {
	u32 frame_u32 = frame;
	u32 index_u32 = index;

	frame_u32 = frame_u32 << 16;
	index_u32 = index_u32 << 24;

	return index_u32 + frame_u32 + operation;
}

u32 Experiment::readResult(u8 frame, u8 index) {
	return this->read(READ_RESULT, frame, index);
}

u32 Experiment::read(u16 operation, u8 frame, u8 index) {
	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x0);
	Xil_Out32(
			this->baseAddress + COMMAND_REGISTER_OFFSET,
			this->composeOperation(frame, index, operation));
	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x1);
	return Xil_In32(this->baseAddress + DATA_REGISTER_OFFSET);
}

void Experiment::writeToData(u32 value) {
	Xil_Out32(
			this->baseAddress + DATA_REGISTER_OFFSET,
			value);
	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x1);
	Xil_Out32(
			this->baseAddress + COMMIT_REGISTER_OFFSET,
			0x0);
}
