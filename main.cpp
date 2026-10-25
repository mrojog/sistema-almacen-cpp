#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

// --- ESTRUCTURA PRODUCTO ---
struct Producto {
    string nombre;
    int stock;
    int pasillo;
};

// --- ESTRUCTURA SOLICITUD ---
struct Solicitud {
    int id;
    string cliente;
    vector<pair<string, int> > items; 
};

// --- ESTRUCTURA COMPARADOR DE PRIORIDAD ---
struct CompararPrioridad {
    bool operator()(const Solicitud& a, const Solicitud& b) {
        int prioridadA = (a.cliente == "VIP") ? 1 : 2;
        int prioridadB = (b.cliente == "VIP") ? 1 : 2;
        return prioridadA > prioridadB;
    }
};

// --- CLASE SISTEMA ALMACÉN ---
class SistemaAlmacen {
private:
    map<string, Producto> inventario;
    vector<Solicitud> solicitudes;

public:
    void agregarProducto(string nombre, int stock, int pasillo) {
        Producto p;
        p.nombre = nombre;
        p.stock = stock;
        p.pasillo = pasillo;
        inventario[nombre] = p;
    }

    void recibirSolicitud(int idPedido, string clienteTipo, vector<pair<string, int> > items) {
        Solicitud s;
        s.id = idPedido;
        s.cliente = clienteTipo;
        s.items = items;
        solicitudes.push_back(s);
    }

    void reabastecerProducto(string nombre, int cantidad) {
        map<string, Producto>::iterator it = inventario.find(nombre);
        if (it != inventario.end()) {
            it->second.stock += cantidad;
            cout << "[RESTOCK] Se agregaron " << cantidad << " unidades a '" << nombre 
                 << "'. Nuevo stock: " << it->second.stock << "\n";
        } else {
            cout << "[ERROR RESTOCK] El producto '" << nombre << "' no existe.\n";
        }
    }

    // [NUEVA FUNCIÓN] Cambiar la ubicación física (pasillo) de un producto existente
    void cambiarPasilloProducto(string nombre, int nuevoPasillo) {
        map<string, Producto>::iterator it = inventario.find(nombre);
        if (it != inventario.end()) {
            it->second.pasillo = nuevoPasillo;
            cout << "[PASILLO MODIFICADO] '" << nombre << "' movido con exito al Pasillo " << nuevoPasillo << ".\n";
        } else {
            cout << "[ERROR PASILLO] El producto '" << nombre << "' no existe en el inventario.\n";
        }
    }

    void cancelarSolicitud(int idPedido) {
        vector<Solicitud>::iterator it;
        bool encontrado = false;
        for (it = solicitudes.begin(); it != solicitudes.end(); ) {
            if (it->id == idPedido) {
                it = solicitudes.erase(it);
                encontrado = true;
            } else {
                ++it;
            }
        }
        if (encontrado) {
            cout << "[CANCELACION] Pedido ID " << idPedido << " eliminado.\n";
        } else {
            cout << "[ERROR CANCELACION] No se encontro el Pedido ID " << idPedido << ".\n";
        }
    }

    // [NUEVA FUNCIÓN] Vaciar de forma masiva todos los pedidos en la lista de espera
    void vaciarPedidosPendientes() {
        if (!solicitudes.empty()) {
            solicitudes.clear();
            cout << "[VACIADO MASIVO] Se eliminaron todas las solicitudes pendientes de la lista.\n";
        } else {
            cout << "[VACIADO MASIVO] La lista de solicitudes ya estaba vacia.\n";
        }
    }

    void mostrarReporteInventario() {
        cout << "\n=============================================\n";
        cout << "         REPORTE DE STOCK ACTUAL            \n";
        cout << "=============================================\n";
        for (map<string, Producto>::iterator it = inventario.begin(); it != inventario.end(); ++it) {
            const Producto& p = it->second;
            cout << "- Producto: " << p.nombre 
                 << " | Pasillo: " << p.pasillo 
                 << " | Stock: " << p.stock;
            if (p.stock == 0) cout << " -> [AGOTADO]";
            else if (p.stock < 5) cout << " -> [STOCK CRITICO]";
            cout << "\n";
        }
        cout << "=============================================\n";
    }

