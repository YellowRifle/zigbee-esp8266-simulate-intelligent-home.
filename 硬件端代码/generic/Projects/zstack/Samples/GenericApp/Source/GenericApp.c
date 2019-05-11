/**************************************************************************************************
  Filename:       GenericApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Generic Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends "Hello World" to another "Generic"
  application every 15 seconds.  The application will also
  receive "Hello World" packets.

  The "Hello World" messages are sent/received as MSG type message.

  This applications doesn't have a profile, so it handles everything
  directly - itself.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/
  
/*********************************************************************
 * INCLUDES
 */


#include <stdio.h>
#include <string.h>

#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "OnBoard.h"

#include "GenericApp.h"
#include "DebugTrace.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

/*********************************************************************
 * MACROS
 */
#define HAL_LED_OFF   1
#define HAL_LED_ON    0

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  100
#endif

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in GenericApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t GenericApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.
devStates_t GenericApp_NwkState;


byte GenericApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GenericApp_DstAddr;
unsigned char RxBuf[SERIAL_APP_TX_MAX+1];
byte recData[1];
static uint8 SerialApp_TxLen;
int httpCon1 = 0;
int httpsend = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
void GenericApp_HandleKeys( byte shift, byte keys );
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GenericApp_SendTheMessage( void );
void rxCB(uint8 port,uint8 event);
void Delay_ustt(uint8 Time);

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void GenericApp_Init( byte task_id )
{
  
  
  GenericApp_TaskID = task_id;              //任务初始化  详情见协议栈
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  GenericApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;  //点播配置
  GenericApp_DstAddr.endPoint = 0;
  GenericApp_DstAddr.addr.shortAddr = 0;

  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( GenericApp_TaskID );
  
  halUARTCfg_t uartConfig;            //串口配置
  
  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = HAL_UART_BR_115200;       //波特路
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 64;   // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = 128;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = 128;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = 6;    // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE; // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = rxCB;
 
  
  // Update the display
 #if defined(ZDO_COORDINATOR)
    HalUARTOpen (HAL_UART_PORT_0, &uartConfig); 
    HalUARTWrite(HAL_UART_PORT_0,"UartIni04 RR\r\n", sizeof("UartInitr OK\r\n"));  //测试串口
   
    HalUARTWrite(HAL_UART_PORT_0, "AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"));     //AT指令设置模式

#endif
    
  //ZDO_RegisterForZDOMsg( GenericApp_TaskID, End_Device_Bind_rsp );
  //ZDO_RegisterForZDOMsg( GenericApp_TaskID, Match_Desc_rsp );
}

/*********************************************************************
 * @fn      GenericApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */


//事件处理逻辑
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          GenericApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
          
        case KEY_CHANGE:
          GenericApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          GenericApp_MessageMSGCB( MSGpkt );          //收到AF数据
          break;

        case ZDO_STATE_CHANGE:
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);   //网络状态变化，没有用到
          /*if ( (GenericApp_NwkState == DEV_ZB_COORD)
              || (GenericApp_NwkState == DEV_ROUTER)
              || (GenericApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending "the" message in a regular interval.
            osal_start_timerEx( GenericApp_TaskID,
                                GENERICAPP_SEND_MSG_EVT,
                              GENERICAPP_SEND_MSG_TIMEOUT );
          }*/
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in GenericApp_Init()).
  if ( events & GENERICAPP_SEND_MSG_EVT )
  {
    // Send "the" message
    GenericApp_SendTheMessage();

    // Setup to send message again
    osal_start_timerEx( GenericApp_TaskID,
                        GENERICAPP_SEND_MSG_EVT,
                      GENERICAPP_SEND_MSG_TIMEOUT );

    // return unprocessed events
    return (events ^ GENERICAPP_SEND_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      GenericApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined(BLINK_LEDS)
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            GenericApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            GenericApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      GenericApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */


//这个不用管，按键触发事件，这里没有用到，之后需要按键触发数据发送可以用
void GenericApp_HandleKeys( byte shift, byte keys )
{
  zAddrType_t dstAddr;
  
  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request for the mandatory endpoint
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = 0x0000; // Coordinator
      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(), 
                            GenericApp_epDesc.endPoint,
                            GENERICAPP_PROFID,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      // Initiate a Match Description Request (Service Discovery)
      dstAddr.addrMode = AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        GENERICAPP_PROFID,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        FALSE );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */

void Delay_ustt(uint8 Time) //us延时
{
unsigned char i;
for(i=0;i<Time;i++)
{
MicroWait(1);    //协议栈自带延时 1us 
}
}

//接收数据
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  byte data ;
     
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
#if defined(ZDO_COORDINATOR)
      // "the" message
#else 
      
     data  = pkt->cmd.Data[0];    //osal_memcpy(&data, pkt->cmd.Data, 1);
     
     if(data == 0x10)
       HalLedBlink(HAL_LED_2, 0, 50, 500);  //闪烁
    
     else if(data == 0x20)
       HalLedSet(HAL_LED_2, HAL_LED_OFF);   //关闭
   
     
     else if(data == 0x30)
       
       HalLedSet(HAL_LED_2, HAL_LED_ON);  //打开
    
     
  
      
       
      
#endif      
      break;
  }
}

