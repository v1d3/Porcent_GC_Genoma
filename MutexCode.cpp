#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <filesystem>
#include <condition_variable>

float umbral_g = 0;
namespace fs = std::filesystem;
typedef struct{
    std::string nombre_archivo;
    std::string contenido;
} Genoma;

typedef struct{
    std::string directorio;
    std::string archivo;
} DatosHilo;

std::mutex mutex;
std::vector<Genoma> cola;

void encolar(const Genoma& gen){
    std::unique_lock<std::mutex> lock(mutex);
    cola.push_back(gen);
    lock.unlock();
}

void imprimir_gen(){
    std::unique_lock<std::mutex> lock(mutex);

    bool algun_genoma_cumple_umbral = false;

    for(const auto& genoma : cola){
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
        float porcentaje_gc = (Contador_C + Contador_G) / Contador_Total ;
        if(porcentaje_gc >= umbral_g ){
            std::cout<< "\nGenomas con contenido GC mayor al umbral (" << umbral_g << "):\n\n";
            algun_genoma_cumple_umbral = true;
            std::cout<< "Nombre archivo: "<< genoma.nombre_archivo << std::endl;
            std::cout<< "Contenido: " << genoma.contenido << std::endl;
            std::cout<< "Porcentaje GC: " << porcentaje_gc * 100 << "%" << std::endl;
            std::cout<< "----------------------------" <<std::endl;
        }
    }
    if (!algun_genoma_cumple_umbral) {
        std::cout << "\nNo hay ningún Genoma con contenido GC mayor al umbral (" << umbral_g << ").\n\n";
    }

}

void *procesar_archivo(DatosHilo* datos_hilo) {
    
    std::string ruta_completa = datos_hilo->directorio + "/" + datos_hilo->archivo;
    std::cout<<"Procesando archivo: "<< ruta_completa << std::endl; 
    
    std::ifstream archiveopen(ruta_completa);

    if(archiveopen.is_open()){ //Just in case
        archiveopen.seekg(0,std::ios::end);
        size_t tamaño = archiveopen.tellg();
        archiveopen.seekg(0,std::ios::beg);

        std::string tam_archivo(tamaño,'\0');

        archiveopen.read(&tam_archivo[0],tamaño);
            
        float Contador_C = 0;
        float Contador_G = 0;
        float Contador_Total = 0;

        for(size_t i = 0; i < tamaño; i++){
            if( tam_archivo[i] == '>'){
                while(i < tamaño && tam_archivo[i] != '\n'){
                    i++;
                }
                i--;    
            }
            else if(tam_archivo[i] == 'C'){
                Contador_C++;
                Contador_Total++;
            }else if(tam_archivo[i] == 'G'){
                Contador_G++;
                Contador_Total++;
            }else if(tam_archivo[i] == 'T' || tam_archivo[i] == 'A'){
                Contador_Total++;
            }
        }
        float prom = ((Contador_C+Contador_G)/Contador_Total);
        if (prom >= umbral_g ){
          
            Genoma genoma{ datos_hilo->archivo, tam_archivo};
            encolar(genoma);
        }
    }
    delete datos_hilo;
    return nullptr;
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
    for (auto& hilo : hilos) { //Esperar a los demás hilos
        hilo.join();
    }
}

int main(int argc, char const *argv[])
{
    // Verificar si se proporciona al menos un argumento (el nombre del programa)
    if (argc < 2) {
        printf("Uso: %s <directorio>\n", argv[0]);
        return 1; // Terminar el programa con un código de error
    }

    // El primer argumento (argv[0]) es el nombre del programa, el segundo (argv[1]) es el directorio y el tercero (argv[3]) es el umbral
    std::string directorio = argv[1];

    umbral_g = atof(argv[2]);

    // Iniciar el proceso de exploración de directorios y creación de hilos
    explorar_directorio(directorio);
    
    imprimir_gen();

    return 0; // Terminar el programa con éxito
}