#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <string>
#include <semaphore.h>

using namespace std;
namespace fs = std::filesystem;

struct Genoma {
    string nombre;
    string contenido;
};

struct DatosHilo {
    string directorio;
    string archivo;
};

const int MAX_GENOMES = 100;
float umbral_g = 0;

vector<Genoma> cola;
sem_t sem_coladatos;  

void encolar_genoma(const Genoma& genoma) {
    sem_wait(&sem_coladatos); 
    cola.push_back(genoma);
    sem_post(&sem_coladatos); 
}


void imprimir_genomas() {
    sem_wait(&sem_coladatos);

    bool algun_genoma_cumple_umbral = false;

    for (const auto& genoma : cola) {
        float Contador_C = 0;
        float Contador_G = 0;
        float Contador_Total = 0;

        for(size_t j = 0; j < genoma.contenido.length(); j++){
            if(genoma.contenido[j] == 'C'){    
                Contador_C++;
                Contador_Total++;
            } else if(genoma.contenido[j] == 'G'){
                Contador_G++;
                Contador_Total++;
            } else if(genoma.contenido[j] == 'A' || genoma.contenido[j] == 'T'){
                Contador_Total++;
            }
        }
        float porcentaje_GC = (Contador_C + Contador_G) / Contador_Total;
        if (porcentaje_GC > umbral_g) {
            std::cout<< "\nGenomas con contenido GC mayor al umbral (" << umbral_g << "):\n\n";
            algun_genoma_cumple_umbral = true;
            
            cout << "Nombre archivo: " << genoma.nombre << endl;
            cout << "Contenido:\n" << genoma.contenido << endl;
            cout << "Porcentaje GC: " << porcentaje_GC * 100 << "%" << endl;
            cout << "----------------------------------------------\n";
        }
    }

    if (!algun_genoma_cumple_umbral) {
        std::cout << "\nNo hay ningún Genoma con contenido GC mayor al umbral (" << umbral_g << ").\n\n";
    }

    sem_post(&sem_coladatos);
}


void procesar_archivo(DatosHilo* datos_hilo) {
    string ruta_completa = datos_hilo->directorio + "/" + datos_hilo->archivo;
    cout << "Procesando archivo: " << ruta_completa << endl;
    ifstream archivo_entrada(ruta_completa);

    if (archivo_entrada.is_open()) {
        archivo_entrada.seekg(0, ios::end);
        size_t tamaño = archivo_entrada.tellg();
        archivo_entrada.seekg(0, ios::beg);

        string tam_archivo(tamaño, '\0');
        archivo_entrada.read(&tam_archivo[0], tamaño);

        float Contador_C = 0, Contador_G = 0, Contador_Total = 0;

        for(size_t i = 0; i < tamaño; i++){
            if( tam_archivo[i] == '>'){
                while(i < tamaño && tam_archivo[i] != '\n'){
                    i++;
                }
                i--;    
            }else if(tam_archivo[i] == 'C'){
                Contador_C++;
                Contador_Total++;
            }else if(tam_archivo[i] == 'G'){
                Contador_G++;
                Contador_Total++;
            }else if(tam_archivo[i] == 'T' || tam_archivo[i] == 'A'){
                Contador_Total++;
            }
        }
        float prom = ((Contador_C + Contador_G) / Contador_Total);

        if (prom >= umbral_g) {
            Genoma genoma{ datos_hilo->archivo, tam_archivo };
            encolar_genoma(genoma);
        }
    }
    delete datos_hilo;
}

void explorar_directorio(const std::string& directorio) {
    std::vector<std::thread> hilos;
    for (const auto& entrada : fs::directory_iterator(directorio)) {
        if (entrada.is_regular_file()) {
            DatosHilo* datos_hilo = new DatosHilo{ directorio, entrada.path().filename() };
            hilos.emplace_back(procesar_archivo, datos_hilo);
        } else if (entrada.is_directory()) {
            explorar_directorio(entrada.path());
        }
    }
    for (auto& hilo : hilos) {
        hilo.join();
    }
}

int main(int argc, char* argv[]) {
    
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <directorio> <umbral>" << endl;
        return 1;
    }

    string directorio = argv[1];
    umbral_g = stof(argv[2]);

    sem_init(&sem_coladatos, 0, 1); 
    explorar_directorio(directorio);
    imprimir_genomas();

    sem_destroy(&sem_coladatos); 
    return 0;
}

