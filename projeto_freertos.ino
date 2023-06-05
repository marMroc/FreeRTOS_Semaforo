#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <semphr.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Ultrasonic.h>

/* Defines gerais */
#define BAUDRATE_SERIAL                    115200

/* Defines - LCD */
#define LCD_16X2_CLEAN_LINE                "                "
#define LCD_16X2_I2C_ADDRESS               0x27
#define LCD_16X2_COLS                      16 
#define LCD_16X2_ROWS                      2 
#define LCD_TEMPO_ENTRE_VERIFICACOES       1000 //ms

/* Defines - Sensor ultrassônico */
#define ULTRASSONICO_TRIGGER                13
#define ULTRASSONICO_ECHO                   12
#define ULTRASSONICO_TEMPO_ENTRE_LEITURAS   500 //ms

/* Definições - sensor MQ-2 */
#define ENTRA_ANALOGICA_A0               0 //A0

/* Defines - Sensor ultrassônico */
#define MQ2_TEMPO_ENTRE_LEITURAS         1000 //ms

/* Define - tempo para aguardar tomada de controle do semáforo */
#define TEMPO_PARA_AGUARDAR_SEMAFORO        ( TickType_t ) 100

/* Define - tempo para aguardar tomada de controle da fila */
#define TEMPO_PARA_AGUARDAR_FILA            ( TickType_t ) 100

/* filas (queues) */
QueueHandle_t fila_sensor_MQ2;
QueueHandle_t fila_sensor_ultrassonico;

/* semaforos utilizados */
SemaphoreHandle_t semaforo_serial;

/* Objetos e variáveis globais */
/* Objeto de controle do LCD */
LiquidCrystal_I2C lcd(LCD_16X2_I2C_ADDRESS, 
                      LCD_16X2_COLS, 
                      LCD_16X2_ROWS);

/* Objeto de controle do sensor ultrassônico */
Ultrasonic ultrasonic(ULTRASSONICO_TRIGGER, ULTRASSONICO_ECHO);                 

/* Tarefas */
void task_lcd( void *pvParameters );
void task_sensor_MQ2( void *pvParameters );
void task_sensor_ultrassonico( void *pvParameters );                     


void setup() 
{
    /* Inicializa serial (para uso com serial monitor) */
    Serial.begin(BAUDRATE_SERIAL);
 
    /* Inicializa o LCD, liga o backlight e limpa o LCD */
    lcd.init();
    lcd.backlight();
    lcd.clear();
 
    /* Criação das filas (queues) */ 
    fila_sensor_MQ2 = xQueueCreate( 1, sizeof(int) );
    fila_sensor_ultrassonico = xQueueCreate( 1, sizeof(float) );

    /* Verifica se as filas foram corretamente criadas */
    if ( (fila_sensor_MQ2 == NULL) || (fila_sensor_ultrassonico == NULL) )
    {
        Serial.println("Uma ou mais filas nao pode ser criada.");
        Serial.println("O programa nao pode continuar");
        while(1)
        {
          
        }
    }
  
    /* Criação do semáforo */
    semaforo_serial = xSemaphoreCreateMutex();

    /* Verifica se o semáforo foi corretamente criado */
    if (semaforo_serial == NULL)
    {
        Serial.println("O semaforo nao pode ser criado.");
        Serial.println("O programa nao pode continuar");
        while(1)
        {
          
        }
    }
    
    /* Criação das tarefas */
    xTaskCreate(
      task_lcd                    
      ,  (const portCHAR *)"lcd" 
      ,  156                       
      ,  NULL                      
      ,  1                        
      ,  NULL );                 
  
    xTaskCreate(
      task_sensor_MQ2
      ,  (const portCHAR *) "MQ2"
      ,  156  
      ,  NULL
      ,  2 
      ,  NULL );
  
  
    xTaskCreate(
      task_sensor_ultrassonico
      ,  (const portCHAR *)"ultrassonico"
      ,  156  
      ,  NULL
      ,  3 
      ,  NULL );

}

void loop() 
{
    //Tudo feito pelas tarefas
}

/* task_lcd: escrever leituras no LCD*/
void task_lcd( void *pvParameters )
{
    float distancia = 0.0;
    int leitura_MQ2 = 0;
    char linha_str[16] = {0x00};
    int distancia_cm = 0;
    
    while(1)
    {
        /* Escreve última leitura do sensor ultrassônico */
        if( xQueuePeek(fila_sensor_ultrassonico, &distancia, TEMPO_PARA_AGUARDAR_FILA) )
        {
            lcd.setCursor(0,0);
            lcd.print(LCD_16X2_CLEAN_LINE);
            lcd.setCursor(0,0);

            distancia_cm = (int)distancia;
            sprintf (linha_str, "Dist: %d cm", distancia_cm);
            lcd.print(linha_str);
        }

        /* Escreve última leitura analógica do sensor MQ-2 */
        if( xQueuePeek(fila_sensor_MQ2, &leitura_MQ2, TEMPO_PARA_AGUARDAR_FILA) )
        {
            lcd.setCursor(0,1);
            lcd.print(LCD_16X2_CLEAN_LINE);
            lcd.setCursor(0,1);

            sprintf (linha_str, "MQ2: %d", leitura_MQ2);
            lcd.print(linha_str);
        }
        
        /* Aguarda tempo definido em LCD_TEMPO_ENTRE_VERIFICACOES
           para verificar se ha novas leituras a serem escritas no display */
        vTaskDelay( LCD_TEMPO_ENTRE_VERIFICACOES / portTICK_PERIOD_MS );
    }
}

void task_sensor_MQ2( void *pvParameters )
{
    int leitura_analogica = 0;

    while(1)
    {
        leitura_analogica = analogRead(ENTRA_ANALOGICA_A0);

        /* Insere leitura na fila */
        xQueueOverwrite(fila_sensor_MQ2, (void *)&leitura_analogica);
       if ( xSemaphoreTake(semaforo_serial, TEMPO_PARA_AGUARDAR_SEMAFORO ) == pdTRUE )
       {
           Serial.print("- Leitura MQ-2: ");
           Serial.println(leitura_analogica);
           xSemaphoreGive(semaforo_serial);
       }
        vTaskDelay( MQ2_TEMPO_ENTRE_LEITURAS / portTICK_PERIOD_MS );
    }
}

/* task_sensor_ultrassonico: tarefa responsável por ler o sensor ultrassônico e enviar, 
                             através de uma fila, cada leitura para a tarefa que controla 
                             o display LCD */
void task_sensor_ultrassonico( void *pvParameters )
{
    float distancia_cm = 0.0;
    long microsec = 0;

    while(1)
    {
       microsec = ultrasonic.timing();
       distancia_cm = ultrasonic.convert(microsec, Ultrasonic::CM);

       xQueueOverwrite(fila_sensor_ultrassonico, (void *)&distancia_cm);
       if ( xSemaphoreTake(semaforo_serial, TEMPO_PARA_AGUARDAR_SEMAFORO ) == pdTRUE )
       {
           Serial.print("- Distancia: ");
           Serial.print(distancia_cm);
           Serial.println("cm");
           xSemaphoreGive(semaforo_serial);
       }

       vTaskDelay( ULTRASSONICO_TEMPO_ENTRE_LEITURAS / portTICK_PERIOD_MS );
    }    
}