    void procesarPedidosYRuta() {
        priority_queue<Solicitud, vector<Solicitud>, CompararPrioridad> colaPrioridad;

        cout << "\n--- 1. ORGANIZACION Y PRIORIZACION DE SOLICITUDES ---\n";
        for (size_t i = 0; i < solicitudes.size(); ++i) {
            Solicitud solicitud = solicitudes[i];
            colaPrioridad.push(solicitud);
            int nivel = (solicitud.cliente == "VIP") ? 1 : 2;
            cout << "Pedido " << solicitud.id << " (" << solicitud.cliente << ") anadido a la cola con nivel " << nivel << ".\n";
        }

        if (colaPrioridad.empty()) {
            cout << "No hay pedidos pendientes por procesar.\n";
            return;
        }

        cout << "\n--- 2. CONTROL DE STOCK Y SECUENCIA DE RECOLECCION OPTIMIZADA ---\n";
        while (!colaPrioridad.empty()) {
            Solicitud pedidoActual = colaPrioridad.top();
            colaPrioridad.pop();

            int nivelActual = (pedidoActual.cliente == "VIP") ? 1 : 2;
            cout << "\nProcesando Pedido ID: " << pedidoActual.id << " (Prioridad nivel " << nivelActual << ")\n";

            vector<pair<int, pair<string, int> > > itemsVerificados;

            for (size_t i = 0; i < pedidoActual.items.size(); ++i) {
                pair<string, int> item = pedidoActual.items[i];
                string nombreProd = item.first;
                int cantidadPedida = item.second;

                map<string, Producto>::iterator it = inventario.find(nombreProd);
                if (it != inventario.end()) {
                    Producto& prodInventario = it->second;

                    if (prodInventario.stock >= cantidadPedida) {
                        prodInventario.stock -= cantidadPedida;
                        itemsVerificados.push_back(make_pair(prodInventario.pasillo, make_pair(nombreProd, cantidadPedida)));
                        cout << "  [STOCK OK] " << nombreProd << ": " << cantidadPedida << " unidades reservadas. Nuevo stock: " << prodInventario.stock << "\n";
                    } else {
                        cout << "  [STOCK INSUFICIENTE] " << nombreProd << ": Solicitado " << cantidadPedida << ", disponible " << prodInventario.stock << ". Item cancelado.\n";
                    }
                } else {
                    cout << "  [ERROR] " << nombreProd << " no existe en el inventario.\n";
                }
            }

            if (!itemsVerificados.empty()) {
                sort(itemsVerificados.begin(), itemsVerificados.end());
                cout << "  >> Secuencia de recoleccion optimizada para Pedido " << pedidoActual.id << ":\n";
                for (size_t i = 0; i < itemsVerificados.size(); ++i) {
                    int pasillo = itemsVerificados[i].first;
                    string nombreProd = itemsVerificados[i].second.first;
                    int cant = itemsVerificados[i].second.second;
                    cout << "     -> Ir a Pasillo " << pasillo << ": Recoger " << cant << " unidades de '" << nombreProd << "'\n";
                }
            } else {
                cout << "  >> Pedido " << pedidoActual.id << " no pudo ser preparado por falta de inventario general.\n";
            }
        }
        solicitudes.clear();
    }
};

// --- FUNCIÓN PRINCIPAL ---
int main() {
    SistemaAlmacen almacen;

    // Inicialización del Inventario
    almacen.agregarProducto("Laptop", 10, 5);
    almacen.agregarProducto("Teclado", 25, 1);
    almacen.agregarProducto("Monitor", 8, 3);
    almacen.agregarProducto("Mouse", 15, 2);

    // PRUEBA 1: Cambiar de pasillo un producto
    cout << "--- PRUEBA CAMBIO DE PASILLO ---\n";
    almacen.cambiarPasilloProducto("Laptop", 4); // Cambia la laptop del pasillo 5 al 4

    almacen.mostrarReporteInventario();

    // Carga de Pedidos normales
    vector<pair<string, int> > items1;
    items1.push_back(make_pair("Laptop", 2));
    items1.push_back(make_pair("Teclado", 1));
    almacen.recibirSolicitud(101, "Regular", items1);

    vector<pair<string, int> > items2;
    items2.push_back(make_pair("Monitor", 1));
    items2.push_back(make_pair("Mouse", 2));
    items2.push_back(make_pair("Teclado", 3));
    almacen.recibirSolicitud(102, "VIP", items2);

    almacen.procesarPedidosYRuta();

    // PRUEBA 2: Carga y posterior vaciado masivo de emergencia
    cout << "\n--- PRUEBA VACIADO MASIVO ---\n";
    vector<pair<string, int> > items3;
    items3.push_back(make_pair("Monitor", 2));
    almacen.recibirSolicitud(103, "Regular", items3);
    
    almacen.vaciarPedidosPendientes(); // Elimina el pedido 103 antes de ser procesado
    almacen.procesarPedidosYRuta();    // Indicará que no hay nada pendiente

    almacen.mostrarReporteInventario();

    return 0;
}

