/*
 * TB6612FNG.c
 *
 *  Created on: Jun 13, 2024
 *      Author: asmnop
 */


#include "TB6612FNG.h"
#include "tim.h"
#include "universal_data.h"


//	UWAGI:
//	-podłączenia do pinów sterownika:
//	-jest jedna wspólna masa, do której muszą byc podłączone zarówno masa z uC jak i masa z zasilania silnika DC,
//	-Vcc - zasilanie sekcji logicznej --> 2.7 - 5.5 [V]
//	-do linii STBY musi byc podpięty sygnał wysoki, np. podłączony z zasilania mikrokontrolera, linia nie może wisiec w powietrzu,
//	-do linii PWMx można na początek podłączyć stan wysoki z pinu uC --> maksymalne wypełnienie,
//	-jeśli chcemy podłączyc jeden silnik do jednego sterownika, który posiada dwa kanały to łączymy ze sobą linie:
//	AO1 - BO1, AO2 - BO2, PWMA - PWMB, AIN1 - BIN1, AIN2 - BIN2,
//	-A i B to nazwy kanałów, 1 i 2 to numery linii. Łaczymy ze sobą różne kanały o tych samych numerach linii!!!
//	-połączenie kanałów zwiększa wydajność prądową, stosować w sytuacjach gdy możliwe jest zatrzymanie silników,
//	-stosować także podczas testów początkowych lub jeśli nie znamy parametrów silnika,
//	-maksymalna częstotliwość przełączania fPWM = 100 000 [Hz],
//	-należy wprowadzic opóźnienia czasowe dla:
//	-tr = 24 [ns], WTF??? - ODP time raise,
//	-tf = 41 [ns], WTF??? - ODP time fall,
//	-Dead time H-->L = (50) [ns], WTF???
//	-Dead time L-->H = (230) [ns], WTF???,
//	-pisany kod odnosi się jedynie do obsługi jednego silnika, nie ma tutaj funkcji wykonujących zadanie np. jazda
//	do przodu z zadaną prędkością pojazdu dwukołowego,
// -sterownik silnika DC tak dobrac aby ciągły prąd wyjściowy sterownika był większy od prądu silnika przy zatrzymanym wale,

//	OSTRZEŻENIA:
//	-nigdy po osiągnięciu pełnej prędkości nie używać do zatrzymania silnika opcji z hamulcem!!!,
//	-raz tak zrobiłem i spaliłem sterownik TB6612FNG, używany silnik to silnik od wkrętarki ręcznej,

//	UWAGI AVR:
//	-jeśli podczas włączania obrotów poszczególnych silników występują spadki napięcia w układzie zasilania
//	do którego jest również podłączony moduł BTM-222 (przygasające lampki) to należy wprowadzic delaye pomiędzy
//	załączeniami poszczególnych silników. Delaye około 50ms. W przeciwnym razie może nastąpic utrata połączenia
//	pomiedzy PC a BTM-222,
//	-nie podłączać linii sterujących kierunkiem obrotu silników do pinów uC odpowiedzialnych za programowanie tzn. PB3 (MOSI),
//	PB4 (MISO), PB5 (CLK). Podczas programowania silniki zostają uruchomione. Linia PWMA podłączona do Vcc - stan wysoki,
//	-zakładając, że maksymalna F_CPU wynosi 20 000 000 to musimy wprowadzic opóźnienia tylko dla 'Dead time L-->H'
//	-opóźnienie należy wprowadzic już około dla częstotliwości F_CPU > 4 000 000,
//	-maksymalne opóźnienie dla F_CPU to 5 NOP-ów,
//	-opóźnienie należy wprowadzic tylko podczas wchodzenia do trybu Short Brake z trybu ruchu, oraz do wychodzenia
//	z trybu Short Break do trybu ruchu,
//	-WAŻNE: jeśli podłączamy plus z silnika do plusa zasilania i minus z silnika do minusa zasilania to silnik kręci się w CCW!!!

//	DEFINICJA KIERUNKU OBROTÓW:
//	-kierunek obrotów CW - clockwise - jest rozumiany jako zgodny z ruchem wskazówek zegara patrząc na silnik od strony
//	wału wyjściowego,
//	-kierunek obrotów CCW - counter-clockwise - jest rozumiany jako przeciwny do ruchu wskazówek zegara patrząc na silnik
//	od strony wału wyjściowego,

//	TRYBY PRACY:
//	-dostępne tryby pracy do obsługi: Short Brake, Stop, Standby, CW, CCW,
//	-jako tryb domyślny przy starcie/inicjalizacji jest ustawiany tryb Short Brake,
//	-tryb Standby niedostępny - podłączenie pinu na stałe do Vcc,
//	-tryb Short Brake cechuje się koniecznością ustawienia linii:
//	-PWM w stan niski, wejścia IN - stan dowolny,
//	-PWM - stan dowolny, wejścia IN - stan wysoki,

/*
	TABELA PRAWDY UKŁADU TB6612FNG:

	|-------------------------------|-----------------------------|
	|             Input             |            Output           |
	|-------|-------|-------|-------|-------|-------|-------------|
	|  IN1  |  IN2  |  PWM  | STBY  | OUT1  | OUT2  |     MODE    |
	|-------|-------|-------|-------|-------|-------|-------------|
	|   H   |   H   |  H/L  |   H   |   L   |   L   | Short Brake |
	|---------------|-------|-------|-------|-------|-------------|
	|       |       |   H   |   H   |   L   |   H   |     CCW     |
	|   L   |   H   |-------|-------|-------|-------|-------------|
	|       |       |   L   |   H   |   L   |   L   | Short Brake |
	|---------------|-------|-------|-------|-------|-------------|
	|       |       |   H   |   H   |   H   |   L   |     CW      |
	|   H   |   L   |-------|-------|-------|-------|-------------|
	|       |       |   L   |   H   |   L   |   L   | Short Brake |
	|-------|-------|-------|-------|---------------|-------------|
	|   L   |   L   |  H/L  |   H   |      OFF      |    Stop     |
	|-------|-------|-------|-------|---------------|-------------|
	|  H/L  |  H/L  |  H/L  |   L   |      OFF      |   Standby   |
	|-------|-------|-------|-------|---------------|-------------|

*/

