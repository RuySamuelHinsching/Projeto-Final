#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "cmsis_os2.h" // CMSIS-RTOS 
 
// includes da biblioteca driverlib 
#include "inc/hw_memmap.h" 
#include "driverlib/gpio.h" 
#include "driverlib/uart.h" 
#include "driverlib/sysctl.h" 
#include "driverlib/pin_map.h" 
#include "utils/uartstdio.h" 
#include "system_TM4C1294.h" 
 
enum Estado{pega_mensagem = 0, manda_comando, aguarda_processo};

osThreadId_t ElevadorEsquerda_thread_id, ElevadorCentral_thread_id, ElevadorDireita_thread_id, ControladorElevador_thread_id; 
osMutexId_t uart_id; 

osMessageQueueId_t filaElevador[3];


typedef struct elevador
{
  int andarAtual;
}elevador;

elevador Esquerdo, Central, Direito;

int tickAnterior = 0;

 
const osThreadAttr_t thread1_attr = { 
  .name = "Thread 1" 
}; 
 
const osThreadAttr_t thread2_attr = { 
  .name = "Thread 2" 
}; 

const osThreadAttr_t thread3_attr = { 
  .name = "Thread 3" 
}; 
 
const osThreadAttr_t thread0_attr = { 
  .name = "Thread 0" 
}; 
 
//---------- 
// UART definitions 
extern void UARTStdioIntHandler(void); 
 
void UARTInit(void){ 
  // Enable UART0 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)); 
 
  // Initialize the UART for console I/O. 
  UARTStdioConfig(0, 115200, SystemCoreClock); 
 
  // Enable the GPIO Peripheral used by the UART. 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)); 
 
  // Configure GPIO Pins for UART mode. 
  GPIOPinConfigure(GPIO_PA0_U0RX); 
  GPIOPinConfigure(GPIO_PA1_U0TX); 
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1); 
} // UARTInit 
 
void UART0_Handler(void){ 
  UARTStdioIntHandler(); 
} // UART0_Handler 
//---------- 

// osRtxIdleThread 
__NO_RETURN void osRtxIdleThread(void *argument){ 
  (void)argument; 
   
  while(1){ 
    asm("wfi"); 
  } // while 
} // osRtxIdleThread 


/**-----------------------------------------------------------------**/

__NO_RETURN void ControladorElevador(void *arg){ 
  uint32_t tick; 
   
  int tamanhoBuffer = 10; 
  char buffer[15]; 
  
  while (1)
  {
    tick = osKernelGetTickCount(); 
    osMutexAcquire(uart_id, 15);
    UARTgets(buffer , tamanhoBuffer); 
      
    UARTFlushRx();
    osMutexRelease(uart_id);
      
    osDelayUntil(tick + 1000);
      
    int andar = 0;
      
      if ((int)buffer[1] >= 49 && (int)buffer[1] <= 57) //calculo do andar
      {
        andar = 1; 
      }
      else
      {
        andar = 0;
      }
      
      if (buffer[0] == 'e') 
      {
        if (andar) 
        {
          Esquerdo.andarAtual =((buffer[1]-0x30)*1);
        }
        else
        {
          osMessageQueuePut(filaElevador[0], &buffer, 0, 0);
        }
        
      }
      else if (buffer[0] == 'c')
      {
        if (andar)
        {
          Central.andarAtual =((buffer[1]-0x30)*1);
        }
        else
        {
          osMessageQueuePut(filaElevador[1], &buffer, 0, 0);
          printf("botão\n");
        }
        
      }
      else if (buffer[0] == 'd')
      {
        if (andar)
        {
          Direito.andarAtual =((buffer[1]-0x30)*1);
        }
        else
        {
          osMessageQueuePut(filaElevador[2], &buffer, 0, 0);
        }
      }
      
      tickAnterior = tick;
  }
  
}

/**-----------------------------------------------------------------**/

