	.set noreorder

#include "pspimport.s"

	IMPORT_START "sceNetIfhandle_lib",0x40090011
	IMPORT_FUNC  "sceNetIfhandle_lib",0x1560F143,sceNetMCopyback
