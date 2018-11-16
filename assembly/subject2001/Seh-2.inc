comment 'These two SEH macros use stack to store values that should be saved'  	
InstSehFrame MACRO Safe_Place,SehHandler,HandlerInfo,ContinueFlag
	PUSH EBP;ERR+14h save ebp (being ebp at safe-place) 
	PUSH ContinueFlag  ;ERR+10h area for flags 
	push HandlerInfo;ERR+0Ch information for handler 
	PUSH OFFSET Safe_Place;ERR+8h new eip at safe-place
	PUSH OFFSET SehHandler  ;ERR+4h address of handler
	ASSUME FS : NOTHING  
	PUSH FS:[0]	            ;ERR+0h keep next ERR up the chain 
	MOV  FS:[0], ESP        ;point to ERR just made on the stack 
ENDM

KillSehFrame MACRO
	POP  FS:[0]
	ADD  ESP, 20;restore next ERR up the chain
ENDM