/*********************************************************************
 * @fn      GenericApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */


//发送数据
void GenericApp_SendTheMessage( void )
{
  afAddrType_t P2P_DstAddr;
  P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  P2P_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  P2P_DstAddr.addr.shortAddr = 0xFFFF; //终端短地址在LCD上有显示，此处换成终端短地址就可以点播了。
  
  if ( AF_DataRequest( &P2P_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       1,                         //数据长度
                       recData,                   //数据
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  
  {
    // Successfully requested to be sent.
  }
  else
  {
    // Error occurred in request to send.
  }
}

static void rxCB(uint8 port,uint8 event)
{   
  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
#if SERIAL_APP_LOOPBACK
      (SerialApp_TxLen < SERIAL_APP_TX_MAX))
#else
      !SerialApp_TxLen)
#endif
  { 
    uint16 len = Hal_UART_RxBufLen(HAL_UART_PORT_0);  //获取串口数据长度
    SerialApp_TxLen = HalUARTRead(HAL_UART_PORT_0, RxBuf,len); //保持串口读取
    
      if ( strstr(RxBuf, "CWMODE=1") != NULL&&strstr(RxBuf, "OK") != NULL )
          {
              HalUARTWrite(HAL_UART_PORT_0,"AT+CWJAP=\"MI 6\",\"123456789q\"\r\n", strlen("AT+CWJAP=\"MI 6\",\"123456789q\"\r\n")); //连接wifi
          }
      else if ( strstr(RxBuf, "IFI GOT IP") != NULL )
          {
              HalUARTWrite(HAL_UART_PORT_0, "AT+CIPMUX=1\r\n", strlen("AT+CIPMUX=1\r\n")); //设置ip模式
             
          }
      else if (strstr(RxBuf, "AT+CIPMUX=1") != NULL  && strstr(RxBuf, "OK") != NULL )
          {

              HalUARTWrite(HAL_UART_PORT_0,"AT+CIPSERVER=1,8800\r\n", strlen("AT+CIPSERVER=1,8800\r\n")); //服务端口
           
          }
      else if ( strstr(RxBuf, "AT+CIPSERVER=1,8800") != NULL  && strstr(RxBuf, "OK") != NULL  )  
          {
           
              HalUARTWrite(HAL_UART_PORT_0, "AT+CIPSTART=0,\"TCP\",\"206.253.164.235\",10086\r\n", strlen("AT+CIPSTART=0,\"TCP\",\"206.253.164.235\",10086\r\n")); //连接服务器
          
          }
      else if (strstr(RxBuf, "ONNECT") != NULL  && strstr(RxBuf, "OK") != NULL )
          {
           
               HalUARTWrite(HAL_UART_PORT_0, "AT+CIPSEND=0,9\r\n", strlen("AT+CIPSEND=0,9\r\n")); //发送确认数据
          }
      else if (strstr(RxBuf, "CIPSEND=0,9") != NULL  && strstr(RxBuf, "OK") != NULL )
           {
              HalUARTWrite(HAL_UART_PORT_0, "CONN_9527\r\n", strlen("CONN_9527\r\n"));   //确认数据，用于服务端验证是否为esp8266 
          }
      else if(strstr(RxBuf, "IPD,0,") != NULL)  //收到服务器传来的数据
      { 
        
        if(RxBuf[11]== 0x01 && RxBuf[12] == 0x00 && RxBuf[13] == 0x00)  //闪烁指令
        {  //
          HalUARTWrite(HAL_UART_PORT_0,&RxBuf[11], 1);
          recData[0]= 0x10;
          GenericApp_SendTheMessage();
          HalUARTWrite(HAL_UART_PORT_0,&recData[0], 1);
        }
        else if (RxBuf[11] == 0x00 && RxBuf[12] == 0x01 && RxBuf[13] == 0x00) //关闭指令
        {
          recData[0] = 0x20;
          GenericApp_SendTheMessage();
          HalUARTWrite(HAL_UART_PORT_0,recData, 1);
        }
        else if (RxBuf[11] == 0x00 && RxBuf[12] == 0x00 && RxBuf[13] == 0x01) //开启指令
        {
          *recData = 0x30;
          GenericApp_SendTheMessage();
          HalUARTWrite(HAL_UART_PORT_0,recData, 1);
        }
      }
      SerialApp_TxLen=0;
    }
  }



/*********************************************************************
*********************************************************************/
