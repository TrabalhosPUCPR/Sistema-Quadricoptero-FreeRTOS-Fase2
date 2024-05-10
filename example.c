// Leonardo Matthew Knight
// Ciência da Computação

#include "FreeRTOS.h"
#include "task.h"
#include "basic_io.h"
#include "semphr.h"

/*
	GUINADA - SENTIDO:
	0 = SENTIDO HORARIO
	1 (ou qualquer outro numero) = SENTIDO ANTI-HORARIO


	ARFAGEM - DIRECAO:
	0 = FRENTE
	1 (ou qualquer outro numero) = TRAS


	ROLAGEM - ORIENTACAO
	0 = ESQUERDA
	1 (ou qualquer outro numero) = DIREITA
*/
#define SENTIDO_HORARIO_NUMBER 0
#define SENTIDO_ANTI_HORARIO_NUMBER 1
#define DIRECAO_FRENTE_NUMBER 0
#define DIRECAO_TRAS_NUMBER 1
#define ORIENTACAO_ESQUERDA_NUMBER 0
#define ORIENTACAO_DIREITA_NUMBER 1

#define SENTIDO_HORARIO "horario"
#define SENTIDO_ANTI_HORARIO "anti_horario"
#define DIRECAO_FRENTE "frente"
#define DIRECAO_TRAS "tras"
#define ORIENTACAO_ESQUERDA "esquerda"
#define ORIENTACAO_DIREITA "direita"

void taskGuinada(void* param);
void taskRolagem(void* param);
void taskArfagem(void* param);
void taskRadioFrequencia(void* param);

void decrement_motor(SemaphoreHandle_t m_handle, int* motor);
void increment_motor(SemaphoreHandle_t m_handle, int* motor);

void update_manobras(struct Manobras m);

void print_motors_information(char* task_name);

// 4 inteiros para 4 motores
volatile int motor1, motor2, motor3, motor4 = 0;

// 3 inteiros para cada manobra, com valor inicial estando no main
// tamanho 12 pq o anti_horario tem 12 caracteres
struct Manobras {
	char sentido[13];
	char direcao[13];
	char orientacao[13];
};
volatile struct Manobras manobras;
SemaphoreHandle_t sem1, sem2, sem3, sem4, man;

int main_() {

	// instancia os 4 semaforos + o semaforo para controlar as manobras
	sem1 = xSemaphoreCreateBinary();
	sem2 = xSemaphoreCreateBinary();
	sem3 = xSemaphoreCreateBinary();
	sem4 = xSemaphoreCreateBinary();
	man = xSemaphoreCreateBinary();

	xSemaphoreGive(sem1);
	xSemaphoreGive(sem2);
	xSemaphoreGive(sem3);
	xSemaphoreGive(sem4);
	xSemaphoreGive(man);

	srand(570);

	if (sem1 == NULL || sem2 == NULL || sem3 == NULL || sem4 == NULL) {
		vPrintString("Sem espaco no heap");
		return 0;
	}

	// manobras iniciais, mudar aqui
	struct Manobras manobraInicial;
	sprintf(manobraInicial.direcao, DIRECAO_FRENTE);
	sprintf(manobraInicial.orientacao, ORIENTACAO_DIREITA);
	sprintf(manobraInicial.sentido, SENTIDO_HORARIO);

	xTaskCreate(taskGuinada, "guinada", 1000, (void*)manobraInicial.sentido, 2, NULL);
	xTaskCreate(taskRolagem, "rolagem", 1000, (void*)manobraInicial.orientacao, 2, NULL);
	xTaskCreate(taskArfagem, "arfagem", 1000, (void*)manobraInicial.direcao, 2, NULL);
	xTaskCreate(taskRadioFrequencia, "radio frequencia", 1000, (void*)&manobraInicial, 1, NULL); // passamos a referencia da struct

	// inicia o escalonador
	vTaskStartScheduler();
	for (;;);
	return 0;
}

void taskGuinada(void* param) {
	const TickType_t delay = 10 * portTICK_PERIOD_MS; // delay constante para ser usado no loop
	char* sentido = (char*)param;
	for (;;) {
		if (strcmp(sentido, SENTIDO_HORARIO) == 0) {
			// diminuindo motores 2 e 4
			// aumentando motores 1 e 3
			increment_motor(sem1, &motor1);
			increment_motor(sem3, &motor3);
			decrement_motor(sem2, &motor2);
			decrement_motor(sem4, &motor4);
			print_motors_information("Guinada | horario");
		}
		else {
			// diminuindo motores 1 e 3
			// aumentando motores 2 e 4
			decrement_motor(sem1, &motor1);
			decrement_motor(sem3, &motor3);
			increment_motor(sem2, &motor2);
			increment_motor(sem4, &motor4);
			print_motors_information("Guinada | anti-horario");
		}
		vTaskDelay(delay);
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) { // take no semaforo para obter as manobras de forma concorrente
			sprintf(sentido, manobras.sentido);
			xSemaphoreGive(man);
		}
	}
	vTaskDelete(NULL);
}

