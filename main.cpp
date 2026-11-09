/* =========================================================================
   Write here the full name of the members of the team formed for the
   development of the practice
       - Samuel De La Cruz Garcia Vasquez
       - Laura Castro Montoya
   =========================================================================
   
   PRACTICE - STARTING CODE (DO NOT CHANGE THE GIVEN SECTIONS)
   Integrated Itinerary and Fare Engine - Medellin Metro System
   Programming Languages and Paradigms (ST0244) - Universidad EAFIT

   Build: g++ -std=c++17 -Wall -Wextra -o practice main.cpp
   Target environment: OnlineGDB, language selector set to C++17

   WHAT YOU RECEIVE
     - the currency formatter, the Leg and ChargedLeg records and the whole
       main() test harness, so that the expected output is reproducible
     - the declarations you must complete, each marked with TODO

   WHAT YOU MUST WRITE
     - PART 1: the TransportMode hierarchy
     - PART 2: the Passenger hierarchy
     - PART 3: chargeItinerary and the Network class
     - PART 4: the four report functions, with STL algorithms only

   The file must compile with ZERO errors and ZERO warnings under
   g++ -std=c++17 -Wall -Wextra. A warning is a defect, not a detail.
   ========================================================================= */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>

/* -------------------------------------------------------------------------
   Utility: Colombian peso formatting (3820 -> "$ 3.820")
   ------------------------------------------------------------------------- */
std::string formatCOP(long long value) {
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        out.push_back(*it);
        if (++count % 3 == 0 && it + 1 != digits.rend()) {
            out.push_back('.');
        }
    }
    std::reverse(out.begin(), out.end());
    return "$ " + out;
}

/* -------------------------------------------------------------------------
   PART 1. Abstract base class of the transport-mode hierarchy
   ------------------------------------------------------------------------- */
class TransportMode {
public:
    TransportMode(std::string name, int stations)
        : name_(std::move(name)), stations_(stations) {}

    virtual ~TransportMode() = default;          // mandatory: deleted through base pointer

    const std::string& name() const { return name_; }
    int stations() const { return stations_; }

    // TODO 1.1  Declare baseFare() and serviceType() as PURE VIRTUAL.
    //           This class cannot know either value: that is the whole point.

    // Cada modo dice cuanto cuesta y que tipo de servicio es.
    virtual long long baseFare() const = 0;
    virtual std::string serviceType() const = 0;

    // TODO 1.2  Declare the three hooks with a sensible default of true:
    //           allowsStudentFlatFare(), allowsIntegration(),
    //           opensIntegrationWindow().

    // Por defecto un modo deja todo; solo los cambiamos donde no se cumple.
    virtual bool allowsStudentFlatFare() const { return true; }
    virtual bool allowsIntegration() const { return true; }
    virtual bool opensIntegrationWindow() const { return true; }

    // TODO 1.3  Implement describe() as a NON-virtual template method that
    //           returns, for example:
    //              "Metro line A (21 stations) - full fare $ 3.820"
    //           It must call the pure virtual members through this object.

    // describe() no es virtual: se escribe una vez y llama a lo que define cada modo.
    std::string describe() const {
        return serviceType() + " " + name() + " (" + std::to_string(stations()) +
               " stations) - full fare " + formatCOP(baseFare());
    }

private:
    std::string name_;
    int stations_;
};

// TODO 1.4  Write the five concrete modes. Full fare of the integrated
//           system in 2026: 3820 pesos.
//
//              MetroLine      serviceType "Metro line"    fare 3820
//              TramLine       serviceType "Tram line"     fare 3820
//              CableLine      serviceType "Cable line"    fare 3820
//              TouristCable   inherits from CableLine, adds a surcharge
//                             given in its constructor. It must NOT repeat
//                             the 3820 literal: obtain the parent value with
//                             CableLine::baseFare(). No student flat fare,
//                             no integration, does not open a window.
//              FeederRoute    serviceType "Feeder route"  fare 0
//                             Free because the trunk fare covers it, so it
//                             must NOT open an integration window.

