#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

struct Equipo {
    string nombre;
    int pj=0, pg=0, pe=0, pp=0;
    int gf=0, gc=0, pts=0;
};

struct Partido {
    string fecha, local, visitante;
    int golesLocal, golesVisitante;
};

struct Config {
    string nombreLiga;
    int victoria, empate, derrota;
    vector<string> equipos;
};

Config cargarConfig() {
    Config cfg;
    ifstream file("config.txt");

    if (!file) {
        cout << "Error config.txt\n";
        exit(0);
    }

    string linea;
    while (getline(file, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        string clave, valor;
        getline(ss, clave, '=');
        getline(ss, valor);

        if (clave == "nombre") cfg.nombreLiga = valor;
        else if (clave == "victoria") cfg.victoria = atoi(valor.c_str());
        else if (clave == "empate") cfg.empate = atoi(valor.c_str());
        else if (clave == "derrota") cfg.derrota = atoi(valor.c_str());
        else if (clave == "equipo") cfg.equipos.push_back(valor);
    }

    if (cfg.nombreLiga == "" || cfg.equipos.size() == 0) {
        cout << "Error en config\n";
        exit(0);
    }

    return cfg;
}

void actualizarEquipo(Equipo *e, int gf, int gc, Config &cfg) {
    e->pj++;
    e->gf += gf;
    e->gc += gc;

    if (gf > gc) {
        e->pg++;
        e->pts += cfg.victoria;
    } else if (gf < gc) {
        e->pp++;
        e->pts += cfg.derrota;
    } else {
        e->pe++;
        e->pts += cfg.empate;
    }
}

bool partidoRepetido(Partido nuevo) {
    ifstream file("partidos.txt");
    string linea;

    while (getline(file, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        string fecha, local, visitante;

        getline(ss, fecha, ',');
        getline(ss, local, ',');
        getline(ss, visitante, ',');

        if (local == nuevo.local && visitante == nuevo.visitante)
            return true;

        if (local == nuevo.visitante && visitante == nuevo.local)
            return true;
    }
    return false;
}

bool equipoYaJugoEnFecha(Partido nuevo) {
    ifstream file("partidos.txt");
    string linea;

    while (getline(file, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        string fecha, local, visitante;

        getline(ss, fecha, ',');
        getline(ss, local, ',');
        getline(ss, visitante, ',');

        if (fecha == nuevo.fecha) {
            if (local == nuevo.local || visitante == nuevo.local)
                return true;

            if (local == nuevo.visitante || visitante == nuevo.visitante)
                return true;
        }
    }
    return false;
}

vector<Partido> leerPartidos() {
    vector<Partido> partidos;
    ifstream file("partidos.txt");
    string linea;

    while (getline(file, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        Partido p;
        string temp;

        getline(ss, p.fecha, ',');
        getline(ss, p.local, ',');
        getline(ss, p.visitante, ',');

        getline(ss, temp, ',');
        p.golesLocal = atoi(temp.c_str());

        getline(ss, temp, ',');
        p.golesVisitante = atoi(temp.c_str());

        partidos.push_back(p);
    }

    return partidos;
}

void verTabla(Config &cfg) {
    vector<Partido> partidos = leerPartidos();
    vector<Equipo> tabla;

    for (int i = 0; i < cfg.equipos.size(); i++) {
        Equipo e;
        e.nombre = cfg.equipos[i];
        tabla.push_back(e);
    }

    for (int i = 0; i < partidos.size(); i++) {
        for (int j = 0; j < tabla.size(); j++) {

            if (tabla[j].nombre == partidos[i].local)
                actualizarEquipo(&tabla[j], partidos[i].golesLocal, partidos[i].golesVisitante, cfg);

            if (tabla[j].nombre == partidos[i].visitante)
                actualizarEquipo(&tabla[j], partidos[i].golesVisitante, partidos[i].golesLocal, cfg);
        }
    }

    sort(tabla.begin(), tabla.end(), [](Equipo a, Equipo b) {
        int dgA = a.gf - a.gc;
        int dgB = b.gf - b.gc;

        if (a.pts == b.pts) {
            if (dgA == dgB)
                return a.gf > b.gf;
            return dgA > dgB;
        }
        return a.pts > b.pts;
    });

    ofstream out("tabla.txt");

    string linea1 = "+----+-----------------------+----+----+----+----+----+----+----+----+\n";
    string linea2 = "| #  | Equipo                | PJ | PG | PE | PP | GF | GC | DG | PTS|\n";

    cout << "\n" << linea1 << linea2 << linea1;
    out << linea1 << linea2 << linea1;

    for (int i = 0; i < tabla.size(); i++) {

        string nombre = tabla[i].nombre;
        while (nombre.length() < 22) nombre += " ";

        int dg = tabla[i].gf - tabla[i].gc;

        string pos = to_string(i+1);
        if (pos.length() == 1) pos = " " + pos;

        auto num = [](int n) {
            string s = to_string(n);
            while (s.length() < 3) s = " " + s;
            return s + " ";
        };

        string fila = "| " + pos + " | " + nombre;
        fila += "|" + num(tabla[i].pj);
        fila += "|" + num(tabla[i].pg);
        fila += "|" + num(tabla[i].pe);
        fila += "|" + num(tabla[i].pp);
        fila += "|" + num(tabla[i].gf);
        fila += "|" + num(tabla[i].gc);
        fila += "|" + num(dg);
        fila += "|" + num(tabla[i].pts) + "|\n";

        cout << fila;
        out << fila;
    }

    cout << linea1;
    out << linea1;
}

void registrarPartido(Config &cfg) {
    Partido p;

    cout << "Fecha: ";
    cin >> p.fecha;

    for (int i = 0; i < cfg.equipos.size(); i++)
        cout << i+1 << ". " << cfg.equipos[i] << endl;

    int a,b;
    cout << "Local: "; cin >> a;
    cout << "Visitante: "; cin >> b;

    if (a == b || a < 1 || b < 1 || a > cfg.equipos.size() || b > cfg.equipos.size()) {
        cout << "Error\n";
        return;
    }

    p.local = cfg.equipos[a-1];
    p.visitante = cfg.equipos[b-1];

    if (partidoRepetido(p)) {
        cout << "Ese partido ya fue registrado\n";
        return;
    }

    if (equipoYaJugoEnFecha(p)) {
        cout << "Un equipo ya jugo en esa fecha\n";
        return;
    }

    cout << "Goles local: "; cin >> p.golesLocal;
    cout << "Goles visitante: "; cin >> p.golesVisitante;

    ofstream file("partidos.txt", ios::app);
    file << p.fecha << "," << p.local << "," << p.visitante << ","
         << p.golesLocal << "," << p.golesVisitante << "\n";

    ofstream fechas("fechas.txt", ios::app);
    fechas << p.fecha << " | " << p.local << " "
           << p.golesLocal << "-" << p.golesVisitante
           << " " << p.visitante << "\n";

    cout << "Guardado\n";
}

void reiniciarLiga() {
    ofstream file("partidos.txt");
    file.close();

    ofstream fechas("fechas.txt");
    fechas.close();

    cout << "Liga reiniciada\n";
}

void verPartidos() {
    ifstream file("partidos.txt");
    string linea;

    cout << "\nPARTIDOS:\n";

    while (getline(file, linea)) {
        cout << linea << endl;
    }
}

void menu(Config &cfg) {
    int op;

    do {
        cout << "\n===== " << cfg.nombreLiga << " =====\n";
        cout << "1. Ver tabla\n";
        cout << "2. Registrar partido\n";
        cout << "3. Ver partidos\n";
        cout << "4. Reiniciar liga\n";
        cout << "5. Salir\n";
        cout << "Opcion: ";
        cin >> op;

        if (op == 1) verTabla(cfg);
        else if (op == 2) registrarPartido(cfg);
        else if (op == 3) verPartidos();
        else if (op == 4) reiniciarLiga();

    } while (op != 5);
}

int main() {
    Config cfg = cargarConfig();
    menu(cfg);
    return 0;
}
