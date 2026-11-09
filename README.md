# LPC Bip. Integrated Itinerary and Fare Engine - Valle de Aburrá

Práctica 2 de **SI1002-7 Lenguajes y Paradigmas de la Computación** (2026-2), Universidad EAFIT.

Un motor en C++17 que calcula cuánto paga una persona por un día de viaje en el sistema integrado del Valle de Aburrá. El cobro depende del transporte que usa, del tipo de pasajero que es y de lo que ya validó antes: dentro de una ventana de 90 minutos tiene hasta dos transbordos gratis.


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

## Diseño

### Parte 1: jerarquía de transportes (líneas 59–161)

- `TransportMode` es una **clase abstracta**. `baseFare()` y `serviceType()` son virtuales puras (`= 0`), así que cada modo está obligado a definirlas.
- Tiene tres funciones con valor por defecto `true` (`allowsStudentFlatFare()`, `allowsIntegration()`, `opensIntegrationWindow()`). Cada modo solo redefine las que cambian.
- `describe()` **no es virtual**: es un método plantilla escrito una sola vez que llama a lo que define cada modo.
- El destructor es **virtual**, porque los modos se destruyen desde un puntero a la base.
- `TouristCable` hereda de `CableLine` y obtiene la tarifa del padre con `CableLine::baseFare()` (el equivalente de `super`) más el recargo. No repite el `3820`.

### Parte 2: jerarquía de pasajeros (líneas 181–275)

Usa el patrón **Non-Virtual Interface**:

- `fareOn()` es pública y **no virtual**: llama a `computeFare()` y limita el resultado con `std::max`.
- `computeFare()` es `protected` y virtual: cada perfil la redefine y pide la tarifa base con `Passenger::computeFare(modo)`.
- `FrequentPassenger` no redefine nada: hereda la tarifa plena.
- `StudentPassenger` le pregunta al modo con `allowsStudentFlatFare()`, nunca qué clase es.

### Parte 3: red e itinerario (líneas 280–379)

- `Network` es la **dueña** de los modos: los guarda en `std::vector<std::unique_ptr<TransportMode>>`. `add()` recibe el puntero con `std::move` y devuelve una referencia, así que `main()` observa los modos sin ser su dueño.
- `chargeItinerary()` recibe los tramos **por copia**, los ordena con `std::sort` y una lambda, y los recorre con el **único ciclo** del programa aplicando la regla de integración.

### Parte 4: reportes con la STL (líneas 384–445)

| Función | Algoritmo | Qué calcula |
| ------- | --------- | ----------- |
| `totalPaid` | `std::accumulate` | Total pagado |
| `freeTransferCount` | `std::count_if` | Número de transbordos gratis |
| `paidByServiceType` | `std::for_each` + `std::map` | Pagado por tipo de servicio (el mapa se captura por referencia) |
| `namesMatching` | `std::function` inyectada | Nombres de los modos que cumplen un criterio que decide quien llama |
| `joined` | `std::accumulate` sobre strings | Une los nombres con `", "` |

Además, `main()` usa `std::find_if` para encontrar el primer tramo que realmente pagó.

## Reglas del enunciado que cumple

- **Regla A:** ninguna decisión depende del tipo del objeto. No hay `dynamic_cast`, `typeid` ni comparaciones con `serviceType()`.
- **Regla B:** sin ciclos en los reportes. El único ciclo escrito por nosotros está en `chargeItinerary`.
- **Regla C:** el literal `3820` aparece en código solo en `MetroLine`, `TramLine` y `CableLine`.
- **Calidad:** cero advertencias con `-Wall -Wextra`, métodos que no modifican el objeto declarados `const`, y propiedad con `std::unique_ptr` y `std::make_unique`, sin `new` ni `delete`.