class MetroLine : public TransportMode {
public:
    MetroLine(const std::string& nombre, int estaciones) : TransportMode(nombre, estaciones) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Metro line"; }
};

class TramLine : public TransportMode {
public:
    TramLine(const std::string& nombre, int estaciones) : TransportMode(nombre, estaciones) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Tram line"; }
};

class CableLine : public TransportMode {
public:
    CableLine(const std::string& nombre, int estaciones) : TransportMode(nombre, estaciones) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Cable line"; }
};

// La tarifa del cable turistico es la del padre (CableLine::baseFare(),
// que seria como super) mas el recargo que llega en el constructor.
class TouristCable : public CableLine {
public:
    TouristCable(const std::string& nombre, int estaciones, long long recargo)
        : CableLine(nombre, estaciones), recargo_(recargo) {}

    long long baseFare() const override { return CableLine::baseFare() + recargo_; }
    std::string serviceType() const override { return "Tourist cable"; }

    bool allowsStudentFlatFare() const override { return false; }
    bool allowsIntegration() const override { return false; }
    bool opensIntegrationWindow() const override { return false; }

private:
    long long recargo_;
};

class FeederRoute : public TransportMode {
public:
    FeederRoute(const std::string& nombre, int estaciones) : TransportMode(nombre, estaciones) {}
    long long baseFare() const override { return 0; }
    std::string serviceType() const override { return "Feeder route"; }
    bool opensIntegrationWindow() const override { return false; }
};

/* -------------------------------------------------------------------------
   PART 2. Passenger hierarchy, Non-Virtual Interface idiom
   fareOn() is NOT virtual: it is the template method that clamps the result.
   computeFare() is the customisation point each profile overrides.
   ------------------------------------------------------------------------- */
// TODO 2.1  Write the Passenger hierarchy using the Non-Virtual Interface
//           idiom, which is the C++ way of writing a template method:
//
//              public:    long long fareOn(const TransportMode&) const
//                         NOT virtual. It calls computeFare and clamps the
//                         result with std::max(0LL, ...), because a fare is
//                         never negative.
//              protected: virtual long long computeFare(const TransportMode&) const
//                         Default behaviour: the full fare of the mode.
//
//           Also declare profile() as pure virtual and give the class a
//           virtual destructor.

class Passenger {
public:
    explicit Passenger(const std::string& nombre) : nombre_(nombre) {}
    virtual ~Passenger() = default;

    const std::string& name() const { return nombre_; }
    virtual std::string profile() const = 0;

    // fareOn() no es virtual: llama a computeFare() y con std::max
    // nos aseguramos de que la tarifa nunca quede negativa.
    long long fareOn(const TransportMode& modo) const {
        return std::max(0LL, computeFare(modo));
    }

protected:
    // Por defecto se cobra la tarifa plena del modo.
    virtual long long computeFare(const TransportMode& modo) const {
        return modo.baseFare();
    }

private:
    std::string nombre_;
};

// TODO 2.2  Write the five profiles. Each one overrides computeFare and
//           obtains the full fare by calling Passenger::computeFare(mode),
//           which is the C++ equivalent of 'super'. Civica fares, 2026:
//
//              FrequentPassenger      "Civica frequent"         3820
//                                     does NOT override computeFare
//              StudentPassenger       "Municipal student"       1600 flat,
//                                     but only when the mode allows it
//              SeniorPassenger        "Senior citizen"          base - 490
//              DisabilityPassenger    "Person with disability"  base - 1100
//              BearerPassenger        "Bearer (occasional)"     base + 580

const long long TARIFA_ESTUDIANTE      = 1600;
const long long DESCUENTO_ADULTO_MAYOR = 490;
const long long DESCUENTO_DISCAPACIDAD = 1100;
const long long RECARGO_PORTADOR       = 580;

// El usuario frecuente paga la tarifa plena, asi que no sobrescribe computeFare.
class FrequentPassenger : public Passenger {
public:
    explicit FrequentPassenger(const std::string& nombre) : Passenger(nombre) {}
    std::string profile() const override { return "Civica frequent"; }
};

