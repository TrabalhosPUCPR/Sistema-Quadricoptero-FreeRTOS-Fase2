// Leonardo Matthew Knight

#include "FreeRTOS.h"
#include "task.h"
#include "basic_io.h"
#include "semphr.h"

void taskGuinada(void* param);
void taskRolagem(void* param);
void taskArfagem(void* param);

void decrement_motor(SemaphoreHandle_t m_handle, int* motor);
void increment_motor(SemaphoreHandle_t m_handle, int* motor);

void print_motors_information();

// 4 inteiros para 4 motores, e um mutex para cada
int motor1, motor2, motor3, motor4 = 0;
SemaphoreHandle_t mutex1, mutex2, mutex3, mutex4;

int main_() {

	// instancia os 4 mutex
	mutex1 = xSemaphoreCreateMutex();
	mutex2 = xSemaphoreCreateMutex();
	mutex3 = xSemaphoreCreateMutex();
	mutex4 = xSemaphoreCreateMutex();

	// passando 0 como parametro para todos
	xTaskCreate(taskGuinada, "guinada", 1000, (void*)0, 1, NULL);
	xTaskCreate(taskRolagem, "rolagem", 1000, (void*)0, 1, NULL);
	xTaskCreate(taskArfagem, "arfagem", 1000, (void*)0, 1, NULL);

	// inicia o escalonador
	vTaskStartScheduler();
	for (;;);
	return 0;
}

/*
	0 = SENTIDO HORARIO
	1 (ou qualquer outro numero) = SENTIDO ANTI-HORARIO
*/
void taskGuinada(void* param) {
	const TickType_t delay = 10 / portTICK_PERIOD_MS;
	const int job = (int)param;
	// se for sentido horario ( 0 ), inicia o loop para essa funcao, caso contrario, inicia para anti-horario
	// mesma coisa para as outras tasks, diferenca é só o nome e quais motores modifica
	if (job == 0) {
		vPrintString("Task Guinada executando manobra em sentido horario\n");
		for (;;) {
			// diminuindo motores 2 e 4
			// aumentando motores 1 e 3
			increment_motor(mutex1, &motor1);
			increment_motor(mutex3, &motor3);
			decrement_motor(mutex2, &motor2);
			decrement_motor(mutex4, &motor4);
			print_motors_information("Guinada | horario");
			vTaskDelay(delay);
		}
	}
	else {
		vPrintString("Task Guinada executando manobra em sentido anti-horario\n");
		for (;;) {
			// diminuindo motores 1 e 3
			// aumentando motores 2 e 4
			decrement_motor(mutex1, &motor1);
			decrement_motor(mutex3, &motor3);
			increment_motor(mutex2, &motor2);
			increment_motor(mutex4, &motor4);
			print_motors_information("Guinada | anti-horario");
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

/*
	0 = ESQUERDA
	1 (ou qualquer outro numero) = DIREITA
*/
void taskRolagem(void* param) {
	const TickType_t delay = 20 / portTICK_PERIOD_MS;
	const int job = (int)param;
	if (job == 0) {
		vPrintString("Task Rolagem executando manobra para esquerda\n");
		for (;;) {
			// diminuindo motores 1 e 4
			// aumentando motores 2 e 3
			decrement_motor(mutex1, &motor1);
			decrement_motor(mutex4, &motor4);
			increment_motor(mutex2, &motor3);
			increment_motor(mutex3, &motor3);
			print_motors_information("Rolagem | esquerda");
			vTaskDelay(delay);
		}
	}
	else {
		vPrintString("Task Rolagem executando manobra para direita\n");
		for (;;) {
			// diminuindo motores 2 e 3
			// aumentando motores 1 e 4
			decrement_motor(mutex2, &motor2);
			decrement_motor(mutex3, &motor3);
			increment_motor(mutex1, &motor1);
			increment_motor(mutex4, &motor4);
			print_motors_information("Rolagem | direita");
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

/*
	0 = FRENTE
	1 (ou qualquer outro numero) = TRAS
*/
void taskArfagem(void* param) {
	const TickType_t delay = 40 / portTICK_PERIOD_MS;
	const int job = (int)param;
	if (job == 0) {
		vPrintString("Task Arfagem executando manobra para frente\n");
		for (;;) {
			// diminuindo motores 1 e 2
			// aumentando motores 3 e 4
			increment_motor(mutex3, &motor3);
			increment_motor(mutex4, &motor4);
			decrement_motor(mutex1, &motor1);
			decrement_motor(mutex2, &motor2);
			print_motors_information("Arfagem | frente");
			vTaskDelay(delay);
		}
	}
	else {
		vPrintString("Task Arfagem executando manobra para tras\n");
		for (;;) {
			// diminuindo motores 3 e 4
			// aumentando motores 1 e 2
			increment_motor(mutex1, &motor1);
			increment_motor(mutex2, &motor2);
			decrement_motor(mutex3, &motor3);
			decrement_motor(mutex4, &motor4);
			print_motors_information("Arfagem | tras");
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

/*
	FUNCAO INLINE = COMPILADOR COLOCA ESSE BLOCO DE CODIGO DIRETO ONDE A FUNCAO É CHAMADA

	apenas para nao precisar ficar dando ctrl c + v do que basicamente é a mesma coisa.

	funcao recebe um ponteiro para um numero inteiro para mudar aquele ponto da memoria.
*/
inline void decrement_motor(SemaphoreHandle_t m_handle, int* motor) {
	if (xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE) {
		*motor = *motor - 1; // deref do ponteiro para mudar o valor no ponto da memória que ele esta apontando
		xSemaphoreGive(m_handle);
	}
}

/*
	mesma coisa em cima, mas ao inves, incrementa o valor do motor
*/
inline void increment_motor(SemaphoreHandle_t m_handle, int* motor) {
	if (xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE) {
		*motor = *motor + 1;
		xSemaphoreGive(m_handle);
	}
}

void print_motors_information(char* task_name) {
	// take de todos os motores para nao ter problema na hora de printar os dados de todos
	if (xSemaphoreTake(mutex1, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(mutex2, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(mutex3, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(mutex4, portMAX_DELAY) == pdTRUE
	) {
		printf("[Task %s] Motor 1: %d - Motor 2: %d - Motor 3: %d - Motor 4: %d\n", task_name, motor1, motor2, motor3, motor4);
		xSemaphoreGive(mutex1);
		xSemaphoreGive(mutex2);
		xSemaphoreGive(mutex3);
		xSemaphoreGive(mutex4);
	}
}




