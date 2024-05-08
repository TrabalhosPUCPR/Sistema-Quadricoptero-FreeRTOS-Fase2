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

#define SENTIDO_HORARIO 0
#define SENTIDO_ANTI_HORARIO 1
#define DIRECAO_FRENTE 0
#define DIRECAO_TRAS 1
#define ORIENTACAO_ESQUERDA 1
#define ORIENTACAO_DIREITA 0

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
struct Manobras {
	int sentido;
	int direcao;
	int orientacao;
};
struct Manobras manobras;
SemaphoreHandle_t sem1, sem2, sem3, sem4, man;

int main_() {

	// instancia os 4 semaforos + o semaforo para controlar as manobras
	sem1 = xSemaphoreCreateBinary();
	sem2 = xSemaphoreCreateBinary();
	sem3 = xSemaphoreCreateBinary();
	sem4 = xSemaphoreCreateBinary();
	man = xSemaphoreCreateBinary();

	srand(570);

	if (sem1 == NULL || sem2 == NULL || sem3 == NULL || sem4 == NULL) {
		vPrintString("Sem espaco no heap");
		return 0;
	}

	// manobras iniciais, mudar aqui
	struct Manobras manobraInicial;
	manobraInicial.direcao = DIRECAO_FRENTE;
	manobraInicial.orientacao = ORIENTACAO_DIREITA;
	manobraInicial.sentido = SENTIDO_HORARIO;

	xTaskCreate(taskGuinada, "guinada", 1000, NULL, 2, NULL);
	xTaskCreate(taskRolagem, "rolagem", 1000, NULL, 2, NULL);
	xTaskCreate(taskArfagem, "arfagem", 1000, NULL, 2, NULL);
	xTaskCreate(taskRadioFrequencia, "radio frequencia", 1000, (void*)&manobraInicial, 1, NULL); // passamos a referencia da struct

	// inicia o escalonador
	vTaskStartScheduler();
	for (;;);
	return 0;
}

void taskGuinada(void* param) {
	const TickType_t delay = 10 * portTICK_PERIOD_MS; // delay constante para ser usado no loop
	for (;;) {
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) { // take no semaforo para obter as manobras de forma concorrente
			// o resto é auto explicativo
			if (manobras.sentido == SENTIDO_HORARIO) {
				xSemaphoreGive(man);
				// diminuindo motores 2 e 4
				// aumentando motores 1 e 3
				increment_motor(sem1, &motor1);
				increment_motor(sem3, &motor3);
				decrement_motor(sem2, &motor2);
				decrement_motor(sem4, &motor4);
				print_motors_information("Guinada | horario");
			}
			else {
				xSemaphoreGive(man);
				// diminuindo motores 1 e 3
				// aumentando motores 2 e 4
				decrement_motor(sem1, &motor1);
				decrement_motor(sem3, &motor3);
				increment_motor(sem2, &motor2);
				increment_motor(sem4, &motor4);
				print_motors_information("Guinada | anti-horario");
			}
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

void taskRolagem(void* param) {
	const TickType_t delay = 20 * portTICK_PERIOD_MS;
	for (;;) {
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) {
			if (manobras.orientacao == ORIENTACAO_ESQUERDA) {
				xSemaphoreGive(man);
				// diminuindo motores 1 e 4
				// aumentando motores 2 e 3
				decrement_motor(sem1, &motor1);
				decrement_motor(sem4, &motor4);
				increment_motor(sem2, &motor3);
				increment_motor(sem3, &motor3);
				print_motors_information("Rolagem | esquerda");
			}
			else {
				xSemaphoreGive(man);
				// diminuindo motores 2 e 3
				// aumentando motores 1 e 4
				decrement_motor(sem2, &motor2);
				decrement_motor(sem3, &motor3);
				increment_motor(sem1, &motor1);
				increment_motor(sem4, &motor4);
				print_motors_information("Rolagem | direita");
			}
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

void taskArfagem(void* param) {
	const TickType_t delay = 40 * portTICK_PERIOD_MS;
	for (;;) {
		if (xSemaphoreTake(man, portMAX_DELAY) == pdTRUE) {
			if (manobras.direcao == DIRECAO_FRENTE) {
				xSemaphoreGive(man);
				// diminuindo motores 1 e 2
				// aumentando motores 3 e 4
				increment_motor(sem3, &motor3);
				increment_motor(sem4, &motor4);
				decrement_motor(sem1, &motor1);
				decrement_motor(sem2, &motor2);
				print_motors_information("Arfagem | frente");
			}
			else {
				xSemaphoreGive(man);
				// diminuindo motores 3 e 4
				// aumentando motores 1 e 2
				increment_motor(sem1, &motor1);
				increment_motor(sem2, &motor2);
				decrement_motor(sem3, &motor3);
				decrement_motor(sem4, &motor4);
				print_motors_information("Arfagem | tras");
			}
			vTaskDelay(delay);
		}
	}
	vTaskDelete(NULL);
}

void taskRadioFrequencia(void* param) {
	xSemaphoreGive(man); 
	const TickType_t delay = 100 * portTICK_PERIOD_MS;
	update_manobras(*(struct Manobras*)param); // primeira coisa que fazemos é atualizar as manobras com o que foi passado como parametro
	// ligamos apos setar as manobras iniciais, sinalizando que o radio "ligou"
	xSemaphoreGive(sem1);
	xSemaphoreGive(sem2);
	xSemaphoreGive(sem3);
	xSemaphoreGive(sem4);
	vTaskDelay(delay); // damos um delay para nao sobreescrever as manobras iniciais logo de primeira
	for (;;) { // inicia o loop, sobreescrevendo as manobras uma nova de forma aleatória.
		struct Manobras new_manobras;
		new_manobras.direcao = (rand() % 100) % 2;
		new_manobras.orientacao = (rand() % 100) % 2;
		new_manobras.sentido = (rand() % 100) % 2;
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
		if (m.sentido == SENTIDO_HORARIO) {
			vPrintString("Sentido horario\n");
		}
		else {
			vPrintString("Sentido anti-horario\n");
		}
		if (m.orientacao == ORIENTACAO_ESQUERDA) {
			vPrintString("Orientacao Esquerda\n");
		}
		else {
			vPrintString("Orientacao direita\n");
		}
		if (m.direcao == DIRECAO_FRENTE) {
			vPrintString("Direcao frente\n");
		}
		else {
			vPrintString("Direcao tras\n");
		}
		manobras.direcao = m.direcao;
		manobras.orientacao = m.orientacao;
		manobras.sentido = m.sentido;
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