// El estudiante le pregunta al modo si admite la tarifa plana.
class StudentPassenger : public Passenger {
public:
    explicit StudentPassenger(const std::string& nombre) : Passenger(nombre) {}
    std::string profile() const override { return "Municipal student"; }

protected:
    long long computeFare(const TransportMode& modo) const override {
        if (modo.allowsStudentFlatFare()) {
            return TARIFA_ESTUDIANTE;
        }
        return Passenger::computeFare(modo);
    }
};

class SeniorPassenger : public Passenger {
public:
    explicit SeniorPassenger(const std::string& nombre) : Passenger(nombre) {}
    std::string profile() const override { return "Senior citizen"; }

protected:
    long long computeFare(const TransportMode& modo) const override {
        return Passenger::computeFare(modo) - DESCUENTO_ADULTO_MAYOR;
    }
};

class DisabilityPassenger : public Passenger {
public:
    explicit DisabilityPassenger(const std::string& nombre) : Passenger(nombre) {}
    std::string profile() const override { return "Person with disability"; }

protected:
    long long computeFare(const TransportMode& modo) const override {
        return Passenger::computeFare(modo) - DESCUENTO_DISCAPACIDAD;
    }
};

class BearerPassenger : public Passenger {
public:
    explicit BearerPassenger(const std::string& nombre) : Passenger(nombre) {}
    std::string profile() const override { return "Bearer (occasional)"; }

protected:
    long long computeFare(const TransportMode& modo) const override {
        return Passenger::computeFare(modo) + RECARGO_PORTADOR;
    }
};

/* -------------------------------------------------------------------------
   PART 3. The itinerary and the integration rule
   ------------------------------------------------------------------------- */
struct Leg {
    int minute;                   // minutes elapsed since the journey started
    const TransportMode* mode;    // non-owning observer pointer
};

struct ChargedLeg {
    int minute;
    std::string modeName;
    std::string serviceType;
    long long charge;
    bool integrated;              // true: free transfer inside the window
};

// TODO 3.1  Write the Network class. It OWNS its modes:
//
//              std::vector<std::unique_ptr<TransportMode>> modes_;
//              TransportMode& add(std::unique_ptr<TransportMode> mode);
//              const std::vector<std::unique_ptr<TransportMode>>& modes() const;
//              void printInventory() const;   // one line per mode
//
//           add() returns a reference to the stored object so that main()
//           can keep observing it without owning it.

// La red es la duena de los modos (unique_ptr).
class Network {
public:
    TransportMode& add(std::unique_ptr<TransportMode> modo) {
        modes_.push_back(std::move(modo));
        return *modes_.back();
    }

    const std::vector<std::unique_ptr<TransportMode>>& modes() const { return modes_; }

    void printInventory() const {
        std::for_each(modes_.begin(), modes_.end(),
                      [](const std::unique_ptr<TransportMode>& modo) {
                          std::cout << "  - " << modo->describe() << '\n';
                      });
    }

private:
    std::vector<std::unique_ptr<TransportMode>> modes_;
};

// TODO 3.2  Write chargeItinerary, which applies the integration rule:
//
//    std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> legs,
//                                            const Passenger& passenger);
//
//      a) sort the legs by minute with std::sort and a lambda comparator;
//         the itinerary in main() is deliberately out of order
//      b) the first validation pays the full fare and opens a 90-minute
//         window
//      c) up to TWO transfers inside the window are free
//      d) a mode with allowsIntegration() == false always pays and never
//         touches the window
//      e) a mode with opensIntegrationWindow() == false pays its own fare
//         but leaves the window untouched
//      f) when the window expires or the two free transfers run out, the
//         next validation pays again and opens a new window
//
//    This is the only place where an explicit loop over the legs is allowed.

const int MINUTOS_VENTANA = 90;
const int TRANSBORDOS_GRATIS = 2;

