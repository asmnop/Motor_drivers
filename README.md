# Motor_drivers
Drivers for all kind of motors

STEROWNIKI SILNIKÓW DC:
# TB6612FNG:
-sterownik silnika DC,

-sygnał PWM jest generowany na linii PA15 --> TIM2, TIM_CHANNEL_1,

-założenie jest takie, że znamy już dokładnie numer timera oraz kanału, z którego generowany jest sygnał PWM,

-założenie, że obsługujemy tylko jeden silnik DC,

-jeśli byśmy chcieli dodać obsługę kolejnego to musimy utworzyć nowe funkcje dla nowych parametrów wybranych timerów i kanałów,

-zostały dodane funkcje testowe, inicjalizacja, ustawianie prędkości obrotowej (wypełnienia) oraz kierunku obrotów,
