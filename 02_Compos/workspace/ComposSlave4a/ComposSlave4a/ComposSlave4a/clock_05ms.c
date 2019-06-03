/*******************************************************************************
*                                                                               
*    ﾌｧｲﾙ名 ：   clock_05ms.c                                                   
*                                                                               
*    概要 ：                                                                    
*          ソフトウェアタイマ割り込みのプログラム                               
*                                                                               
*    公開API :                                                                  
*                                                                               
*    備考 :                                                                     
*                                                                               
*                                                                               
*    変更履歴 :                                                                 
*            日  付    内    容                            Ver No.              
*            --------  ----------------------------------- -------              
*            10.01.20  初版作成                            0.00                 
*            11.12.08  1版作成                             1.00                 
*                                                                               
*******************************************************************************/

#include "clock_05ms.h"		// 10,100msecインターバルフラグ
#include "PSoCAPI.h"		// PSoC API definitions for all User Modules

/******************************************************************************
*** RAM・ROM
******************************************************************************/

unsigned char   flag100ms;		// 100msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
unsigned char   flag10ms;		// 10msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
unsigned char   flag05ms;		// 0.5msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
unsigned char   tmp_100ms;		// システムタイマー 100msecタイマ生成用
unsigned char   tmp_10ms;		// システムタイマー 10msecタイマ生成用

/******************************************************************************
*** プログラムコード
******************************************************************************/


/******************************************************************************
##
## Name    :clock_init
## Function:ソフトウェアタイマ割り込み初期化
## Input   :なし
## Output  :なし
## Memo    :なし
##
******************************************************************************/
void clock_init( void )
{
	flag100ms = 0;				// 100msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
	flag10ms = 0;				// 10msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
	flag05ms = 0;				// 0.5msec周期フラグ(セット:割込処理 , クリア:ユーザー処理)
	tmp_100ms = 0;				// システムタイマー 100msecタイマ生成用
	tmp_10ms = 0;				// システムタイマー 10msecタイマ生成用
}


/******************************************************************************
##
## Name    :PWM8_1_ISR2
## Function:ソフトウェアタイマ割り込み
## Input   :なし
## Output  :なし
## Memo    :なし
##
******************************************************************************/
#pragma interrupt_handler PWM8_1_ISR2
#pragma nomac
void PWM8_1_ISR2( void )
{
    flag05ms = 1;				// 0.5msec周期フラグセット
  	tmp_10ms++;					// カウントアップ(1msec毎)
	if(tmp_10ms >= 20)			// 10msec経過
	{
        flag10ms = 1;			// 10msec周期フラグセット
   	    tmp_10ms = 0;			//
	}
  	tmp_100ms++;				// カウントアップ(1msec毎)
	if(tmp_100ms >= 200)		// 100msec経過
	{
        flag100ms = 1;			// 100msec周期フラグセット
   	    tmp_100ms = 0;			//
	}
}
#pragma usemac

