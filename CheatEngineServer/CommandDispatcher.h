#pragma once

#include "CheatEngine.h"

enum class CommandReturn {
	CR_OK, CR_FAIL_UNHANDLED, CR_FAIL_ALLOC, CR_FAIL_NETWORK, CR_FAIL_KMEM, CR_FAIL_OTHER
};

int CheckForAndDispatchCommand(CEConnection& con);

enum CommandReturn DispatchCommand(CEConnection& con, char command);