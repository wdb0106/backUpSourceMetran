;;*****************************************************************************
;;*****************************************************************************
;;  FILENAME:   MOSI.asm
;;  Version: 1.2, Updated on 2009/7/10 at 10:45:0
;;  Generated by PSoC Designer 5.0.985.0
;;
;;  DESCRIPTION: LED user module for 22/24/27/29xxx PSoC family of devices.
;;
;;
;;  NOTE: User Module APIs conform to the fastcall16 convention for marshalling
;;        arguments and observe the associated "Registers are volatile" policy.
;;        This means it is the caller's responsibility to preserve any values
;;        in the X and A registers that are still needed after the API functions
;;        returns. For Large Memory Model devices it is also the caller's 
;;        responsibility to perserve any value in the CUR_PP, IDX_PP, MVR_PP and 
;;        MVW_PP registers. Even though some of these registers may not be modified
;;        now, there is no guarantee that will remain the case in future releases.
;;-----------------------------------------------------------------------------
;;  Copyright (c) Cypress MicroSystems 2005. All Rights Reserved.
;;*****************************************************************************
;;*****************************************************************************

include "MOSI.inc"
include "memory.inc"

export _MOSI_Start
export  MOSI_Start

export _MOSI_Stop
export  MOSI_Stop

export _MOSI_On
export  MOSI_On

export _MOSI_Off
export  MOSI_Off

export _MOSI_Switch
export  MOSI_Switch

export _MOSI_Invert
export  MOSI_Invert

export _MOSI_GetState
export  MOSI_GetState


AREA UserModules (ROM, REL)


.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: MOSI_Start(void)
;  FUNCTION NAME: MOSI_Stop(void)
;
;  FUNCTION NAME: MOSI_Switch(void)
;
;  DESCRIPTION: ( Switch )
;     Turn LED on or off     
;
;  DESCRIPTION: ( Start, Stop )
;     Turn LED off                       
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS:  ( Switch )
;     A => If 0, turn off LED, if > 0 turn on LED
;
;  ARGUMENTS:  ( Start, Stop )
;      None
;
;  RETURNS:  none
;
;  SIDE EFFECTS:
;    REGISTERS ARE VOLATILE: THE A AND X REGISTERS MAY BE MODIFIED!
;
;-----------------------------------------------------------------------------
_MOSI_On:
 MOSI_On:
   mov  A,0x01
   jmp  MOSI_Switch 

_MOSI_Start:
 MOSI_Start:
_MOSI_Stop:
 MOSI_Stop:
_MOSI_Off:
 MOSI_Off:
   mov  A,0x00

_MOSI_Switch:
 MOSI_Switch:
   RAM_PROLOGUE RAM_USE_CLASS_4
   RAM_PROLOGUE RAM_USE_CLASS_2
   RAM_SETPAGE_CUR >Port_1_Data_SHADE

   or   A,0x00                                   ; Check mode
   jz   .Turn_Off_LED

.Turn_On_LED:
IF(1)                                            ; Active High Digit Drive
   or   [Port_1_Data_SHADE],MOSI_PinMask
ELSE                                             ; Active Low Digit Drive
   and  [Port_1_Data_SHADE],~MOSI_PinMask
ENDIF
   jmp  .Switch_LED

.Turn_Off_LED:
IF(1)                      ; Active High Digit Drive
   and  [Port_1_Data_SHADE],~MOSI_PinMask
ELSE                              ; Active Low Digit Drive
   or   [Port_1_Data_SHADE],MOSI_PinMask
ENDIF

.Switch_LED:
   mov  A,[Port_1_Data_SHADE]
   mov  reg[MOSI_PortDR],A

   RAM_EPILOGUE RAM_USE_CLASS_2
   RAM_EPILOGUE RAM_USE_CLASS_4
   ret
.ENDSECTION



.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: MOSI_Invert(void)
;
;  DESCRIPTION:
;     Invert state of LED                               
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS:  none
;
;  SIDE EFFECTS:
;    REGISTERS ARE VOLATILE: THE A AND X REGISTERS MAY BE MODIFIED!
;
;-----------------------------------------------------------------------------
_MOSI_Invert:
 MOSI_Invert:
   RAM_PROLOGUE RAM_USE_CLASS_4
   RAM_PROLOGUE RAM_USE_CLASS_2
   RAM_SETPAGE_CUR >Port_1_Data_SHADE

   xor  [Port_1_Data_SHADE],MOSI_PinMask
   mov  A,[Port_1_Data_SHADE]
   mov  reg[MOSI_PortDR],A

   RAM_EPILOGUE RAM_USE_CLASS_2
   RAM_EPILOGUE RAM_USE_CLASS_4
   ret
.ENDSECTION

.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: MOSI_GetState(void)
;
;  DESCRIPTION:
;     Get state of LED
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS:  
;    State of LED   1 = ON,  0 = OFF
;
;  SIDE EFFECTS:
;    REGISTERS ARE VOLATILE: THE A AND X REGISTERS MAY BE MODIFIED!
;
;-----------------------------------------------------------------------------
_MOSI_GetState:
 MOSI_GetState:
   RAM_PROLOGUE RAM_USE_CLASS_4
   RAM_PROLOGUE RAM_USE_CLASS_2
   RAM_SETPAGE_CUR >Port_1_Data_SHADE

   mov   A,[Port_1_Data_SHADE]         ; Get shade value
IF(1)                                  ; Active High Digit Drive
   // Nothing for now
ELSE                                   ; Active Low Digit Drive
   cpl   A                             ; Invert bit if Active low
ENDIF
   and   A,MOSI_PinMask                ; Mask off the trash
   jz    .End_LED_GS                   ; If zero, we're done
   mov   A,0x01                        ; Return a 1 no mater what the mask is.

.End_LED_GS:
   RAM_EPILOGUE RAM_USE_CLASS_2
   RAM_EPILOGUE RAM_USE_CLASS_4
   ret
.ENDSECTION