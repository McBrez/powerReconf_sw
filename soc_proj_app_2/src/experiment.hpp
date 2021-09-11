#include <xil_types.h>

namespace experiment {

// AXI address offsets.
const UINTPTR COMMAND_REGISTER_OFFSET = 0x0;
const UINTPTR DATA_REGISTER_OFFSET = 0x4;
const UINTPTR COMMIT_REGISTER_OFFSET = 0x8;

// Experiment command identifiers.
const u16 WRITE_OP = 0;
const u16 READ_RESULT = 2;
const u16 TRIGGER_MEASUREMENT = 3;
const u16 READ_READY = 4;
const u16 READ_AGING_SENSOR = 5;
const u16 MODULE_SELECT = 6;

// Experiment operand/result indices.
const u8 OPERAND_IDX_X = 0;
const u8 OPERAND_IDX_Y = 1;
const u8 RESULT_IDX_OUTR = 0;
const u8 RESULT_IDX_OUTI = 1;
const u8 RESULT_IDX_OVERFLOW = 2;

class Experiment {
public:
	Experiment(UINTPTR baseAddress, u16 frameCount);
	void writeOperand(u32 operand, u8 frame, u8 index);
	u32 readResult(u8 frame, u8 index);
	void writeToData(u32 value);

private:
	void write(u32 value, u16 operation, u8 frame, u8 index);
	u32 read(u16 operation, u8 frame, u8 index);
	u32 composeOperation(u8 frame, u8 index, u16 operation);
	UINTPTR baseAddress;
	u16 frameCount;
};

} // experiment