//	ANALIZA TABELI PRAWDY UKŁADU TB6612FNG:
//	-weryfikacja ręczna tabeli prawdy: zgodność z tabelą oraz dopisanie stanu IN1 = L, IN2 = L, PWM = L, STBY = H --> STOP,
//	-aby silnik się obracał to na linie PWM musi być podawany stan wysoki,
//	-jeśli będziemy chcieli zatrzymać silnik to PWM w stan niski,
//	-jeśli będziemy chcieli zatrzymać silnik to linie wejściowe ustawiamy w stan niski --> tryb STOP,
//	-wtedy nieważne co jest ustawione na linii PWM, silnik się nie obraca,
//	-jeśli chcemy wprowadzić stan Short Brake to po wejściu do trybu STOP ustawiamy linię PWM w stan niski,
//	-wtedy nie jest możliwe włączenie silnika do obracania się,
//	-następnie wystarczy ustawić jedną linię IN w stan wysoki --> tryb Short Brake,
//	-lepiej dla pewności ustawić dwie linie IN w stan wysoki, PWM w stan niski,
//	-tak więc jeśli chcemy używać trybu Short Brake to musimy zerować wypełnienie sygnału PWM,
//	-jeśli stosujemy jedynie tryb STOP to na linię PWM możemy cały czas podawać sygnał PWM,
//	-jeśli chcemy zmienić kierunek obrotów silnika to musimy wykonać przejście przez prędkość zero czy wypełnienie
//	sygnału PWM musi w pewnym momencie wynosić zero,

//	PRZEMYŚLENIA:
//	-wykonać funkcję która ustawia kierunek obrotów na CW lub CCW,
//	-foo która zmienia kierunek obrotów z zachowaniem prędkości obrotowej,



void TM6612FNG_init(TB6612FNG_t *motor);
uint8_t TB6612FNG_get_dir(TB6612FNG_t *motor);
void TB6612FNG_dir(TB6612FNG_t *motor, const uint8_t dir);
void TB6612FNG_speed(TB6612FNG_t *motor, uint8_t speed);


void TM6612FNG_init(TB6612FNG_t *motor)
{
	//	konfiguracja wstępna sterownika,

	HAL_GPIO_WritePin(motor->port_IN1, motor->pin_IN1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->port_IN2, motor->pin_IN2, GPIO_PIN_RESET);

	__HAL_TIM_SetCompare(motor->htim, motor->Channel, motor->duty);			//	Równoważne z 'htim2.Instance->CCR1 = 0;'

	HAL_TIM_PWM_Start(motor->htim, motor->Channel);
}

void TB6612FNG_dir(TB6612FNG_t *motor, const uint8_t dir)
{
	uint8_t set_dir = TB6612FNG_get_dir(motor);

	if(dir != set_dir)
	{
		uint16_t speed_temp = motor->duty;

		TB6612FNG_speed(motor, 0);
		HAL_GPIO_WritePin(motor->port_IN1, motor->pin_IN1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(motor->port_IN2, motor->pin_IN2, GPIO_PIN_SET);

		if(dir == CW)
		{
			HAL_GPIO_WritePin(motor->port_IN1, motor->pin_IN1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(motor->port_IN2, motor->pin_IN2, GPIO_PIN_RESET);
		}
		else if(dir == CCW)
		{
			HAL_GPIO_WritePin(motor->port_IN1, motor->pin_IN1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(motor->port_IN2, motor->pin_IN2, GPIO_PIN_SET);
		}

		TB6612FNG_speed(motor, speed_temp);
	}
}

void TB6612FNG_speed(TB6612FNG_t *motor, uint8_t speed)
{
	//	-ustawianie prędkości obrotowej wału silnika,
	//	-w rzeczywistości jest to ustawienie wypełnienia sygnału PWM,
	//	-należy pamiętać aby nie przekroczyć wartości z rejestru ARR, który oznacza liczbę, do której zlicza
	//	timer a po jej osiągnięciu jego wartość przyjmuje wartość 0,
	//	wartość speed jest podstawiana do rejestru CCRx = TIM capture/compare register x,

	if(speed >= motor->htim->Instance->ARR)
		speed = motor->htim->Instance->ARR;

	__HAL_TIM_SetCompare(motor->htim, motor->Channel, speed);

	motor->duty = speed;
	//__HAL_TIM_GET_COMPARE(motor->htim, motor->Channel);
}

uint8_t TB6612FNG_get_dir(TB6612FNG_t *motor)
{
	uint8_t state_IN1 = 0;
	uint8_t state_IN2 = 0;

	state_IN1 = HAL_GPIO_ReadPin(motor->port_IN1, motor->pin_IN1);
	state_IN2 = HAL_GPIO_ReadPin(motor->port_IN2, motor->pin_IN2);

	if(state_IN1 != state_IN2)
	{
		if(state_IN1 == 1)
			return CW;
		else
			return CCW;
	}

	return 88;
}