// Ordenamos los tramos por minuto y los recorremos: si el modo integra y
// estamos dentro de la ventana con transbordos disponibles, el tramo es gratis.
// Si no, paga su tarifa y, si el modo integra, abre una ventana nueva.
std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> tramos, const Passenger& pasajero) {
    std::sort(tramos.begin(), tramos.end(),
              [](const Leg& tramoA, const Leg& tramoB) { return tramoA.minute < tramoB.minute; });

    std::vector<ChargedLeg> cobrados;
    bool ventanaAbierta = false;
    int inicioVentana = 0;
    int transbordosUsados = 0;

    for (const Leg& tramo : tramos) {
        const TransportMode& modo = *tramo.mode;
        bool integra = modo.allowsIntegration() && modo.opensIntegrationWindow();
        bool dentroDeVentana = ventanaAbierta &&
                               tramo.minute - inicioVentana <= MINUTOS_VENTANA &&
                               transbordosUsados < TRANSBORDOS_GRATIS;

        if (integra && dentroDeVentana) {
            transbordosUsados++;
            cobrados.push_back({tramo.minute, modo.name(), modo.serviceType(), 0, true});
        } else {
            cobrados.push_back({tramo.minute, modo.name(), modo.serviceType(),
                                pasajero.fareOn(modo), false});
            if (integra) {
                ventanaAbierta = true;
                inicioVentana = tramo.minute;
                transbordosUsados = 0;
            }
        }
    }
    return cobrados;
}

/* -------------------------------------------------------------------------
   PART 4. Reports built exclusively with STL algorithms and lambdas
   ------------------------------------------------------------------------- */
// TODO 4.1  long long totalPaid(const std::vector<ChargedLeg>&)
//           Total paid. Use std::accumulate with a lambda. No loops.
//
// TODO 4.2  long long freeTransferCount(const std::vector<ChargedLeg>&)
//           Number of free transfers. Use std::count_if. No loops.
//
// TODO 4.3  std::map<std::string, long long>
//               paidByServiceType(const std::vector<ChargedLeg>&)
//           Amount paid per service type. Use std::for_each with a lambda
//           that captures the map by reference. No loops.
//
// TODO 4.4  std::vector<std::string> namesMatching(
//               const Network&,
//               const std::function<bool(const TransportMode&)>& policy)
//           Names of the modes that satisfy a criterion the CALLER injects.
//           The network must not know what the criterion is.
//
// TODO 4.5  std::string joined(const std::vector<std::string>&)
//           Join with ", ". Use std::accumulate over strings. No loops.

long long totalPaid(const std::vector<ChargedLeg>& tramos) {
    return std::accumulate(tramos.begin(), tramos.end(), 0LL,
                           [](long long total, const ChargedLeg& tramo) { return total + tramo.charge; });
}

long long freeTransferCount(const std::vector<ChargedLeg>& tramos) {
    return std::count_if(tramos.begin(), tramos.end(),
                         [](const ChargedLeg& tramo) { return tramo.integrated; });
}

std::map<std::string, long long> paidByServiceType(const std::vector<ChargedLeg>& tramos) {
    std::map<std::string, long long> pagadoPorTipo;
    std::for_each(tramos.begin(), tramos.end(),
                  [&pagadoPorTipo](const ChargedLeg& tramo) {
                      pagadoPorTipo[tramo.serviceType] += tramo.charge;
                  });
    return pagadoPorTipo;
}

// El criterio llega desde afuera, asi la red no necesita saber cual es.
std::vector<std::string> namesMatching(const Network& red,
                                       const std::function<bool(const TransportMode&)>& criterio) {
    std::vector<std::string> nombres;
    std::for_each(red.modes().begin(), red.modes().end(),
                  [&nombres, &criterio](const std::unique_ptr<TransportMode>& modo) {
                      if (criterio(*modo)) {
                          nombres.push_back(modo->name());
                      }
                  });
    return nombres;
}

// Empezamos con el primer nombre y vamos agregando ", " y el siguiente.
std::string joined(const std::vector<std::string>& nombres) {
    if (nombres.empty()) {
        return "";
    }
    return std::accumulate(nombres.begin() + 1, nombres.end(), nombres[0],
                           [](const std::string& texto, const std::string& nombre) {
                               return texto + ", " + nombre;
                           });
}

