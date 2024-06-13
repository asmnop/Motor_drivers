# Motor_drivers
Drivers for all kind of motors

STEROWNIKI SILNIKÓW DC:
# TB6612FNG:
-sterownik silnika DC,

-sygnał PWM jest generowany na linii PA15 --> TIM2, TIM_CHANNEL_1,

-założenie jest takie, że znamy już dokładnie numer timera oraz kanału, z którego generowany jest sygnał PWM,

-zostały dodane funkcje testowe, inicjalizacja, ustawianie prędkości obrotowej (wypełnienia) oraz kierunku obrotów,