void taskRolagem(void* param) {
	const TickType_t delay = 20 * portTICK_PERIOD_MS;
	char* orientacao = (char*)param;
	for (;;) {
		if (strcmp(orientacao, ORIENTACAO_ESQUERDA) == 0) {
			// diminuindo motores 1 e 4
			// aumentando motores 2 e 3
			decrement_motor(sem1, &motor1);
			decrement_motor(sem4, &motor4);
			increment_motor(sem2, &motor3);
			increment_motor(sem3, &motor3);
			print_motors_information("Rolagem | esquerda");
		}
		else {
			// diminuindo motores 2 e 3
			// aumentando motores 1 e 4
			decrement_motor(sem2, &motor2);
			decrement_motor(sem3, &motor3);
			increment_motor(sem1, &motor1);
			increment_motor(sem4, &motor4);
			print_motors_information("Rolagem | direita");
		}
		vTaskDelay(delay);
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) {
			sprintf(orientacao, manobras.orientacao);
			xSemaphoreGive(man);
		}
	}
	vTaskDelete(NULL);
}

void taskArfagem(void* param) {
	const TickType_t delay = 40 * portTICK_PERIOD_MS;
	char* direcao = (char*)param;
	for (;;) {
		if (strcmp(direcao, DIRECAO_FRENTE) == 0) {
			// diminuindo motores 1 e 2
			// aumentando motores 3 e 4
			increment_motor(sem3, &motor3);
			increment_motor(sem4, &motor4);
			decrement_motor(sem1, &motor1);
			decrement_motor(sem2, &motor2);
			print_motors_information("Arfagem | frente");
		}
		else {
			// diminuindo motores 3 e 4
			// aumentando motores 1 e 2
			increment_motor(sem1, &motor1);
			increment_motor(sem2, &motor2);
			decrement_motor(sem3, &motor3);
			decrement_motor(sem4, &motor4);
			print_motors_information("Arfagem | tras");
		}
		vTaskDelay(delay);
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) {
			sprintf(direcao, manobras.direcao);
			xSemaphoreGive(man);
		}
	}
	vTaskDelete(NULL);
}

void taskRadioFrequencia(void* param) {
	const TickType_t delay = 100 * portTICK_PERIOD_MS;
	update_manobras(*(struct Manobras*)param); // primeira coisa que fazemos é atualizar as manobras com o que foi passado como parametro
	vTaskDelay(delay); // damos um delay para nao sobreescrever as manobras iniciais logo de primeira
	for (;;) { // inicia o loop, sobreescrevendo as manobras uma nova de forma aleatória.
		struct Manobras new_manobras;
		if ((rand() % 100) % 2 == DIRECAO_FRENTE_NUMBER) {
			sprintf(new_manobras.direcao, DIRECAO_FRENTE);
		}
		else {
			sprintf(new_manobras.direcao, DIRECAO_TRAS);
		}
		if ((rand() % 100) % 2 == ORIENTACAO_DIREITA_NUMBER) {
			sprintf(new_manobras.orientacao, ORIENTACAO_DIREITA);
		}
		else {
			sprintf(new_manobras.orientacao, ORIENTACAO_ESQUERDA);
		}
		if ((rand() % 100) % 2 == SENTIDO_ANTI_HORARIO_NUMBER) {
			sprintf(new_manobras.sentido, SENTIDO_ANTI_HORARIO);
		}
		else {
			sprintf(new_manobras.sentido, SENTIDO_HORARIO);
		}
		update_manobras(new_manobras);
		vTaskDelay(delay);
	}
	vTaskDelete(NULL);
}


// FUNCAO INLINE = COMPILADOR COLOCA ESSE BLOCO DE CODIGO DIRETO ONDE A FUNCAO É CHAMADA
// equivalente a eu fazer ctrl c + v.
// é bom para coisa pequena que repete bastante no codigo
/*
	Funcao de conveniencia para diminuir o valor de um motor em 1 de forma concorrente
*/
inline void decrement_motor(SemaphoreHandle_t m_handle, int* motor) {
	if (xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE) {
		*motor = *motor - 1; // deref do ponteiro para mudar o valor no ponto da memória que ele esta apontando
		xSemaphoreGive(m_handle);
	}
}

/*
	Funcao de conveniencia para aumentar o valor de um motor em 1 de forma concorrente
*/
inline void increment_motor(SemaphoreHandle_t m_handle, int* motor) { // essa funcao é igual ao de cima, só muda o - para +
	if (xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE) {
		*motor = *motor + 1;
		xSemaphoreGive(m_handle);
	}
}

/*
	Funcao de conveniencia para atualizar a variavel global de manobras de forma concorrente e segura.

	Essa funcao também printa os novos valores passadas como argumento
*/
void update_manobras(struct Manobras m) {
	if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) {
		printf("Direcao %s\n", m.direcao);
		printf("Sentido %s\n", m.sentido);
		printf("Orientacao %s\n", m.orientacao);
		sprintf(manobras.direcao, m.direcao);
		sprintf(manobras.orientacao, m.orientacao);
		sprintf(manobras.sentido, m.sentido);
		xSemaphoreGive(man);
	}
}

/*
	Funcao de conveniencia para printar todos os dados atuais dos motores junto com uma string parametrizada
	passada por cada task que chama a função.

	Funcao é concorrente, sendo que ela da take no semaforo de todos os motores antes de pegar as suas informações
*/
void print_motors_information(char* task_name) {
	if (xSemaphoreTake(sem1, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(sem2, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(sem3, portMAX_DELAY) == pdTRUE &&
		xSemaphoreTake(sem4, portMAX_DELAY) == pdTRUE
		) {
		printf("[Task %s] Motor 1: %d - Motor 2: %d - Motor 3: %d - Motor 4: %d\n", task_name, motor1, motor2, motor3, motor4);
		xSemaphoreGive(sem1);
		xSemaphoreGive(sem2);
		xSemaphoreGive(sem3);
		xSemaphoreGive(sem4);
	}
}