__NO_RETURN void ElevadorEsquerda(void *arg)
{ 

  char buffer[15];
  
  enum Estado estado = pega_mensagem;
  int proximoAndar;
  
  while (1)
  {
    switch (estado)
    {
      case pega_mensagem:
        if (osMessageQueueGetCount(filaElevador[0]) != 0)
        {
          osMessageQueueGet (filaElevador[0], &buffer, NULL, osWaitForever);
          estado = manda_comando;
        
        }       
        break;
      case manda_comando:
        if (buffer[1] == 'I')
        {
          proximoAndar =(int) buffer[2] - 97;
        }
        else if (buffer[1]=='E')
        {
          proximoAndar =((buffer[2]-0x30)*10)+((buffer[3]-0x30));
          
        }
        osMutexAcquire(uart_id, 15);
        UARTprintf("ef\r", osThreadGetName(osThreadGetId()), 15); 
        UARTFlushTx(true);
        osMutexRelease(uart_id); 
        
        if (proximoAndar > Esquerdo.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("es\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          else if (proximoAndar < Esquerdo.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("ed\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          
          estado = aguarda_processo;
        
        break;
        case aguarda_processo:
        {
          int tick = 0;
          do
          {
            tick = osKernelGetTickCount(); 
            
             if (tick - tickAnterior > 1000)
             {
                if (proximoAndar > Esquerdo.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("es\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
                else if (proximoAndar < Esquerdo.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("ed\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
             }
            
            if (proximoAndar == Esquerdo.andarAtual)
            {
              osMutexAcquire(uart_id, 15);
              UARTprintf("ep\r", osThreadGetName(osThreadGetId()), 15); 
              UARTprintf("ea\r", osThreadGetName(osThreadGetId()), 15); 
              UARTFlushTx(true);
              osMutexRelease(uart_id); 
              estado = pega_mensagem;
            }
            tickAnterior = tick;
          } while (estado == aguarda_processo);
          
        
        
        break;
        }
    }
    
  }
} 
 
/**-----------------------------------------------------------------**/

__NO_RETURN void ElevadorCentral(void *arg)
{ 
  char buffer[15];
  
  enum Estado estado = pega_mensagem;
  int proximoAndar;
  
  while (1)
  {
    switch (estado)
    {
      case pega_mensagem:
        if (osMessageQueueGetCount(filaElevador[1]) != 0)
        {
          osMessageQueueGet (filaElevador[1], &buffer, NULL, osWaitForever);
          estado = manda_comando;
        
        }
        break;
      case manda_comando:
        if (buffer[1] == 'I')
        {
          proximoAndar =(int) buffer[2] - 97;
        }
        else if (buffer[1]=='E')
        {
          proximoAndar =((buffer[2]-0x30)*10)+((buffer[3]-0x30));
          
        }
        
        osMutexAcquire(uart_id, 15);
        UARTprintf("cf\r", osThreadGetName(osThreadGetId()), 15); 
        UARTFlushTx(true);
        osMutexRelease(uart_id); 
        
        if (proximoAndar > Central.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("cs\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          else if (proximoAndar < Central.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("cd\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          
          estado = aguarda_processo;
        
          break;
        case aguarda_processo:
        {
          int tick = 0;
          do
          {
            tick = osKernelGetTickCount(); 
            
             if (tick - tickAnterior > 1000)
             {
                if (proximoAndar > Central.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("cs\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
                else if (proximoAndar < Central.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("cd\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
             }
            
            if (proximoAndar == Central.andarAtual)
            {
              osMutexAcquire(uart_id, 15);
              UARTprintf("cp\r", osThreadGetName(osThreadGetId()), 15); 
              
              UARTprintf("ca\r", osThreadGetName(osThreadGetId()), 15); 
              UARTFlushTx(true);
              osMutexRelease(uart_id); 
              estado = pega_mensagem;
            }
            tickAnterior = tick;
          } while (estado == aguarda_processo);
          
        
        
        break;
        }
    }
  }
} 
 
/**-----------------------------------------------------------------**/

__NO_RETURN void ElevadorDireita(void *arg)
{ 

  char buffer[15];
  
  enum Estado estado = pega_mensagem;
  int proximoAndar;
  
  while (1)
  {
    switch (estado)
    {
      case pega_mensagem:
        if (osMessageQueueGetCount(filaElevador[2]) != 0)
        {
          osMessageQueueGet (filaElevador[2], &buffer, NULL, osWaitForever);
          estado = manda_comando;
        
        }       
        break;
      case manda_comando:
        if (buffer[1] == 'I')
        {
          proximoAndar =(int) buffer[2] - 97;
        }
        else if (buffer[1]=='E')
        {
          proximoAndar =((buffer[2]-0x30)*10)+((buffer[3]-0x30));
          
        }
        
        osMutexAcquire(uart_id, 15);
        UARTprintf("df\r", osThreadGetName(osThreadGetId()), 15); 
        UARTFlushTx(true);
        osMutexRelease(uart_id); 
        
        if (proximoAndar > Direito.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("ds\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          else if (proximoAndar < Direito.andarAtual)
          {
            osMutexAcquire(uart_id, 15);
            UARTprintf("dd\r", osThreadGetName(osThreadGetId()), 15); 
            UARTFlushTx(true);
            osMutexRelease(uart_id); 
          }
          
          estado = aguarda_processo;
        
        break;
        case aguarda_processo:
        {
          int tick = 0;
          do
          {
            tick = osKernelGetTickCount(); 
            
             if (tick - tickAnterior > 1000)
             {
                if (proximoAndar > Direito.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("ds\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
                else if (proximoAndar < Direito.andarAtual)
                {
                  osMutexAcquire(uart_id, 15);
                  UARTprintf("dd\r", osThreadGetName(osThreadGetId()), 15); 
                  UARTFlushTx(true);
                  osMutexRelease(uart_id); 
                }
             }
            
            if (proximoAndar == Direito.andarAtual)
            {
              osMutexAcquire(uart_id, 15);
              UARTprintf("dp\r", osThreadGetName(osThreadGetId()), 15); 
              UARTprintf("da\r", osThreadGetName(osThreadGetId()), 15); 
              UARTFlushTx(true);
              osMutexRelease(uart_id); 
              estado = pega_mensagem;
            }
            tickAnterior = tick;
          } while (estado == aguarda_processo);
          
        
        
        break;
        }
    }
    
  }
} // thread2 

/**-----------------------------------------------------------------**/

void main(void)
{
  int tamMem = 15;
  
  Esquerdo.andarAtual = 0;
  Central.andarAtual = 0;
  Direito.andarAtual = 0;
  
  UARTInit(); 
  
  if(osKernelGetState() == osKernelInactive) 
     osKernelInitialize(); 
  
  filaElevador[0] = osMessageQueueNew(tamMem, sizeof(char*), NULL);
  filaElevador[1] = osMessageQueueNew(tamMem, sizeof(char*), NULL);
  filaElevador[2] = osMessageQueueNew(tamMem, sizeof(char*), NULL);

  
  ControladorElevador_thread_id = osThreadNew(ControladorElevador, NULL, &thread0_attr); 
  
  ElevadorCentral_thread_id = osThreadNew(ElevadorCentral, NULL, &thread2_attr); 
  ElevadorEsquerda_thread_id = osThreadNew(ElevadorEsquerda, NULL, &thread1_attr); 
  ElevadorDireita_thread_id = osThreadNew(ElevadorDireita, NULL, &thread3_attr); 

  uart_id = osMutexNew(NULL); 
  
  osMutexAcquire(uart_id, osWaitForever); 
  UARTprintf("er\r");
  UARTprintf("cr\r");
  UARTprintf("dr\r");
  UARTprintf("er\r", osThreadGetName(osThreadGetId()), 50);
  UARTprintf("cr\r", osThreadGetName(osThreadGetId()), 50);
  UARTprintf("dr\r", osThreadGetName(osThreadGetId()), 50);

  UARTFlushTx(true);
  osMutexRelease(uart_id);
 
  if(osKernelGetState() == osKernelReady) 
    osKernelStart(); 
 
  while(1); 
} // main
