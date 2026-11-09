# LPC Bip. Integrated Itinerary and Fare Engine - Valle de Aburrá

SI1002-7 Lenguajes y Paradigmas de la Computación · 2026-2

## Integrantes

- Samuel De La Cruz García Vásquez
- Laura Castro Montoya

## Cómo compilar y ejecutar

```bash
g++ -std=c++17 -Wall -Wextra -o main main.cpp
./main
```

En OnlineGDB: Language C++17, pegar main.cpp y Run

## Preguntas

### 1. ¿Qué pasa si el destructor de `TransportMode` no es virtual?

La red guarda los modos como `std::unique_ptr<TransportMode>`, así que cada modo se destruye desde un
puntero a la base. Si `~TransportMode()` no es virtual, se llama solo al destructor de la base y no al
de la clase hija; según el estándar eso es comportamiento indefinido y los recursos propios de la clase
hija no se liberan. Por eso el destructor virtual es necesario y no es solo una formalidad.

### 2. ¿Por qué una clase abstracta con funciones virtuales puras y no un `enum` con un `switch`?

Con un `enum` y un `switch`, cada decisión (tarifa, tipo de servicio, tarifa de estudiante, integración,
ventana) queda en un `switch` distinto, y al agregar un modo nuevo hay que modificarlos todos. Con la
clase abstracta, un modo nuevo es una clase nueva que define solo lo que cambia, y el resto del código no
se toca. Además, las funciones virtuales puras obligan a que cada modo defina su tarifa y su tipo de
servicio; si falta alguna, el programa no compila.