/* -------------------------------------------------------------------------
   MAIN
   ------------------------------------------------------------------------- */
int main() {
    std::cout << "============================================================\n";
    std::cout << "   INTEGRATED ITINERARY AND FARE ENGINE - Valle de Aburra\n";
    std::cout << "============================================================\n\n";

    Network network;
    TransportMode& lineA  = network.add(std::make_unique<MetroLine>("A", 21));
    TransportMode& lineB  = network.add(std::make_unique<MetroLine>("B", 7));
    TransportMode& tramTA = network.add(std::make_unique<TramLine>("T-A Ayacucho", 9));
    TransportMode& cableK = network.add(std::make_unique<CableLine>("K Santo Domingo", 4));
    TransportMode& arvi   = network.add(std::make_unique<TouristCable>("L Arvi", 2, 9180));
    TransportMode& feeder = network.add(std::make_unique<FeederRoute>("130 Robledo", 18));

    std::cout << ">> 1. NETWORK INVENTORY (polymorphic describe)\n";
    network.printInventory();
    std::cout << '\n';

    /* Itinerary deliberately given out of chronological order, so that the
       sort step is observable. */
    const std::vector<Leg> itinerary = {
        {45,  &cableK},
        {0,   &feeder},
        {12,  &lineA},
        {150, &lineB},
        {70,  &tramTA},
        {200, &arvi}
    };

    const FrequentPassenger frequent("Diego");
    const StudentPassenger  student("Valentina");
    const SeniorPassenger   senior("Dona Amparo");

    std::cout << ">> 2. FULL FARE PER PROFILE ON METRO LINE A\n";
    std::cout << "  " << frequent.profile() << " pays " << formatCOP(frequent.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile()  << " pays " << formatCOP(student.fareOn(lineA))  << '\n';
    std::cout << "  " << senior.profile()   << " pays " << formatCOP(senior.fareOn(lineA))   << '\n';
    std::cout << "  " << student.profile()  << " on the tourist cable pays "
              << formatCOP(student.fareOn(arvi)) << " (no flat fare there)\n\n";

    const std::vector<ChargedLeg> charged = chargeItinerary(itinerary, frequent);

    std::cout << ">> 3. CHARGED ITINERARY (frequent profile)\n";
    std::for_each(charged.begin(), charged.end(), [](const ChargedLeg& c) {
        std::cout << "  min " << c.minute << " | " << c.serviceType << " " << c.modeName
                  << " | " << formatCOP(c.charge)
                  << (c.integrated ? "  [free transfer]" : "") << '\n';
    });
    std::cout << '\n';

    std::cout << ">> 4. REPORTS BUILT WITH STL ALGORITHMS\n";
    std::cout << "  accumulate: total paid -> " << formatCOP(totalPaid(charged)) << '\n';
    std::cout << "  count_if: free transfers -> " << freeTransferCount(charged) << '\n';

    const auto byType = paidByServiceType(charged);
    std::for_each(byType.begin(), byType.end(),
        [](const std::pair<const std::string, long long>& entry) {
            std::cout << "  map: paid on " << entry.first << " -> "
                      << formatCOP(entry.second) << '\n';
        });

    const auto firstPaid = std::find_if(charged.begin(), charged.end(),
        [](const ChargedLeg& c) { return c.charge > 0; });
    std::cout << "  find_if: first leg that actually paid -> "
              << (firstPaid == charged.end() ? std::string("none") : firstPaid->modeName) << '\n';

    const std::function<bool(const TransportMode&)> bigModes =
        [](const TransportMode& m) { return m.stations() > 8; };
    std::cout << "  injected policy: modes with more than 8 stations -> "
              << joined(namesMatching(network, bigModes)) << '\n';

    std::cout << "  total for the student on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, student))) << '\n';
    std::cout << "  total for the senior on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, senior))) << '\n';

    std::cout << "\n============================================================\n";
    std::cout << "  End of demonstration.\n";
    std::cout << "============================================================\n";
    return 0;
}
